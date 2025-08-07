#!/usr/bin/env python3
"""
Register Simulation Framework
Simulates L6470 and AS5600 register behavior for safe testing
Part of STM32H753ZI safe development framework
"""

import sys
import yaml
import argparse
from pathlib import Path
from typing import Dict, List, Any, Optional
from dataclasses import dataclass, field
from enum import Enum
import threading
import time
import math


class SimulationMode(Enum):
    """Simulation operation modes"""
    STATIC = "static"           # Fixed register values
    INTERACTIVE = "interactive"  # Manual control via API
    PHYSICS = "physics"         # Motor physics simulation
    PATTERN = "pattern"         # Predefined motion patterns


@dataclass
class RegisterState:
    """State of a single register in simulation"""
    value: int = 0
    last_write_time: float = 0.0
    write_count: int = 0
    read_count: int = 0
    access_log: List[str] = field(default_factory=list)
    
    def log_access(self, operation: str, value: Optional[int] = None):
        """Log register access for debugging"""
        timestamp = time.time()
        if value is not None:
            entry = f"{timestamp:.3f}: {operation} = 0x{value:04X}"
        else:
            entry = f"{timestamp:.3f}: {operation} = 0x{self.value:04X}"
        self.access_log.append(entry)
        
        # Keep only last 100 entries to prevent memory bloat
        if len(self.access_log) > 100:
            self.access_log.pop(0)


class L6470Simulator:
    """L6470 Stepper Driver Register Simulation"""
    
    def __init__(self, schema_path: str):
        self.schema_path = Path(schema_path)
        self.schema = self._load_schema()
        self.registers = {}
        self.mode = SimulationMode.STATIC
        self.motor_position = 0  # Current position in steps
        self.motor_speed = 0     # Current speed
        self.motor_direction = 1  # 1 = forward, -1 = reverse
        self.motor_state = "stopped"  # stopped, accelerating, running, decelerating
        self.physics_thread = None
        self.running = False
        self._init_registers()
        
    def _load_schema(self) -> Dict[str, Any]:
        """Load register schema"""
        with open(self.schema_path, 'r') as f:
            return yaml.safe_load(f)
            
    def _init_registers(self):
        """Initialize all registers with default values"""
        for reg_name, reg_def in self.schema['registers'].items():
            default_value = reg_def.get('default', 0)
            self.registers[reg_name] = RegisterState(value=default_value)
            
        # Set specific defaults for motor simulation
        self.registers['ABS_POS'].value = 0
        self.registers['SPEED'].value = 0
        self.registers['STATUS'].value = 0x7E00  # Not busy, no faults
        
    def read_register(self, address: int) -> int:
        """Simulate register read"""
        reg_name = self._get_register_name(address)
        if reg_name is None:
            return 0
            
        reg_state = self.registers[reg_name]
        
        # Update dynamic registers before read
        self._update_dynamic_registers()
        
        reg_state.read_count += 1
        reg_state.log_access("READ")
        
        return reg_state.value
        
    def write_register(self, address: int, value: int) -> bool:
        """Simulate register write"""
        reg_name = self._get_register_name(address)
        if reg_name is None:
            return False
            
        reg_def = self.schema['registers'][reg_name]
        
        # Check if register is writable
        if reg_def['access'] == 'read_only':
            return False
            
        # Validate value against schema
        if not self._validate_value(reg_name, value):
            return False
            
        reg_state = self.registers[reg_name]
        reg_state.value = value
        reg_state.last_write_time = time.time()
        reg_state.write_count += 1
        reg_state.log_access("WRITE", value)
        
        # Handle command registers that trigger actions
        self._handle_register_write(reg_name, value)
        
        return True
        
    def send_command(self, command: int, parameter: int = 0) -> bool:
        """Simulate command execution"""
        cmd_name = self._get_command_name(command)
        if cmd_name is None:
            return False
            
        print(f"L6470 Sim: Executing command {cmd_name} with param 0x{parameter:06X}")
        
        # Handle specific commands
        if cmd_name == "RUN":
            direction = (command & 0x01) == 0  # 0 = forward, 1 = reverse
            self._start_motor(parameter, direction)
        elif cmd_name == "MOVE":
            direction = (command & 0x01) == 0
            self._move_steps(parameter, direction)
        elif cmd_name == "GOTO":
            self._goto_position(parameter)
        elif cmd_name == "SOFT_STOP":
            self._stop_motor(soft=True)
        elif cmd_name == "HARD_STOP":
            self._stop_motor(soft=False)
        elif cmd_name == "RESET_DEVICE":
            self._reset_device()
        elif cmd_name == "GET_STATUS":
            return self.registers['STATUS'].value
            
        return True
        
    def _get_register_name(self, address: int) -> Optional[str]:
        """Get register name from address"""
        for reg_name, reg_def in self.schema['registers'].items():
            if reg_def['address'] == address:
                return reg_name
        return None
        
    def _get_command_name(self, command: int) -> Optional[str]:
        """Get command name from opcode"""
        base_cmd = command & 0xF0  # Remove direction/parameter bits
        for cmd_name, cmd_def in self.schema['commands'].items():
            if (cmd_def['opcode'] & 0xF0) == base_cmd:
                return cmd_name
        return None
        
    def _validate_value(self, reg_name: str, value: int) -> bool:
        """Validate register value against schema"""
        reg_def = self.schema['registers'][reg_name]
        
        # Check mask
        if value & (~reg_def['mask']) != 0:
            return False
            
        # Check valid range if defined
        if 'valid_range' in reg_def:
            min_val, max_val = reg_def['valid_range']
            if not (min_val <= value <= max_val):
                return False
                
        return True
        
    def _update_dynamic_registers(self):
        """Update registers that change based on motor state"""
        # Update position register
        self.registers['ABS_POS'].value = self.motor_position & 0x3FFFFF
        
        # Update speed register
        speed_ticks = int(abs(self.motor_speed) * 67108.864)  # Convert to L6470 format
        self.registers['SPEED'].value = speed_ticks & 0x0FFFFF
        
        # Update status register
        status = 0x7E00  # Base status (not in HiZ, no faults)
        if self.motor_state != "stopped":
            status |= 0x0002  # BUSY bit
        if self.motor_direction > 0:
            status |= 0x0010  # DIR bit
            
        # Motor status bits
        if self.motor_state == "accelerating":
            status |= 0x0020  # MOT_STATUS = 01 (acceleration)
        elif self.motor_state == "decelerating":
            status |= 0x0040  # MOT_STATUS = 10 (deceleration)
        elif self.motor_state == "running":
            status |= 0x0060  # MOT_STATUS = 11 (constant speed)
            
        self.registers['STATUS'].value = status
        
    def _handle_register_write(self, reg_name: str, value: int):
        """Handle side effects of register writes"""
        if reg_name == "ABS_POS":
            self.motor_position = value
        elif reg_name in ["ACC", "DEC", "MAX_SPEED", "MIN_SPEED"]:
            # Motion parameters changed - update physics if running
            pass
            
    def _start_motor(self, speed: int, forward: bool):
        """Start motor at specified speed"""
        self.motor_direction = 1 if forward else -1
        target_speed = self._convert_speed_from_l6470(speed)
        self.motor_state = "accelerating"
        print(f"L6470 Sim: Starting motor {'forward' if forward else 'reverse'} at {target_speed:.1f} steps/sec")
        
    def _move_steps(self, steps: int, forward: bool):
        """Move specified number of steps"""
        self.motor_direction = 1 if forward else -1
        target_position = self.motor_position + (steps * self.motor_direction)
        self.motor_state = "accelerating"
        print(f"L6470 Sim: Moving {steps} steps {'forward' if forward else 'reverse'} to position {target_position}")
        
    def _goto_position(self, position: int):
        """Go to absolute position"""
        if position > self.motor_position:
            self.motor_direction = 1
        else:
            self.motor_direction = -1
        self.motor_state = "accelerating"
        print(f"L6470 Sim: Going to position {position}")
        
    def _stop_motor(self, soft: bool):
        """Stop motor"""
        if soft:
            self.motor_state = "decelerating"
            print("L6470 Sim: Soft stop - decelerating")
        else:
            self.motor_state = "stopped"
            self.motor_speed = 0
            print("L6470 Sim: Hard stop - immediate")
            
    def _reset_device(self):
        """Reset device to power-on state"""
        self.motor_position = 0
        self.motor_speed = 0
        self.motor_state = "stopped"
        self._init_registers()
        print("L6470 Sim: Device reset")
        
    def _convert_speed_from_l6470(self, speed_value: int) -> float:
        """Convert L6470 speed format to steps/second"""
        return speed_value * 250e-9 * (2**28)
        
    def start_physics_simulation(self):
        """Start physics simulation thread"""
        if self.physics_thread is not None:
            return
            
        self.running = True
        self.physics_thread = threading.Thread(target=self._physics_loop)
        self.physics_thread.start()
        print("L6470 Sim: Physics simulation started")
        
    def stop_physics_simulation(self):
        """Stop physics simulation thread"""
        self.running = False
        if self.physics_thread:
            self.physics_thread.join()
            self.physics_thread = None
        print("L6470 Sim: Physics simulation stopped")
        
    def _physics_loop(self):
        """Physics simulation loop"""
        dt = 0.001  # 1ms time step
        
        while self.running:
            # Simple motor physics simulation
            if self.motor_state == "accelerating":
                acceleration = 1000  # steps/sec²
                self.motor_speed += acceleration * dt * self.motor_direction
                max_speed = 2000  # steps/sec
                if abs(self.motor_speed) >= max_speed:
                    self.motor_speed = max_speed * self.motor_direction
                    self.motor_state = "running"
                    
            elif self.motor_state == "decelerating":
                deceleration = 1000  # steps/sec²
                if self.motor_speed > 0:
                    self.motor_speed -= deceleration * dt
                    if self.motor_speed <= 0:
                        self.motor_speed = 0
                        self.motor_state = "stopped"
                else:
                    self.motor_speed += deceleration * dt
                    if self.motor_speed >= 0:
                        self.motor_speed = 0
                        self.motor_state = "stopped"
                        
            # Update position based on speed
            if self.motor_speed != 0:
                self.motor_position += self.motor_speed * dt
                
            time.sleep(dt)


class AS5600Simulator:
    """AS5600 Magnetic Encoder Simulation"""
    
    def __init__(self, schema_path: str):
        self.schema_path = Path(schema_path)
        self.schema = self._load_schema()
        self.registers = {}
        self.angle_degrees = 0.0
        self.angle_pattern = "static"  # static, rotating, sinusoidal
        self.rotation_speed = 0.0  # degrees/second
        self.start_time = time.time()
        self._init_registers()
        
    def _load_schema(self) -> Dict[str, Any]:
        """Load register schema"""
        with open(self.schema_path, 'r') as f:
            return yaml.safe_load(f)
            
    def _init_registers(self):
        """Initialize all registers with default values"""
        for reg_name, reg_def in self.schema['registers'].items():
            default_value = reg_def.get('default', 0)
            self.registers[reg_name] = RegisterState(value=default_value)
            
        # Set specific defaults for encoder simulation
        self.registers['STATUS'].value = 0x20  # Magnet detected
        self.registers['AGC'].value = 128      # Optimal AGC
        self.registers['MAGNITUDE_H'].value = 0x08
        self.registers['MAGNITUDE_L'].value = 0x00  # Good magnitude
        
    def read_register(self, address: int) -> int:
        """Simulate register read"""
        reg_name = self._get_register_name(address)
        if reg_name is None:
            return 0
            
        # Update dynamic registers before read
        self._update_angle_registers()
        
        reg_state = self.registers[reg_name]
        reg_state.read_count += 1
        reg_state.log_access("READ")
        
        return reg_state.value
        
    def write_register(self, address: int, value: int) -> bool:
        """Simulate register write"""
        reg_name = self._get_register_name(address)
        if reg_name is None:
            return False
            
        reg_def = self.schema['registers'][reg_name]
        
        # Check if register is writable
        if reg_def['access'] == 'read_only':
            return False
            
        # Validate value
        if not self._validate_value(reg_name, value):
            return False
            
        reg_state = self.registers[reg_name]
        reg_state.value = value
        reg_state.last_write_time = time.time()
        reg_state.write_count += 1
        reg_state.log_access("WRITE", value)
        
        return True
        
    def _get_register_name(self, address: int) -> Optional[str]:
        """Get register name from address"""
        for reg_name, reg_def in self.schema['registers'].items():
            if reg_def['address'] == address:
                return reg_name
        return None
        
    def _validate_value(self, reg_name: str, value: int) -> bool:
        """Validate register value against schema"""
        reg_def = self.schema['registers'][reg_name]
        
        # Check mask
        if value & (~reg_def['mask']) != 0:
            return False
            
        return True
        
    def _update_angle_registers(self):
        """Update angle-related registers based on simulation pattern"""
        current_time = time.time()
        elapsed = current_time - self.start_time
        
        # Update angle based on pattern
        if self.angle_pattern == "rotating":
            self.angle_degrees = (self.rotation_speed * elapsed) % 360.0
        elif self.angle_pattern == "sinusoidal":
            amplitude = 90.0  # ±90 degrees
            frequency = 0.5   # 0.5 Hz
            self.angle_degrees = amplitude * math.sin(2 * math.pi * frequency * elapsed) + 180.0
        # static pattern keeps angle_degrees unchanged
        
        # Convert angle to 12-bit value (0-4095)
        angle_raw = int((self.angle_degrees / 360.0) * 4096) & 0xFFF
        
        # Update angle registers
        self.registers['ANGLE_H'].value = (angle_raw >> 8) & 0x0F
        self.registers['ANGLE_L'].value = angle_raw & 0xFF
        
        # Raw angle with some noise
        noise = int((time.time() * 1000) % 8) - 4  # ±4 LSB noise
        raw_angle = (angle_raw + noise) & 0xFFF
        self.registers['RAW_ANGLE_H'].value = (raw_angle >> 8) & 0x0F
        self.registers['RAW_ANGLE_L'].value = raw_angle & 0xFF
        
    def set_angle_pattern(self, pattern: str, speed: float = 0.0):
        """Set angle simulation pattern"""
        self.angle_pattern = pattern
        self.rotation_speed = speed
        self.start_time = time.time()
        print(f"AS5600 Sim: Angle pattern set to {pattern} at {speed} deg/sec")
        
    def set_angle(self, degrees: float):
        """Set specific angle"""
        self.angle_degrees = degrees % 360.0
        self.angle_pattern = "static"
        print(f"AS5600 Sim: Angle set to {self.angle_degrees:.1f} degrees")


class RegisterSimulationFramework:
    """Main simulation framework coordinating both chips"""
    
    def __init__(self, l6470_schema: str, as5600_schema: str):
        self.l6470 = L6470Simulator(l6470_schema)
        self.as5600 = AS5600Simulator(as5600_schema)
        self.linked_motion = False  # Whether encoder follows motor
        self.steps_per_revolution = 200  # Default steps per revolution
        
    def enable_linked_motion(self, steps_per_revolution: int = 200):
        """Enable encoder to follow motor position"""
        self.linked_motion = True
        self.steps_per_revolution = steps_per_revolution
        print(f"Simulation: Linked motion enabled ({steps_per_revolution} steps/rev)")
        
    def disable_linked_motion(self):
        """Disable linked motion"""
        self.linked_motion = False
        print("Simulation: Linked motion disabled")
        
    def update_linked_motion(self):
        """Update encoder position based on motor position"""
        if not self.linked_motion:
            return
            
        motor_steps = self.l6470.motor_position
        angle_degrees = (motor_steps / self.steps_per_revolution) * 360.0
        self.as5600.angle_degrees = angle_degrees % 360.0
        
    def get_status_report(self) -> Dict[str, Any]:
        """Get comprehensive simulation status"""
        return {
            "l6470": {
                "position": self.l6470.motor_position,
                "speed": self.l6470.motor_speed,
                "state": self.l6470.motor_state,
                "direction": "forward" if self.l6470.motor_direction > 0 else "reverse"
            },
            "as5600": {
                "angle": self.as5600.angle_degrees,
                "pattern": self.as5600.angle_pattern,
                "speed": self.as5600.rotation_speed
            },
            "simulation": {
                "linked_motion": self.linked_motion,
                "l6470_physics_active": self.l6470.running
            }
        }


def main():
    """Main simulation framework function"""
    parser = argparse.ArgumentParser(description="Register Simulation Framework")
    parser.add_argument("--l6470-schema", default="schemas/l6470_registers.yaml", help="L6470 schema file")
    parser.add_argument("--as5600-schema", default="schemas/as5600_registers.yaml", help="AS5600 schema file")
    parser.add_argument("--interactive", "-i", action="store_true", help="Interactive mode")
    parser.add_argument("--test-pattern", help="Run test pattern (basic, rotation, linked)")
    
    args = parser.parse_args()
    
    # Create simulation framework
    sim = RegisterSimulationFramework(args.l6470_schema, args.as5600_schema)
    
    if args.test_pattern:
        if args.test_pattern == "basic":
            print("Running basic test pattern...")
            # Test basic register access
            sim.l6470.write_register(0x07, 0x041)  # Set MAX_SPEED
            max_speed = sim.l6470.read_register(0x07)
            print(f"L6470 MAX_SPEED: 0x{max_speed:04X}")
            
            # Test AS5600 angle reading
            angle_h = sim.as5600.read_register(0x0E)
            angle_l = sim.as5600.read_register(0x0F)
            angle_raw = ((angle_h & 0x0F) << 8) | angle_l
            angle_deg = (angle_raw / 4096.0) * 360.0
            print(f"AS5600 Angle: {angle_deg:.1f} degrees")
            
        elif args.test_pattern == "rotation":
            print("Running rotation test pattern...")
            sim.as5600.set_angle_pattern("rotating", 30.0)  # 30 deg/sec
            for i in range(10):
                time.sleep(0.5)
                angle_h = sim.as5600.read_register(0x0E)
                angle_l = sim.as5600.read_register(0x0F)
                angle_raw = ((angle_h & 0x0F) << 8) | angle_l
                angle_deg = (angle_raw / 4096.0) * 360.0
                print(f"t={i*0.5:.1f}s: Angle = {angle_deg:.1f} degrees")
                
        elif args.test_pattern == "linked":
            print("Running linked motion test pattern...")
            sim.enable_linked_motion(200)  # 200 steps/revolution
            sim.l6470.start_physics_simulation()
            
            # Move motor and watch encoder follow
            sim.l6470.send_command(0x50, 100)  # RUN forward at speed 100
            
            for i in range(20):
                time.sleep(0.1)
                sim.update_linked_motion()
                
                status = sim.get_status_report()
                print(f"Motor: {status['l6470']['position']:.0f} steps, "
                      f"Encoder: {status['as5600']['angle']:.1f}°")
                      
            sim.l6470.send_command(0xB0)  # SOFT_STOP
            sim.l6470.stop_physics_simulation()
            
    elif args.interactive:
        print("Interactive simulation mode - implement CLI interface here")
        # TODO: Implement interactive CLI
        pass
        
    else:
        print("Simulation framework ready!")
        print("Use --test-pattern or --interactive for testing")
        
    return 0


if __name__ == "__main__":
    sys.exit(main())
