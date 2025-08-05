# STM32H753ZI Motor Control System Documentation

## 📚 Complete Documentation Portal

Welcome to the comprehensive documentation for the STM32H753ZI stepper motor control system with X-NUCLEO-IHM02A1 shield and AS5600 magnetic encoders.

## 🚀 Quick Start

### For Developers
1. **[API Reference](html/html/index.html)** - Generated Doxygen documentation
2. **[Safety Systems Guide](guides/safety_systems.md)** - Critical safety implementation
3. **[Motor Control Guide](guides/motor_control.md)** - L6470 and motion control
4. **[Build Instructions](../README.md)** - Setup and compilation

### For System Integrators
1. **[Safety Validation](validation/safety_validation.md)** - Safety testing procedures
2. **[Configuration Guide](../src/config/)** - SSOT configuration management
3. **[Hardware Setup](guides/hardware_setup.md)** - Physical system configuration

## 📖 Documentation Categories

### 🛡️ Safety Documentation
- **[Safety Systems Guide](guides/safety_systems.md)** - Emergency stop, fault detection, watchdog
- **[Safety Validation Procedures](validation/safety_validation.md)** - Comprehensive safety testing
- **[Emergency Stop Testing](testing/emergency_stop_testing.md)** - Specific emergency stop validation

### ⚙️ Technical Guides
- **[Motor Control Developer Guide](guides/motor_control.md)** - L6470 stepper motor control
- **[Communication Protocols](guides/communication.md)** - UART, CAN, SPI interfaces
- **[Hardware Configuration](guides/hardware_setup.md)** - STM32H753ZI and shield setup

### 🔧 API Documentation
- **[Generated API Reference](html/html/index.html)** - Complete Doxygen documentation
- **[Module Index](html/html/modules.html)** - Organized by functional modules
- **[File Index](html/html/files.html)** - Source file documentation
- **[API Overview](api/README.md)** - API structure and organization

### 📋 Requirements & Design
- **[System Requirements](requirements/system_requirements.md)** - Functional and safety requirements
- **[Architecture Design](design/system_architecture.md)** - Overall system design
- **[Safety Requirements](requirements/safety_requirements.md)** - Safety-critical requirements

### 🧪 Testing & Validation
- **[Testing Strategy](testing/testing_strategy.md)** - Overall testing approach
- **[Unit Testing Guide](testing/unit_testing.md)** - Unity C testing framework
- **[Integration Testing](testing/integration_testing.md)** - System-level testing

## 🎯 Key Features

### Safety Systems
- **Hardware Emergency Stop**: < 100ms response time
- **Software Emergency Stop**: Immediate motor shutdown
- **Fault Detection**: Overcurrent, thermal, encoder monitoring
- **Watchdog Protection**: Hardware and software watchdogs
- **State Management**: Fail-safe state transitions

### Motor Control
- **Dual L6470 Drivers**: Advanced stepper motor control
- **Closed-Loop Feedback**: AS5600 magnetic encoder integration
- **Motion Profiling**: Trapezoidal velocity profiles
- **Multi-Motor Coordination**: Synchronized motion control
- **Real-Time Control**: 1kHz control loop frequency

### Communication
- **UART Interface**: Configuration and monitoring
- **CAN Bus**: Real-time communication
- **SPI Communication**: L6470 and AS5600 interfaces
- **USB CDC**: Debug and diagnostic interface

## 🔍 Enhanced Documentation Search

Use the advanced documentation search system for finding specific information:

```bash
# Search STM32H7 HAL functions
python3 scripts/search_enhanced_docs.py function HAL_GPIO_Init --scope STM32H7

# Find L6470 motor control functions  
python3 scripts/search_enhanced_docs.py function L6470 --scope L6470

# Search Nucleo BSP functions
python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP

# Find concept-related documentation
python3 scripts/search_enhanced_docs.py concept "safety systems" --scope all

# Search across all documentation
python3 scripts/search_enhanced_docs.py concept "motor control" --scope all
```

**Search Capabilities**:
- **91.1MB** total searchable documentation
- **32,200+** indexed keywords
- **4,227** documentation files
- **7x faster** than HTML-based searches

## 📊 System Overview

```
STM32H753ZI Nucleo-144
├── X-NUCLEO-IHM02A1 Shield
│   ├── L6470 Motor Driver 1
│   ├── L6470 Motor Driver 2
│   └── Power Management
├── AS5600 Magnetic Encoders
│   ├── Motor 1 Position Feedback
│   └── Motor 2 Position Feedback
└── Safety Systems
    ├── Hardware Emergency Stop
    ├── Fault Detection
    ├── Watchdog Protection
    └── State Management
```

## 🔗 External Resources

### STM32 Official Documentation
- **[STM32H753ZI Reference Manual](00_reference/RM0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.md)**
- **[STM32H7 HAL Documentation](00_reference/STM32Cube_FW_H7_V1.12.0/)**
- **[X-CUBE-SPN2 Documentation](00_reference/x_cube_spn2_markdown_docs/)**

### Hardware References
- **[X-NUCLEO-IHM02A1 User Manual](00_reference/ST_Assets/)**
- **[L6470 Datasheet](00_reference/x_cube_spn2_markdown_docs/)**
- **[AS5600 Encoder Documentation](00_reference/)**

## ⚠️ Safety Notice

**CRITICAL**: This system controls stepper motors and must follow safety protocols:

1. **Always implement emergency stop procedures**
2. **Test safety systems before operation**
3. **Monitor fault conditions continuously**
4. **Follow proper lockout/tagout procedures**
5. **Validate all safety systems after changes**

## 🛠️ Development Environment

### Prerequisites
- ARM GCC toolchain
- CMake build system
- Unity C testing framework
- Doxygen documentation generator
- ST-LINK debugger/programmer

### Build Commands
```bash
# Build firmware
cmake -S . -B build && cmake --build build

# Run unit tests
cd build && ctest

# Generate documentation
doxygen docs/Doxyfile

# Validate SSOT configuration
python3 scripts/validate_ssot.py

# Flash firmware
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/stm32h753_ihm02a1.elf verify reset exit"
```

## 📝 Recent Updates

- ✅ **Unity Testing Framework**: 12/12 tests passing with host-based testing
- ✅ **Enhanced Doxygen Documentation**: Comprehensive API reference generated
- ✅ **Safety Validation Procedures**: Complete safety testing protocols
- ✅ **SSOT Compliance**: Reduced configuration issues from 236 to 225
- ✅ **Developer Guides**: Safety systems and motor control guides

## 📧 Support

For technical questions, safety concerns, or system integration support:
- Review the appropriate developer guide
- Check the safety validation procedures
- Use the documentation search system
- Consult the API reference documentation

---

**Remember**: Safety first, SSOT always, modular design throughout.
