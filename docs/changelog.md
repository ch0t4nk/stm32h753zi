# Changelog

## [0.2.0] - 2025-08-05 🎉 MAJOR MILESTONE: Peripheral Initialization Complete

### ✅ Added - STM32CubeMX Integration
- **Complete peripheral initialization** for all required peripherals (SPI2, I2C1, I2C2, TIM2, FDCAN1)
- **CubeMX programmatic configuration generator** (`scripts/cubemx_config_generator.py`)
- **Pin conflict resolution** automation in Python tooling
- **Manual peripheral setup** with proper MSP functions and GPIO configuration

### ✅ Added - Development Environment Optimization  
- **Python formatting standardization** with Black formatter (79-character line length)
- **Cross-platform editor consistency** with `.editorconfig` configuration
- **VS Code spell checker** configured for STM32 technical terminology (2000+ terms)
- **pyproject.toml** configuration for Python tools (Black, isort, flake8)

### ✅ Added - Enhanced Build System
- **CMake ARM GCC toolchain** configuration validated and working
- **All peripheral handles** available for driver implementation:
  - `hspi2` - L6470 stepper drivers (SPI2: PB13/14/15, CS: PA9)
  - `hi2c1` - AS5600 encoder 1 (I2C1: PB6/7)  
  - `hi2c2` - AS5600 encoder 2 (I2C2: PB10/11)
  - `htim2` - 1kHz control loop timer
  - `hfdcan1` - MCU communication (FDCAN1: PD0/1)

### ✅ Fixed - Code Quality & Linting
- **All Python lint errors resolved** (E501 line length issues)  
- **Black formatter conflict resolution** - no more auto-revert issues
- **Trailing whitespace cleanup** across all Python files
- **Import organization** with isort integration

### ✅ Enhanced - Documentation System
- **Updated README.md** with current development status and next steps
- **Recent progress summary** documenting all major accomplishments
- **Peripheral status documentation** with ready-to-use code examples
- **Driver development roadmap** with clear implementation targets

### 🔧 Technical Details
- **Pin Assignments Validated**: All peripheral pins configured without conflicts
- **MSP Functions Complete**: Hardware-specific initialization for all peripherals  
- **Interrupt Configuration**: TIM2 and FDCAN1 interrupts properly configured
- **Clock Configuration**: All peripheral clocks enabled and configured

### 🚀 Ready for Next Phase
- **Driver Implementation Phase**: All prerequisites complete
- **L6470 SPI Driver**: Ready for implementation using `hspi2` handle
- **AS5600 I2C Driver**: Ready for implementation using `hi2c1`/`hi2c2` handles
- **Control Loop Integration**: TIM2 interrupt handler ready for motor control logic

## [0.1.0] - 2025-07-28
### ✅ Added - Project Foundation
- Initial scaffold with directory structure
- CI/CD configuration and documentation framework  
- API specifications (REST: `api/openapi.yaml`, CAN: `api/can_api.yaml`)
- Enhanced documentation system with STM32H7 reference integration

---

**Development Status**: ✅ Peripheral initialization complete → 🚀 Ready for driver development

**Next Milestone**: Complete L6470 and AS5600 driver implementation with closed-loop control
