# X-CUBE-MCSDK + X-CUBE-SPN2 Integration Plan

## Overview
This document outlines the integration plan for combining **X-CUBE-MCSDK 6.4.1** (Motor Control SDK) and **X-CUBE-SPN2** (Stepper Motor Package) with our STM32H753ZI + X-NUCLEO-IHM02A1 + AS5600 project.

## Revised Integration Strategy 🎯

### ✅ **Primary Approach: X-CUBE-SPN2 + IHM02A1**
- **Core Framework**: X-CUBE-SPN2 (dedicated stepper motor package)
- **Hardware Driver**: X-NUCLEO-IHM02A1 board support with L6470 drivers
- **Reference Documentation**: `00_reference/x_cube_spn2_markdown_docs` (2.1MB, 197 files)
- **MCSDK Integration**: Selective adoption of MCSDK algorithms and tools for enhanced capabilities

### ✅ **Why This Approach is Superior**:
1. **Native Stepper Support**: X-CUBE-SPN2 is specifically designed for stepper motors
2. **IHM02A1 Optimized**: Perfect match for our X-NUCLEO-IHM02A1 shield
3. **L6470 Integration**: Complete L6470 driver implementation included
4. **STM32H7 Compatible**: No algorithm limitations (unlike MCSDK FOC restrictions)
5. **MCSDK Enhancement**: We can still leverage MCSDK's advanced features selectively

## X-CUBE-SPN2 + MCSDK Hybrid Architecture

### Phase 1: Base X-CUBE-SPN2 Implementation
1. **Use X-CUBE-SPN2 as Foundation**: 
   - Complete L6470 stepper motor control
   - X-NUCLEO-IHM02A1 board support
   - Proven stepper motor algorithms
   
2. **STM32CubeMX Project Creation**:
   - **Board**: NUCLEO-H753ZI 
   - **Add Package**: X-CUBE-SPN2 (stepper-specific)
   - **Optional**: Add X-CUBE-MCSDK 6.4.1 (for selective features)

### Phase 2: Enhance with MCSDK Capabilities  
**Selectively integrate MCSDK 6.4.1 features that benefit stepper control**:

#### ✅ **MCSDK Features to Adopt**:
1. **Advanced Position Control**: MCSDK's position algorithms for precise stepper positioning
2. **Motion Profiling**: Velocity/acceleration profiles for smooth stepper operation  
3. **Real-time Monitoring**: MCSDK's Motor Control Workbench for parameter tuning
4. **Safety Systems**: Enhanced fault detection and recovery mechanisms
5. **Dual Sensor Support**: MCSDK's dual sensor APIs adapted for dual AS5600 encoders

#### ❌ **MCSDK Features to Skip**:
1. **FOC Algorithms**: Not needed for stepper motors
2. **3-Phase PWM Generation**: L6470 handles motor drive internally
3. **Current Sensing Topology**: L6470 manages current control

### Phase 3: AS5600 Integration
- **Base on X-CUBE-SPN2**: Use SPN2's encoder interface as foundation
- **Enhance with MCSDK**: Apply MCSDK's dual sensor support for redundant AS5600s
- **Closed-loop Control**: Combine SPN2 L6470 control with AS5600 position feedback

## Current Project Architecture
```
STM32H753ZI (Main Controller)
├── X-NUCLEO-IHM02A1 Shield (2x L6470 Stepper Drivers)
├── 2x AS5600 Magnetic Encoders (I2C Feedback)
├── Communication: UART, CAN-FD, Ethernet
└── Safety: Watchdog, Fault Detection
```

## MCSDK Integration Strategy

### Phase 1: Install and Generate Base Code
1. **Install X-CUBE-MCSDK** on Windows (using provided installer)
2. **Create CubeMX Project** for STM32H753ZI + NUCLEO board
3. **Add MCSDK Package** via Additional Software
4. **Generate Initial Code** with motor control framework

### Phase 2: Adapt Generated Code to Our Architecture

#### ⚠️ Critical Adaptation Strategy (Based on MCSDK 6.4.1 Capabilities)

**Key Insight**: Since STM32H753ZI supports **FOC algorithms only** (not 6-Step), we'll leverage MCSDK's FOC infrastructure and adapt it for stepper motor control. This provides us with:
- Advanced position control algorithms ✅
- Velocity profiling and acceleration limiting ✅  
- Real-time monitoring and tuning ✅
- Safety and fault detection ✅

#### A. Motor Driver Adaptation (FOC → Stepper)
- **From**: MCSDK FOC 3-phase BLDC/PMSM drivers
- **To**: L6470 stepper driver integration within FOC framework
- **Strategy**: 
  - Use MCSDK's position control algorithms
  - Replace 3-phase PWM generation with L6470 SPI commands
  - Map FOC torque/current commands to L6470 current control
  - Leverage MCSDK's acceleration/deceleration profiles for L6470 motion

#### B. Feedback System Integration (FOC Encoder → AS5600)
- **From**: MCSDK encoder interfaces (typically quadrature)
- **To**: AS5600 I2C magnetic encoder feedback within FOC framework
- **Strategy**:
  - Implement AS5600 drivers within MCSDK encoder interface
  - Use MCSDK's **Dual Sensor Support** (6.4.0 feature) for redundant AS5600s
  - Map AS5600 absolute position to MCSDK electrical angle calculations
  - Leverage MCSDK's **Speed Overshoot Mitigation** (6.4.0 improvement)

#### C. SSOT Integration
- **Merge**: MCSDK configuration with our SSOT config headers
- **Update**: `src/config/motor_config.h` with MCSDK parameters
- **Validate**: Ensure no conflicts with existing SSOT architecture

## 3. Project Structure Integration

### Integration with Existing Architecture
Following `.github/instructions/file-organization.instructions.md` and `ssot-config.instructions.md`, all code integrates into our established `src/` structure:

```
src/
├── config/                     # SSOT Configuration (Updated)
│   ├── hardware_config.h       # ✅ Updated with STM32H7 clock config + SPN2/MCSDK flags
│   ├── motor_config.h          # ✅ Updated with SPN2/MCSDK integration parameters
│   ├── comm_config.h           # Communication protocols (existing)
│   ├── safety_config.h         # Safety systems (existing)
│   └── build_config.h          # Build metadata (existing)
├── drivers/                    # Hardware Abstraction Layer
│   ├── hal/                    # ✅ STM32H7 HAL integration layer
│   │   ├── stm32h7xx_hal_conf.h
│   │   ├── STM32H7xx_HAL_Driver/   # Copied from 00_reference/
│   │   └── CMSIS/                  # Device definitions
│   ├── l6470/                  # L6470 stepper drivers (SPN2 based)
│   ├── as5600/                 # AS5600 encoder drivers  
│   └── adaptation/             # SPN2-MCSDK integration layer
├── controllers/                # Control Algorithms (Enhanced)
│   ├── stepper_control.c       # Base SPN2 stepper control
│   ├── position_control.c      # MCSDK enhanced position control
│   └── motion_profiling.c      # MCSDK advanced motion profiles
├── communication/              # Protocol Implementations (Existing)
├── safety/                     # Safety Systems (Existing) 
└── application/                # ✅ Main Application
    └── main.c                  # Generated with proper SSOT integration
```

### SSOT Integration Points
All configuration references centralized SSOT headers:

**hardware_config.h Updates:**
- STM32H7 clock configuration parameters
- SPN2/MCSDK framework enable flags  
- Hybrid integration mode settings

**motor_config.h Updates:**
- X-CUBE-SPN2 framework configuration
- MCSDK selective enhancement flags
- Integration layer parameters

### Phase 4: Enhanced Capabilities

#### What MCSDK 6.4.1 Brings to Our Project:
1. **Advanced FOC Control Algorithms** (STM32H753ZI Compatible):
   - Position control with velocity/acceleration profiling
   - **Maximum Modulation Index (MMI)** computation (6.4.0 improvement)
   - **Speed Overshoot Mitigation** for smoother startup (6.4.0 improvement)
   - Real-time performance optimization

2. **Enhanced Motor Parameter Management**:
   - Motor characterization tools
   - Dynamic parameter adjustment  
   - **Dual Sensor Switching APIs** (6.4.0 feature)
   - Performance monitoring with reduced memory footprint (6.4.0 improvement)

3. **Advanced Safety and Diagnostics**:
   - Motor fault detection
   - Thermal protection
   - Current limiting and monitoring
   - **Regular Conversion Manager (RCM)** for synchronized ADC (6.4.0 improvement)

4. **Enhanced GUI Development Tools** (6.4.0 Improvements):
   - **New Project Composer UI** for better board selection
   - **Enhanced Motor Control Workbench** for parameter tuning
   - **Enhanced Motor Pilot** with plot trigger functionality
   - Real-time monitoring capabilities
   - Parameter export/import

### Phase 5: Custom Enhancements

#### Our Project-Specific Adaptations:
1. **Dual L6470 Support**: Coordinate dual stepper motors
2. **AS5600 Closed-Loop**: High-precision position feedback
3. **Multi-Protocol Communication**: UART, CAN, Ethernet control
4. **SSOT Configuration Management**: Centralized parameter management

## Implementation Steps

### Step 1: MCSDK Installation and Code Generation
```bash
# After Windows installation and CubeMX code generation:
# Copy generated code to workspace integration area
mkdir -p src/mcsdk_integration
# Analyze generated structure and APIs
```

### Step 2: Driver Integration
```c
// Integrate L6470 with MCSDK motor interface
// File: src/drivers/mcsdk_adaptation/l6470_mcsdk_interface.c

#include "mc_interface.h"
#include "l6470_driver.h"

// Implement MCSDK motor interface using L6470
MC_FuncStatus_t MC_StartMotor(uint8_t motor_id) {
    return l6470_start_motor(motor_id) == L6470_OK ? MC_FUNC_OK : MC_FUNC_FAIL;
}
```

### Step 3: Feedback Integration
```c
// Integrate AS5600 with MCSDK feedback interface
// File: src/drivers/mcsdk_adaptation/as5600_mcsdk_interface.c

#include "mc_interface.h" 
#include "as5600_driver.h"

// Implement MCSDK encoder interface using AS5600
int16_t MC_GetElectricalAngle(uint8_t motor_id) {
    return as5600_get_angle_degrees(motor_id);
}
```

### Step 4: Configuration Unification
```c
// Update SSOT configuration with MCSDK parameters
// File: src/config/motor_config.h

// MCSDK Configuration Integration
#define MCSDK_MOTOR_COUNT           MAX_MOTORS
#define MCSDK_POSITION_FEEDBACK     AS5600_ENCODER
#define MCSDK_MOTOR_DRIVER          L6470_STEPPER
#define MCSDK_CONTROL_FREQUENCY_HZ  CONTROL_LOOP_FREQUENCY_HZ
```

## Benefits of This Integration

### 1. **Proven FOC Framework** (Adapted for Steppers)
- ST's validated FOC algorithms adapted for stepper control
- Optimized for STM32H7 performance (6.4.0 improvements)
- Real-time control loop implementation with synchronized ADC

### 2. **Enhanced Development Tools** (6.4.0 UI Improvements)
- **New Project Composer** with improved board selection
- **Enhanced Motor Control Workbench** for parameter tuning
- **Enhanced Motor Pilot** with plot triggering and Open Loop angle plotting
- Real-time monitoring and debugging capabilities

### 3. **Advanced Features** (6.4.0/6.4.1 Enhancements)
- **Speed Overshoot Mitigation** for smoother stepper operation  
- **Dual Sensor Support** for redundant AS5600 encoders
- **Maximum Modulation Index** computation for optimal control
- Advanced safety and fault detection
- Parameter optimization tools with reduced memory footprint

### 4. **Maintained Project Architecture**
- SSOT configuration management preserved
- Modular driver architecture maintained  
- Enhanced with MCSDK FOC capabilities adapted for steppers

## Known Limitations and Workarounds

## Known Limitations and Adapted Solutions

### ✅ **No Limitations with X-CUBE-SPN2 Approach**
**Advantage**: X-CUBE-SPN2 is specifically designed for stepper motors - no algorithm restrictions!

### ✅ **Perfect STM32H7 Compatibility**  
**Advantage**: X-CUBE-SPN2 + IHM02A1 is fully compatible with STM32H753ZI
**Result**: Native stepper support without workarounds

### ✅ **Enhanced with MCSDK Features**
**Strategy**: Use MCSDK 6.4.1 selectively for advanced features:
- Position control algorithms 
- Motion profiling capabilities
- Real-time monitoring tools
- Enhanced safety systems

## Current Status: Ready for Hybrid Implementation ✅

**Perfect Strategy!** Using X-CUBE-SPN2 as foundation with selective MCSDK enhancement gives us:
- Native stepper motor control ✅
- X-NUCLEO-IHM02A1 optimization ✅  
- Professional control algorithms ✅
- No compatibility limitations ✅

## Next Steps - Create CubeMX Project with SPN2 + MCSDK

### Step 1: ✅ Both Packages Available
- **X-CUBE-MCSDK 6.4.1**: `C:\STMicroelectronics\MC_SDK_6.4.1` ✅
- **X-CUBE-SPN2**: Available in STM32CubeMX package manager ✅

### Step 2: Create New STM32CubeMX Project (Version 6.15.0)
1. **Open STM32CubeMX 6.15.0**
2. **Board Selector**: Search `NUCLEO-H753ZI`
3. **Select NUCLEO-H753ZI** board
4. **Configure board options** (LEDs, Virtual COM Port, etc.) ✅ 
5. **Click "Start Project"**

### Step 2: Create New STM32CubeMX Project (Version 6.15.0)
1. **Open STM32CubeMX 6.15.0**
2. **Board Selector**: Search `NUCLEO-H753ZI`
3. **Select NUCLEO-H753ZI** board
4. **Select recommended board options** (LEDs, Virtual COM Port, etc.)
5. **Click "Start Project"**

### Step 3: Add X-CUBE-SPN2 Package (Primary)
**Priority 1: Stepper Motor Foundation**

1. **Go to "Software Packs"** tab
2. **Select Packs** → **STMicroelectronics**  
3. **Find "X-CUBE-SPN2"** (Stepper motor expansion package)
4. **Install and Enable X-CUBE-SPN2**
5. **Go to "Pinout & Configuration"** tab
6. **Software Packs** → **STMicroelectronics.X-CUBE-SPN2**
7. **Stepper Motor Control** → **Check to Enable**

#### Configure X-CUBE-SPN2 Settings:
8. **Board Selection**: **X-NUCLEO-IHM02A1** ✅ Perfect match
9. **Number of Motors**: **2** (dual L6470 support)
10. **Motor Control Mode**: **Position Control** (with acceleration/deceleration)
11. **Feedback**: **Encoder** (we'll adapt for AS5600)

### Step 4: Add X-CUBE-MCSDK (Selective Enhancement) 
**Priority 2: Enhanced Capabilities**

1. **Software Packs** → **Find "X-CUBE-MCSDK"** version **6.4.1**
2. **Enable MCSDK** (for selective features only)
3. **Configure MCSDK Components**:
   - **Position Control Algorithms**: ✅ Enable
   - **Motion Profiling**: ✅ Enable  
   - **Real-time Monitoring**: ✅ Enable
   - **Motor Control Workbench**: ✅ Enable
   - **Safety Features**: ✅ Enable
   - **Dual Sensor Support**: ✅ Enable

**Note**: We'll use MCSDK as enhancement library, not primary motor control

### Step 5: Configure Peripherals for Our Hardware (CubeMX 6.15.0)
Based on our existing `hardware_config.h`, configure these peripherals for X-NUCLEO-IHM02A1:

#### A. SPI Configuration (for X-NUCLEO-IHM02A1 Shield):
**X-CUBE-SPN2 will configure most of this automatically**, but verify:
- **SPI1**: 
  - Mode: **Full-Duplex Master** 
  - **X-NUCLEO-IHM02A1 Shield Configuration** ✅
  - L6470 #1 and L6470 #2 daisy-chain communication
  - CS, FLAG, STBY, BUSY pins automatically configured by SPN2

#### B. I2C Configuration (for AS5600 Encoders):
**Manual configuration required** (not part of X-CUBE-SPN2):
- **I2C1**: 
  - Mode: **I2C**
  - Speed: **Standard Mode (100 kHz)** or **Fast Mode (400 kHz)**
  - AS5600 Motor 1 encoder

- **I2C2**:
  - Mode: **I2C** 
  - AS5600 Motor 2 encoder

#### C. Communication Interfaces:
- **USART3**: 
  - Mode: **Asynchronous**
  - Baud Rate: **115200**
  - **Virtual COM Port** ✅ (already configured in board options)

- **FDCAN1**:
  - Mode: **Activate**
  - Bit Rate: **1 Mbps**

- **ETH**:
  - Mode: **RMII**
  - PHY: **LAN8742**

### Step 6: Generate Code and Save Project (CubeMX 6.15.0)
1. **Save Project**:
   - **File** → **Save Project As**
   - **Location**: `/workspaces/code/stm32h753zi_spn2_mcsdk.ioc`
   - **Project Name**: `stm32h753zi_spn2_mcsdk`

2. **Configure Code Generation**:
   - **Project Manager** tab → **Project**
   - **Toolchain/IDE**: **Makefile** (for CMake integration)
   - **Project Location**: `/workspaces/code/spn2_mcsdk_generated`
   - **Project Name**: `stm32h753zi_spn2_mcsdk`

3. **Code Generator Settings**:
   - **Copy all used libraries into the project folder** ✅
   - **Generate peripheral initialization as a pair of '.c/.h' files per peripheral** ✅

4. **Generate Code**:
   - **Click "GENERATE CODE"**
   - X-CUBE-SPN2 will generate complete L6470 stepper control framework
   - MCSDK components will be available for selective integration

### Step 7: Analyze Generated X-CUBE-SPN2 + MCSDK Structure
After generation, you'll have:
```
/workspaces/code/spn2_mcsdk_generated/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32h7xx_hal_conf.h
│   │   └── stm32h7xx_it.h
│   └── Src/
│       ├── main.c              # Main application with SPN2 + MCSDK init
│       ├── stm32h7xx_hal_msp.c # HAL MSP configuration
│       └── stm32h7xx_it.c      # Interrupt handlers
├── Drivers/
│   ├── STM32H7xx_HAL_Driver/   # STM32H7 HAL drivers
│   └── CMSIS/                  # CMSIS core and device files
├── X-CUBE-SPN2_v1.x.x/        # ✅ Stepper Motor SDK components
│   ├── BSP/
│   │   ├── X-NUCLEO-IHM02A1/   # IHM02A1 board support
│   │   └── Components/L6470/   # L6470 stepper driver
│   ├── STM32H7xx-Nucleo/      # STM32H7 specific adaptations
│   └── Documentation/          # SPN2 documentation
├── MCSDK_v6_4_1/              # ✅ Motor Control SDK components (selective)
│   ├── MotorControl/           # Enhanced control algorithms
│   │   ├── MCAPI/             # Motor Control API
│   │   ├── Position/          # Position control algorithms
│   │   └── Safety/            # Safety and monitoring
│   └── Tools/                 # Motor Control Workbench integration
├── Inc/                       # Project-specific headers
├── Src/                       # Project-specific sources
├── stm32h753zi_spn2_mcsdk.ioc # CubeMX project file
└── Makefile                   # Generated Makefile
```

### Step 8: Copy Components to Our Workspace
```bash
# Create integration directories
mkdir -p src/spn2
mkdir -p src/mcsdk_selective

# Copy X-CUBE-SPN2 components (primary framework)
cp -r spn2_mcsdk_generated/X-CUBE-SPN2_v1.x.x/BSP src/spn2/
cp -r spn2_mcsdk_generated/X-CUBE-SPN2_v1.x.x/STM32H7xx-Nucleo src/spn2/

# Copy selective MCSDK components (enhancements)
cp -r spn2_mcsdk_generated/MCSDK_v6_4_1/MotorControl/Position src/mcsdk_selective/
cp -r spn2_mcsdk_generated/MCSDK_v6_4_1/MotorControl/Safety src/mcsdk_selective/
cp -r spn2_mcsdk_generated/MCSDK_v6_4_1/Tools src/mcsdk_selective/

# Copy CubeMX project file to workspace root
cp spn2_mcsdk_generated/stm32h753zi_spn2_mcsdk.ioc ./
```

### Step 6: Integration with Our Workspace
After code generation, we'll:
1. **Analyze generated structure** and identify key MCSDK components
2. **Integrate with SSOT** configuration system
3. **Replace motor drivers** with L6470 SPI interface
4. **Replace feedback system** with AS5600 I2C interface
5. **Update CMake build system** to include MCSDK components

## Documentation References

### Official MCSDK 6.4.1 Documentation
- **X-CUBE-MCSDK User Manual**: Comprehensive MCSDK documentation (installed at `C:\STMicroelectronics\MC_SDK_6.4.1\Documentation\`)
- **MCSDK 6.4.0 Release Announcement**: [ST Community Forum](https://community.st.com/t5/stm32-mcus-motor-control/mcsdk-6-4-0-is-out/td-p/803732)
- **Release Notes**: Available in Motor Control Workbench → About → Documentations → Release Notes
- **Motor Control Workbench Guide**: GUI tool documentation (6.4.0 enhanced UI)
- **STM32H7 MCSDK Examples**: Reference implementations (installed locally)

### Version Compatibility
- **X-CUBE-MCSDK**: 6.4.1 (Latest Stable) ✅
- **STM32CubeMX**: 6.15.0 ✅ Compatible
- **STM32H753ZI**: ✅ FOC Support (6-Step NOT supported)
- **Target Hardware**: NUCLEO-H753ZI + X-NUCLEO-IHM02A1 ✅

### Community Resources
- **ST Motor Control Forum**: [STM32 MCUs Motor Control](https://community.st.com/t5/stm32-mcus-motor-control/bd-p/mcu-motor-control-forum)
- **MCSDK Documentation**: [User Manual](https://community.st.com/t5/stm32-mcus-motor-control/user-manual-mcsdk-documentation/td-p/826832)
- **Known Issues**: Search community for "MCSDK v6.41" for latest discussions

### Our Project Instructions
- **Integration with existing instruction system**: All SSOT config files will be updated
- **L6470 Integration**: Reference `.github/instructions/stm32h7-spi-l6470.instructions.md`
- **AS5600 Integration**: Reference `.github/instructions/stm32h7-i2c-as5600.instructions.md`  
- **Hardware Configuration**: Reference `.github/instructions/hardware-pins.instructions.md`

---

**Status**: ✅ Documentation updated with MCSDK 6.4.1 release information
**Critical Notes**: STM32H753ZI requires FOC algorithms (6-Step not supported)
**Next Action**: Create CubeMX project with FOC configuration for stepper motor adaptation
