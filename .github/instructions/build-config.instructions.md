---
applyTo: "src/config/build_config.h"
description: "Build configuration and version management for STM32H753ZI stepper motor project"
---

# Build Configuration and Version Management Instructions

## Overview
This instruction file provides comprehensive guidance for implementing robust build configuration, version management, and compile-time feature selection for the STM32H753ZI stepper motor control project.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive build and development documentation:
- **STM32H7 Build System**: Use `python3 scripts/documentation/search_docs.py concept build` for build system guidance
- **HAL Configuration**: `00_reference/stm32h753xx_User_Manual_md/` contains STM32H753ZI configuration documentation
- **Development Tools**: `00_reference/STM32Cube_FW_H7_V1.12.0/` contains build system examples and templates
- **Compiler Documentation**: STM32CubeIDE and ARM GCC toolchain specifics for STM32H7 optimization

## Version Management System

### Semantic Versioning Structure
Follow semantic versioning (SemVer) specification with additional build metadata:

```c
// Version format: MAJOR.MINOR.PATCH-PRERELEASE+BUILD
// Example: 1.2.3-beta.1+20241201.a1b2c3d

// Major version: Breaking changes, incompatible API changes
#define VERSION_MAJOR           1

// Minor version: New features, backward compatible
#define VERSION_MINOR           0

// Patch version: Bug fixes, backward compatible
#define VERSION_PATCH           0

// Pre-release identifier (empty for release builds)
#define VERSION_PRERELEASE      "dev"

// Build metadata (generated automatically)
#define VERSION_BUILD_DATE      __DATE__
#define VERSION_BUILD_TIME      __TIME__
#define VERSION_BUILD_NUMBER    1234
#define VERSION_GIT_COMMIT      "a1b2c3d4"
#define VERSION_GIT_BRANCH      "develop"
#define VERSION_GIT_DIRTY       0  // 1 if uncommitted changes exist

// Compiler information
#define VERSION_COMPILER        "GCC " __VERSION__
#define VERSION_STM32_HAL       "1.12.0"  // STM32H7 HAL version
#define VERSION_FREERTOS        "10.5.1"  // FreeRTOS version (if used)
#define VERSION_LWIP            "2.1.3"   // LwIP version (if used)
```

### Version String Generation
```c
// Generate version strings at compile time
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Full version string
#define VERSION_STRING_FULL \
    TOSTRING(VERSION_MAJOR) "." \
    TOSTRING(VERSION_MINOR) "." \
    TOSTRING(VERSION_PATCH) \
    (sizeof(VERSION_PRERELEASE) > 1 ? "-" VERSION_PRERELEASE : "") \
    "+" VERSION_BUILD_DATE "." VERSION_GIT_COMMIT

// Short version string (for displays with limited space)
#define VERSION_STRING_SHORT \
    TOSTRING(VERSION_MAJOR) "." \
    TOSTRING(VERSION_MINOR) "." \
    TOSTRING(VERSION_PATCH)

// Version as 32-bit integer for comparisons
#define VERSION_AS_INTEGER \
    ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_PATCH << 8))

// Version compatibility checking
#define VERSION_IS_COMPATIBLE(major, minor) \
    (VERSION_MAJOR == (major) && VERSION_MINOR >= (minor))

// API version (changes when interface breaks)
#define API_VERSION_MAJOR       1
#define API_VERSION_MINOR       0
#define API_VERSION_STRING      TOSTRING(API_VERSION_MAJOR) "." TOSTRING(API_VERSION_MINOR)
```

### Build Identification
```c
// Build type identification
typedef enum {
    BUILD_TYPE_DEBUG = 0,               // Debug build with symbols
    BUILD_TYPE_RELEASE = 1,             // Optimized release build
    BUILD_TYPE_TEST = 2,                // Test build with instrumentation
    BUILD_TYPE_PRODUCTION = 3           // Production build with security
} BuildType_t;

// Build configuration at compile time
#ifdef DEBUG
    #define CURRENT_BUILD_TYPE  BUILD_TYPE_DEBUG
    #define BUILD_TYPE_STRING   "Debug"
#elif defined(TESTING)
    #define CURRENT_BUILD_TYPE  BUILD_TYPE_TEST
    #define BUILD_TYPE_STRING   "Test"
#elif defined(PRODUCTION)
    #define CURRENT_BUILD_TYPE  BUILD_TYPE_PRODUCTION
    #define BUILD_TYPE_STRING   "Production"
#else
    #define CURRENT_BUILD_TYPE  BUILD_TYPE_RELEASE
    #define BUILD_TYPE_STRING   "Release"
#endif

// Target hardware identification
#define TARGET_MCU              "STM32H753ZI"
#define TARGET_BOARD            "NUCLEO-H753ZI"
#define TARGET_EXPANSION        "X-NUCLEO-IHM02A1"
#define TARGET_CPU_FREQ_MHZ     480
#define TARGET_FLASH_SIZE_KB    2048
#define TARGET_RAM_SIZE_KB      1024
```

## Feature Configuration

### Compile-Time Feature Selection
```c
// Communication features
#define FEATURE_UART_ENABLED            1   // Enable UART communication
#define FEATURE_CAN_ENABLED             1   // Enable CAN communication
#define FEATURE_ETHERNET_ENABLED        1   // Enable Ethernet communication
#define FEATURE_SPI_ENABLED             1   // Enable SPI (required for L6470)
#define FEATURE_I2C_ENABLED             1   // Enable I2C (required for AS5600)

// Motor control features
#define FEATURE_DUAL_MOTOR              1   // Enable dual motor control
#define FEATURE_CLOSED_LOOP             1   // Enable closed-loop control
#define FEATURE_ENCODER_FEEDBACK        1   // Enable encoder feedback
#define FEATURE_POSITION_HOMING         1   // Enable automatic homing
#define FEATURE_MOTION_PROFILES         1   // Enable motion profiling
#define FEATURE_ANTI_BACKLASH           0   // Enable backlash compensation

// Safety features
#define FEATURE_WATCHDOG_ENABLED        1   // Enable watchdog timer
#define FEATURE_EMERGENCY_STOP          1   // Enable emergency stop
#define FEATURE_FAULT_RECOVERY          1   // Enable automatic fault recovery
#define FEATURE_THERMAL_PROTECTION      1   // Enable thermal monitoring
#define FEATURE_CURRENT_LIMITING        1   // Enable current limiting
#define FEATURE_POSITION_LIMITS         1   // Enable software position limits

// Diagnostic features
#define FEATURE_SYSTEM_MONITORING       1   // Enable system health monitoring
#define FEATURE_ERROR_LOGGING           1   // Enable error logging
#define FEATURE_PERFORMANCE_COUNTERS    1   // Enable performance monitoring
#define FEATURE_DEBUG_UART              1   // Enable debug output via UART
#define FEATURE_MEMORY_DIAGNOSTICS      0   // Enable memory usage diagnostics

// Advanced features
#define FEATURE_RTOS_ENABLED            1   // Enable FreeRTOS
#define FEATURE_LWIP_ENABLED            1   // Enable LwIP TCP/IP stack
#define FEATURE_FATFS_ENABLED           0   // Enable FatFS file system
#define FEATURE_USB_ENABLED             0   // Enable USB communication
#define FEATURE_BOOTLOADER_SUPPORT      0   // Enable bootloader support
```

### Feature Dependencies and Validation
```c
// Feature dependency validation
#if FEATURE_CLOSED_LOOP && !FEATURE_ENCODER_FEEDBACK
    #error "Closed-loop control requires encoder feedback"
#endif

#if FEATURE_ETHERNET_ENABLED && !FEATURE_LWIP_ENABLED
    #error "Ethernet requires LwIP TCP/IP stack"
#endif

#if FEATURE_DUAL_MOTOR && !FEATURE_SPI_ENABLED
    #error "Dual motor control requires SPI for L6470 drivers"
#endif

#if FEATURE_ENCODER_FEEDBACK && !FEATURE_I2C_ENABLED
    #error "Encoder feedback requires I2C for AS5600 sensors"
#endif

#if FEATURE_RTOS_ENABLED && (FEATURE_WATCHDOG_ENABLED == 0)
    #warning "RTOS without watchdog may not detect deadlocks"
#endif

// Memory requirement validation
#if FEATURE_LWIP_ENABLED
    #define LWIP_MEMORY_REQUIREMENT_KB  128
#else
    #define LWIP_MEMORY_REQUIREMENT_KB  0
#endif

#if FEATURE_RTOS_ENABLED
    #define RTOS_MEMORY_REQUIREMENT_KB  32
#else
    #define RTOS_MEMORY_REQUIREMENT_KB  0
#endif

#define TOTAL_MEMORY_REQUIREMENT_KB \
    (64 + LWIP_MEMORY_REQUIREMENT_KB + RTOS_MEMORY_REQUIREMENT_KB)

#if TOTAL_MEMORY_REQUIREMENT_KB > TARGET_RAM_SIZE_KB
    #error "Total memory requirement exceeds available RAM"
#endif
```

### Configuration Profiles
```c
// Pre-defined configuration profiles
#ifdef CONFIG_PROFILE_MINIMAL
    // Minimal configuration for basic motor control
    #undef FEATURE_ETHERNET_ENABLED
    #define FEATURE_ETHERNET_ENABLED        0
    #undef FEATURE_CAN_ENABLED
    #define FEATURE_CAN_ENABLED             0
    #undef FEATURE_RTOS_ENABLED
    #define FEATURE_RTOS_ENABLED            0
    #undef FEATURE_SYSTEM_MONITORING
    #define FEATURE_SYSTEM_MONITORING       0
#endif

#ifdef CONFIG_PROFILE_DEVELOPMENT
    // Development configuration with full debugging
    #undef FEATURE_DEBUG_UART
    #define FEATURE_DEBUG_UART              1
    #undef FEATURE_ERROR_LOGGING
    #define FEATURE_ERROR_LOGGING           1
    #undef FEATURE_PERFORMANCE_COUNTERS
    #define FEATURE_PERFORMANCE_COUNTERS    1
    #undef FEATURE_MEMORY_DIAGNOSTICS
    #define FEATURE_MEMORY_DIAGNOSTICS      1
#endif

#ifdef CONFIG_PROFILE_PRODUCTION
    // Production configuration with security and minimal debug
    #undef FEATURE_DEBUG_UART
    #define FEATURE_DEBUG_UART              0
    #undef FEATURE_MEMORY_DIAGNOSTICS
    #define FEATURE_MEMORY_DIAGNOSTICS      0
    #define FEATURE_SECURE_BOOT             1
    #define FEATURE_CODE_PROTECTION         1
#endif
```

## Build-Time Constants

### System Configuration Constants
```c
// System timing constants
#define SYSTEM_TICK_FREQUENCY_HZ        1000    // System tick frequency
#define MOTOR_CONTROL_FREQUENCY_HZ      1000    // Motor control loop frequency
#define ENCODER_SAMPLE_FREQUENCY_HZ     2000    // Encoder sampling frequency
#define SAFETY_CHECK_FREQUENCY_HZ       100     // Safety check frequency
#define COMMUNICATION_TIMEOUT_MS        1000    // Default communication timeout

// Buffer sizes (power of 2 for efficiency)
#define UART_TX_BUFFER_SIZE             256     // UART transmit buffer
#define UART_RX_BUFFER_SIZE             256     // UART receive buffer
#define CAN_TX_BUFFER_SIZE              64      // CAN transmit buffer
#define CAN_RX_BUFFER_SIZE              64      // CAN receive buffer
#define TCP_TX_BUFFER_SIZE              1024    // TCP transmit buffer
#define TCP_RX_BUFFER_SIZE              1024    // TCP receive buffer
#define ERROR_LOG_BUFFER_SIZE           512     // Error log buffer

// Motor control constants
#define MAX_MOTORS                      2       // Maximum number of motors
#define MAX_ENCODERS                    2       // Maximum number of encoders
#define STEPS_PER_REVOLUTION            200     // Steps per motor revolution
#define MICROSTEPS_DEFAULT              16      // Default microstep setting
#define MAX_MOTOR_VELOCITY_DPS          360.0f  // Maximum velocity (deg/sec)
#define MAX_MOTOR_ACCELERATION_DPS2     1000.0f // Maximum acceleration
#define POSITION_TOLERANCE_DEGREES      0.1f    // Position control tolerance

// Safety constants
#define WATCHDOG_TIMEOUT_MS             5000    // Watchdog timeout
#define EMERGENCY_STOP_TIMEOUT_MS       100     // Emergency stop response time
#define FAULT_RECOVERY_MAX_ATTEMPTS     3       // Maximum recovery attempts
#define THERMAL_WARNING_TEMP_C          70.0f   // Thermal warning temperature
#define THERMAL_SHUTDOWN_TEMP_C         85.0f   // Thermal shutdown temperature
#define MIN_SUPPLY_VOLTAGE_V            7.0f    // Minimum supply voltage
#define MAX_SUPPLY_VOLTAGE_V            46.0f   // Maximum supply voltage
```

### Hardware Configuration Constants
```c
// GPIO pin assignments (from hardware_config.h)
#define MOTOR_SPI_CS_PIN_NUMBER         9       // SPI CS pin number
#define MOTOR_SPI_SCK_PIN_NUMBER        13      // SPI SCK pin number
#define MOTOR_SPI_MISO_PIN_NUMBER       14      // SPI MISO pin number
#define MOTOR_SPI_MOSI_PIN_NUMBER       15      // SPI MOSI pin number

#define ENCODER1_I2C_SCL_PIN_NUMBER     6       // I2C1 SCL pin number
#define ENCODER1_I2C_SDA_PIN_NUMBER     7       // I2C1 SDA pin number
#define ENCODER2_I2C_SCL_PIN_NUMBER     10      // I2C2 SCL pin number
#define ENCODER2_I2C_SDA_PIN_NUMBER     11      // I2C2 SDA pin number

#define UART_TX_PIN_NUMBER              8       // UART TX pin number
#define UART_RX_PIN_NUMBER              9       // UART RX pin number

#define CAN_TX_PIN_NUMBER               1       // CAN TX pin number
#define CAN_RX_PIN_NUMBER               0       // CAN RX pin number

// Peripheral instance assignments
#define MOTOR_SPI_INSTANCE_NUMBER       2       // SPI2 for motor drivers
#define ENCODER1_I2C_INSTANCE_NUMBER    1       // I2C1 for encoder 1
#define ENCODER2_I2C_INSTANCE_NUMBER    2       // I2C2 for encoder 2
#define UART_INSTANCE_NUMBER            3       // USART3 for VCP
#define CAN_INSTANCE_NUMBER             1       // FDCAN1 for CAN bus
```

## Conditional Compilation

### Debug and Test Macros
```c
// Debug output macros (only active in debug builds)
#if defined(DEBUG) && FEATURE_DEBUG_UART
    #define DEBUG_PRINT(fmt, ...) \
        do { \
            char debug_buffer[128]; \
            snprintf(debug_buffer, sizeof(debug_buffer), \
                     "[DEBUG %s:%d] " fmt "\r\n", \
                     __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            uart_transmit_string(debug_buffer); \
        } while(0)
    
    #define DEBUG_ASSERT(condition) \
        do { \
            if (!(condition)) { \
                DEBUG_PRINT("ASSERTION FAILED: %s", #condition); \
                while(1); /* Halt in debug mode */ \
            } \
        } while(0)
#else
    #define DEBUG_PRINT(fmt, ...)   ((void)0)
    #define DEBUG_ASSERT(condition) ((void)0)
#endif

// Test instrumentation (only active in test builds)
#if defined(TESTING)
    extern uint32_t test_hook_counter;
    #define TEST_HOOK(id) (test_hook_counter |= (1 << (id)))
    #define TEST_COUNTER_INCREMENT(counter) ((counter)++)
#else
    #define TEST_HOOK(id) ((void)0)
    #define TEST_COUNTER_INCREMENT(counter) ((void)0)
#endif

// Performance measurement (only in development builds)
#if FEATURE_PERFORMANCE_COUNTERS
    #define PERF_START(timer) \
        uint32_t perf_start_##timer = HAL_GetTick()
    
    #define PERF_END(timer, max_time_ms) \
        do { \
            uint32_t perf_duration = HAL_GetTick() - perf_start_##timer; \
            if (perf_duration > (max_time_ms)) { \
                DEBUG_PRINT("PERF WARNING: %s took %lu ms (max %lu)", \
                           #timer, perf_duration, (uint32_t)(max_time_ms)); \
            } \
        } while(0)
#else
    #define PERF_START(timer) ((void)0)
    #define PERF_END(timer, max_time_ms) ((void)0)
#endif
```

### Memory Layout Configuration
```c
// Memory regions based on STM32H753ZI memory map
#define FLASH_BASE_ADDRESS              0x08000000UL
#define FLASH_SIZE_BYTES                (2048 * 1024)   // 2MB Flash
#define FLASH_END_ADDRESS               (FLASH_BASE_ADDRESS + FLASH_SIZE_BYTES - 1)

#define RAM_D1_BASE_ADDRESS             0x24000000UL    // AXI SRAM
#define RAM_D1_SIZE_BYTES               (512 * 1024)    // 512KB
#define RAM_D2_BASE_ADDRESS             0x30000000UL    // AHB SRAM1/2
#define RAM_D2_SIZE_BYTES               (288 * 1024)    // 288KB
#define RAM_D3_BASE_ADDRESS             0x38000000UL    // AHB SRAM4
#define RAM_D3_SIZE_BYTES               (64 * 1024)     // 64KB

// Memory allocation strategy
#if FEATURE_LWIP_ENABLED
    // Place Ethernet buffers in D2 domain for DMA access
    #define ETH_BUFFER_SECTION          __attribute__((section(".eth_buffers")))
    #define ETH_BUFFER_BASE             RAM_D2_BASE_ADDRESS
    #define ETH_BUFFER_SIZE             (32 * 1024)     // 32KB for Ethernet
#endif

#if FEATURE_RTOS_ENABLED
    // RTOS heap in main SRAM
    #define RTOS_HEAP_SIZE_BYTES        (128 * 1024)    // 128KB for RTOS
    #define RTOS_STACK_SIZE_BYTES       (4 * 1024)      // 4KB per task stack
#endif

// Application memory allocation
#define APP_HEAP_SIZE_BYTES             (64 * 1024)     // 64KB application heap
#define ERROR_LOG_SIZE_BYTES            (8 * 1024)      // 8KB error log
#define CONFIG_BACKUP_SIZE_BYTES        (4 * 1024)      // 4KB config backup
```

## Build Information Structure

### Runtime Build Information
```c
// Build information structure available at runtime
typedef struct {
    // Version information
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    const char* version_prerelease;
    const char* version_string;
    uint32_t version_integer;
    
    // Build metadata
    const char* build_date;
    const char* build_time;
    uint32_t build_number;
    const char* git_commit;
    const char* git_branch;
    bool git_dirty;
    
    // Target information
    const char* target_mcu;
    const char* target_board;
    uint32_t cpu_frequency_hz;
    uint32_t flash_size_kb;
    uint32_t ram_size_kb;
    
    // Compiler information
    const char* compiler_version;
    const char* hal_version;
    const char* rtos_version;
    
    // Feature flags
    uint32_t features_enabled;      // Bit field of enabled features
    BuildType_t build_type;
    const char* build_type_string;
    
    // Memory configuration
    uint32_t heap_size_bytes;
    uint32_t stack_size_bytes;
    uint32_t buffer_sizes[8];       // Various buffer sizes
} BuildInfo_t;

// Global build information instance
extern const BuildInfo_t g_build_info;

// Build information initialization
#define INIT_BUILD_INFO() { \
    .version_major = VERSION_MAJOR, \
    .version_minor = VERSION_MINOR, \
    .version_patch = VERSION_PATCH, \
    .version_prerelease = VERSION_PRERELEASE, \
    .version_string = VERSION_STRING_FULL, \
    .version_integer = VERSION_AS_INTEGER, \
    .build_date = VERSION_BUILD_DATE, \
    .build_time = VERSION_BUILD_TIME, \
    .build_number = VERSION_BUILD_NUMBER, \
    .git_commit = VERSION_GIT_COMMIT, \
    .git_branch = VERSION_GIT_BRANCH, \
    .git_dirty = (VERSION_GIT_DIRTY != 0), \
    .target_mcu = TARGET_MCU, \
    .target_board = TARGET_BOARD, \
    .cpu_frequency_hz = TARGET_CPU_FREQ_MHZ * 1000000UL, \
    .flash_size_kb = TARGET_FLASH_SIZE_KB, \
    .ram_size_kb = TARGET_RAM_SIZE_KB, \
    .compiler_version = VERSION_COMPILER, \
    .hal_version = VERSION_STM32_HAL, \
    .rtos_version = VERSION_FREERTOS, \
    .build_type = CURRENT_BUILD_TYPE, \
    .build_type_string = BUILD_TYPE_STRING, \
    .heap_size_bytes = APP_HEAP_SIZE_BYTES, \
    .stack_size_bytes = RTOS_STACK_SIZE_BYTES \
}

// Feature bit definitions for features_enabled field
#define FEATURE_BIT_UART            (1 << 0)
#define FEATURE_BIT_CAN             (1 << 1)
#define FEATURE_BIT_ETHERNET        (1 << 2)
#define FEATURE_BIT_CLOSED_LOOP     (1 << 3)
#define FEATURE_BIT_ENCODER         (1 << 4)
#define FEATURE_BIT_WATCHDOG        (1 << 5)
#define FEATURE_BIT_RTOS            (1 << 6)
#define FEATURE_BIT_LWIP            (1 << 7)
// ... continue for all features
```

### Build Validation Functions
```c
// Runtime build validation
bool build_validate_configuration(void) {
    bool valid = true;
    
    // Check version consistency
    if (g_build_info.version_integer != VERSION_AS_INTEGER) {
        DEBUG_PRINT("Version mismatch in build info");
        valid = false;
    }
    
    // Check memory requirements
    uint32_t estimated_usage = g_build_info.heap_size_bytes + 
                              g_build_info.stack_size_bytes * 4; // Assume 4 tasks
    if (estimated_usage > (TARGET_RAM_SIZE_KB * 1024)) {
        DEBUG_PRINT("Memory usage estimate exceeds available RAM");
        valid = false;
    }
    
    // Check feature consistency
    #if FEATURE_ETHERNET_ENABLED
    if (!(g_build_info.features_enabled & FEATURE_BIT_ETHERNET)) {
        DEBUG_PRINT("Ethernet feature mismatch");
        valid = false;
    }
    #endif
    
    return valid;
}

// Get feature status at runtime
bool build_is_feature_enabled(uint32_t feature_bit) {
    return (g_build_info.features_enabled & feature_bit) != 0;
}

// Build information reporting
void build_print_info(void) {
    DEBUG_PRINT("=== Build Information ===");
    DEBUG_PRINT("Version: %s", g_build_info.version_string);
    DEBUG_PRINT("Build: %s %s", g_build_info.build_date, g_build_info.build_time);
    DEBUG_PRINT("Git: %s (%s)%s", g_build_info.git_commit, g_build_info.git_branch,
                g_build_info.git_dirty ? " [dirty]" : "");
    DEBUG_PRINT("Target: %s (%s)", g_build_info.target_mcu, g_build_info.target_board);
    DEBUG_PRINT("Compiler: %s", g_build_info.compiler_version);
    DEBUG_PRINT("HAL: %s", g_build_info.hal_version);
    DEBUG_PRINT("Type: %s", g_build_info.build_type_string);
    DEBUG_PRINT("========================");
}
```

Remember: **Proper build configuration management is essential for maintainability, debugging, and quality assurance. Use feature flags consistently and validate configuration at both compile-time and runtime.**
