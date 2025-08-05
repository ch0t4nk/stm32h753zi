---
applyTo: "src/drivers/**/*.{c,h}"
description: "Hardware configuration, GPIO control, memory management, and STM32H7 HAL guidance for STM32H753ZI stepper motor project"
---

# Hardware & GPIO Instructions

## Overview
This instruction file provides comprehensive hardware configuration guidance for the STM32H753ZI Nucleo-144 board with X-NUCLEO-IHM02A1 expansion shield, including GPIO management, memory configuration, and physical units handling.

## STM32H753ZI Hardware Configuration

### Pin Assignment Reference (SSOT)
**Core System Pins:**
```c
// System Clock and Power
#define HSE_VALUE 25000000UL          // External crystal frequency
#define LSE_VALUE 32768UL             // LSE crystal for RTC
#define VDD_VALUE 3300UL              // Supply voltage in mV

// Debug Interface (ST-Link V3)
#define SWDIO_PIN           GPIO_PIN_13
#define SWDIO_PORT          GPIOA
#define SWCLK_PIN           GPIO_PIN_14
#define SWCLK_PORT          GPIOA
```

### Motor Control Hardware - X-NUCLEO-IHM02A1 Shield

**L6470 Stepper Driver Connections (SPI2 Daisy-Chain):**
```c
// SPI2 Configuration for L6470 Communication
#define L6470_SPI_INSTANCE           SPI2
#define L6470_SPI_CS_PORT            GPIOB
#define L6470_SPI_CS_PIN             GPIO_PIN_12    // CS/NSS
#define L6470_SPI_SCK_PORT           GPIOB
#define L6470_SPI_SCK_PIN            GPIO_PIN_13    // SCLK
#define L6470_SPI_MISO_PORT          GPIOB
#define L6470_SPI_MISO_PIN           GPIO_PIN_14    // MISO
#define L6470_SPI_MOSI_PORT          GPIOB
#define L6470_SPI_MOSI_PIN           GPIO_PIN_15    // MOSI

// L6470 Control Signals
#define L6470_RESET_PORT             GPIOA
#define L6470_RESET_PIN              GPIO_PIN_8     // Active low reset
#define L6470_BUSY_SYNC_PORT         GPIOB
#define L6470_BUSY_SYNC_PIN          GPIO_PIN_3     // Busy/Sync signal
#define L6470_FLAG_PORT              GPIOB
#define L6470_FLAG_PIN               GPIO_PIN_5     // Error flag (active low)

// Motor-specific CS pins for daisy-chain
#define MOTOR1_CS_PORT               GPIOB
#define MOTOR1_CS_PIN                GPIO_PIN_6     // Motor 1 chip select
#define MOTOR2_CS_PORT               GPIOC
#define MOTOR2_CS_PIN                GPIO_PIN_7     // Motor 2 chip select
```

**AS5600 Magnetic Encoder Connections (Dual I2C):**
```c
// I2C1 Configuration for Encoder 1
#define ENCODER1_I2C_INSTANCE        I2C1
#define ENCODER1_I2C_SCL_PORT        GPIOB
#define ENCODER1_I2C_SCL_PIN         GPIO_PIN_8     // I2C1_SCL
#define ENCODER1_I2C_SDA_PORT        GPIOB
#define ENCODER1_I2C_SDA_PIN         GPIO_PIN_9     // I2C1_SDA

// I2C2 Configuration for Encoder 2
#define ENCODER2_I2C_INSTANCE        I2C2
#define ENCODER2_I2C_SCL_PORT        GPIOB
#define ENCODER2_I2C_SCL_PIN         GPIO_PIN_10    // I2C2_SCL
#define ENCODER2_I2C_SDA_PORT        GPIOB
#define ENCODER2_I2C_SDA_PIN         GPIO_PIN_11    // I2C2_SDA

// AS5600 I2C Address
#define AS5600_I2C_ADDRESS           0x36          // 7-bit address
```

### Communication Interfaces

**UART Configuration (ST-Link VCP + External):**
```c
// USART3 - ST-Link Virtual COM Port
#define DEBUG_UART_INSTANCE          USART3
#define DEBUG_UART_TX_PORT           GPIOD
#define DEBUG_UART_TX_PIN            GPIO_PIN_8     // USART3_TX
#define DEBUG_UART_RX_PORT           GPIOD
#define DEBUG_UART_RX_PIN            GPIO_PIN_9     // USART3_RX
#define DEBUG_UART_BAUDRATE          115200

// UART4 - External communication
#define EXT_UART_INSTANCE            UART4
#define EXT_UART_TX_PORT             GPIOA
#define EXT_UART_TX_PIN              GPIO_PIN_0     // UART4_TX
#define EXT_UART_RX_PORT             GPIOA
#define EXT_UART_RX_PIN              GPIO_PIN_1     // UART4_RX
```

**CAN-FD Configuration:**
```c
// FDCAN1 Configuration
#define CAN_INSTANCE                 FDCAN1
#define CAN_TX_PORT                  GPIOD
#define CAN_TX_PIN                   GPIO_PIN_1     // FDCAN1_TX
#define CAN_RX_PORT                  GPIOD
#define CAN_RX_PIN                   GPIO_PIN_0     // FDCAN1_RX
#define CAN_BITRATE_KBPS             500           // 500 kbps nominal
#define CAN_FD_BITRATE_KBPS          2000          // 2 Mbps data phase
```

**Ethernet Configuration (LAN8742 PHY):**
```c
// Ethernet RMII Interface
#define ETH_RMII_REF_CLK_PORT        GPIOA
#define ETH_RMII_REF_CLK_PIN         GPIO_PIN_1     // RMII_REF_CLK
#define ETH_RMII_MDIO_PORT           GPIOA
#define ETH_RMII_MDIO_PIN            GPIO_PIN_2     // RMII_MDIO
#define ETH_RMII_MDC_PORT            GPIOC
#define ETH_RMII_MDC_PIN             GPIO_PIN_1     // RMII_MDC
#define ETH_RMII_CRS_DV_PORT         GPIOA
#define ETH_RMII_CRS_DV_PIN          GPIO_PIN_7     // RMII_CRS_DV
#define ETH_RMII_RXD0_PORT           GPIOC
#define ETH_RMII_RXD0_PIN            GPIO_PIN_4     // RMII_RXD0
#define ETH_RMII_RXD1_PORT           GPIOC
#define ETH_RMII_RXD1_PIN            GPIO_PIN_5     // RMII_RXD1
#define ETH_RMII_TXD0_PORT           GPIOG
#define ETH_RMII_TXD0_PIN            GPIO_PIN_13    // RMII_TXD0
#define ETH_RMII_TXD1_PORT           GPIOB
#define ETH_RMII_TXD1_PIN            GPIO_PIN_13    // RMII_TXD1
#define ETH_RMII_TX_EN_PORT          GPIOG
#define ETH_RMII_TX_EN_PIN           GPIO_PIN_11    // RMII_TX_EN
```

### User Interface Elements

**LEDs (Nucleo-144 Board):**
```c
// User LEDs
#define LED_GREEN_PORT               GPIOB
#define LED_GREEN_PIN                GPIO_PIN_0     // LD1 (Green)
#define LED_YELLOW_PORT              GPIOE
#define LED_YELLOW_PIN               GPIO_PIN_1     // LD2 (Yellow)
#define LED_RED_PORT                 GPIOB
#define LED_RED_PIN                  GPIO_PIN_14    // LD3 (Red)

// Status indication functions
static inline void status_led_on(void) {
    HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
}

static inline void error_led_on(void) {
    HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
}
```

**Push Buttons:**
```c
// User Button (Blue)
#define USER_BUTTON_PORT             GPIOC
#define USER_BUTTON_PIN              GPIO_PIN_13    // B1 (User button)
#define USER_BUTTON_EXTI_LINE        EXTI_LINE_13
#define USER_BUTTON_EXTI_IRQn        EXTI15_10_IRQn

// Emergency stop button (external)
#define ESTOP_BUTTON_PORT            GPIOA
#define ESTOP_BUTTON_PIN             GPIO_PIN_15    // External E-stop
#define ESTOP_BUTTON_EXTI_LINE       EXTI_LINE_15
#define ESTOP_BUTTON_EXTI_IRQn       EXTI15_10_IRQn
```

## STM32H7 GPIO HAL Implementation

### GPIO Initialization Patterns

**Standard GPIO Configuration:**
```c
/**
 * @brief Initialize GPIO pin for digital output
 * @param port GPIO port (GPIOA, GPIOB, etc.)
 * @param pin GPIO pin (GPIO_PIN_0, GPIO_PIN_1, etc.)
 * @param initial_state Initial pin state (GPIO_PIN_SET/GPIO_PIN_RESET)
 */
void gpio_init_output(GPIO_TypeDef* port, uint32_t pin, GPIO_PinState initial_state) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clock based on port
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (port == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (port == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
    
    // Set initial state before configuring as output
    HAL_GPIO_WritePin(port, pin, initial_state);
    
    // Configure GPIO
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

/**
 * @brief Initialize GPIO pin for digital input with optional pull-up/down
 * @param port GPIO port
 * @param pin GPIO pin  
 * @param pull Pull configuration (GPIO_NOPULL, GPIO_PULLUP, GPIO_PULLDOWN)
 */
void gpio_init_input(GPIO_TypeDef* port, uint32_t pin, uint32_t pull) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Enable GPIO clock (same pattern as output)
    gpio_enable_port_clock(port);
    
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = pull;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}
```

**External Interrupt Configuration:**
```c
/**
 * @brief Configure GPIO pin for external interrupt
 * @param port GPIO port
 * @param pin GPIO pin
 * @param trigger Trigger type (GPIO_MODE_IT_RISING, GPIO_MODE_IT_FALLING, GPIO_MODE_IT_RISING_FALLING)
 * @param pull Pull configuration
 * @param priority Interrupt priority (0-15, 0 is highest)
 */
void gpio_init_interrupt(GPIO_TypeDef* port, uint32_t pin, uint32_t trigger, 
                        uint32_t pull, uint32_t priority) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    gpio_enable_port_clock(port);
    
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = trigger;
    GPIO_InitStruct.Pull = pull;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    // Configure NVIC
    IRQn_Type IRQn = gpio_get_exti_irqn(pin);
    HAL_NVIC_SetPriority(IRQn, priority, 0);
    HAL_NVIC_EnableIRQ(IRQn);
}
```

### Alternate Function Configuration

**SPI Alternate Function Setup:**
```c
/**
 * @brief Configure GPIO pins for SPI alternate function
 * @param spi_instance SPI instance (SPI1, SPI2, etc.)
 */
void gpio_init_spi_af(SPI_TypeDef* spi_instance) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (spi_instance == SPI2) {
        // Enable GPIO clocks
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        // Configure SPI2 pins (PB12-PB15)
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;  // SCK, MISO, MOSI
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        
        // Configure CS pin as regular output (manual control)
        gpio_init_output(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    }
}
```

**I2C Alternate Function Setup:**
```c
/**
 * @brief Configure GPIO pins for I2C alternate function
 * @param i2c_instance I2C instance (I2C1, I2C2, etc.)
 */
void gpio_init_i2c_af(I2C_TypeDef* i2c_instance) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if (i2c_instance == I2C1) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        // Configure I2C1 pins (PB8: SCL, PB9: SDA)
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;  // Open drain for I2C
        GPIO_InitStruct.Pull = GPIO_PULLUP;      // Enable pull-ups
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else if (i2c_instance == I2C2) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        
        // Configure I2C2 pins (PB10: SCL, PB11: SDA)
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}
```

## Memory Configuration

### STM32H753ZI Memory Layout
```c
// Memory region definitions for linker script
#define FLASH_BASE_ADDR              0x08000000
#define FLASH_SIZE                   0x200000    // 2MB Flash
#define DTCMRAM_BASE_ADDR           0x20000000
#define DTCMRAM_SIZE                0x20000     // 128KB DTCM RAM
#define RAM_D1_BASE_ADDR            0x24000000
#define RAM_D1_SIZE                 0x80000     // 512KB AXI SRAM
#define RAM_D2_BASE_ADDR            0x30000000
#define RAM_D2_SIZE                 0x48000     // 288KB AHB SRAM
#define RAM_D3_BASE_ADDR            0x38000000
#define RAM_D3_SIZE                 0x10000     // 64KB AHB SRAM
#define ITCMRAM_BASE_ADDR           0x00000000
#define ITCMRAM_SIZE                0x10000     // 64KB ITCM RAM
```

**Memory Section Attributes:**
```c
// Fast data access (motor control variables)
#define DTCM_SECTION __attribute__((section(".dtcmram")))
DTCM_SECTION volatile uint32_t motor_position_data[MAX_MOTORS];

// Fast code execution (interrupt handlers)
#define ITCM_SECTION __attribute__((section(".itcmram")))
ITCM_SECTION void motor_control_interrupt_handler(void);

// DMA-accessible memory (communication buffers)
#define DMA_SECTION __attribute__((section(".dma_buffer")))
DMA_SECTION uint8_t spi_tx_buffer[256];
DMA_SECTION uint8_t spi_rx_buffer[256];
```

### Memory Management Best Practices

**Stack and Heap Configuration:**
```c
// Linker script memory allocation
_Min_Heap_Size = 0x4000;   // 16KB heap
_Min_Stack_Size = 0x2000;  // 8KB stack

// Stack usage monitoring
void monitor_stack_usage(void) {
    extern uint32_t _estack;
    extern uint32_t _Min_Stack_Size;
    
    uint32_t stack_top = (uint32_t)&_estack;
    uint32_t current_sp;
    
    __asm volatile ("mov %0, sp" : "=r" (current_sp));
    
    uint32_t stack_used = stack_top - current_sp;
    uint32_t stack_free = (uint32_t)&_Min_Stack_Size - stack_used;
    
    if (stack_free < 512) {  // Less than 512 bytes free
        // Log stack overflow warning
        error_log(ERROR_STACK_OVERFLOW, stack_free);
    }
}
```

**Cache Management:**
```c
/**
 * @brief Initialize CPU caches for optimal performance
 */
void cpu_cache_init(void) {
    // Enable I-Cache
    SCB_EnableICache();
    
    // Enable D-Cache
    SCB_EnableDCache();
    
    // Configure MPU for cache management if needed
    mpu_configure_cache_regions();
}

/**
 * @brief Clean and invalidate DMA buffers
 * @param buffer Pointer to buffer
 * @param size Buffer size in bytes
 */
void cache_clean_invalidate_dma_buffer(void* buffer, uint32_t size) {
    uint32_t addr = (uint32_t)buffer;
    
    // Align to cache line boundary (32 bytes for Cortex-M7)
    uint32_t aligned_addr = addr & ~0x1F;
    uint32_t aligned_size = ((size + (addr - aligned_addr) + 31) & ~0x1F);
    
    SCB_CleanInvalidateDCache_by_Addr((uint32_t*)aligned_addr, aligned_size);
}
```

## Physical Units and Measurements

### Unit System Definitions
```c
// Base units for motor control system
typedef enum {
    UNIT_DEGREES,           // Angular position in degrees
    UNIT_RADIANS,           // Angular position in radians  
    UNIT_STEPS,             // Stepper motor steps
    UNIT_RPM,               // Rotational speed in RPM
    UNIT_DEG_PER_SEC,       // Angular velocity in deg/s
    UNIT_RAD_PER_SEC,       // Angular velocity in rad/s
    UNIT_STEPS_PER_SEC,     // Step rate
    UNIT_VOLTS,             // Voltage measurements
    UNIT_AMPERES,           // Current measurements
    UNIT_CELSIUS,           // Temperature measurements
    UNIT_MILLISECONDS,      // Time measurements
    UNIT_MICROSECONDS       // High precision timing
} PhysicalUnit_t;

// Measurement structure with units
typedef struct {
    float value;            // Numerical value
    PhysicalUnit_t unit;    // Physical unit
    float precision;        // Measurement precision/resolution
    uint32_t timestamp_ms;  // When measurement was taken
    bool valid;             // Measurement validity flag
} Measurement_t;
```

### Unit Conversion Functions
```c
// Angular unit conversions
static inline float degrees_to_radians(float degrees) {
    return degrees * (M_PI / 180.0f);
}

static inline float radians_to_degrees(float radians) {
    return radians * (180.0f / M_PI);
}

static inline float degrees_to_steps(float degrees, float steps_per_revolution) {
    return (degrees / 360.0f) * steps_per_revolution;
}

static inline float steps_to_degrees(float steps, float steps_per_revolution) {
    return (steps / steps_per_revolution) * 360.0f;
}

// Velocity conversions
static inline float rpm_to_deg_per_sec(float rpm) {
    return rpm * 6.0f;  // 360 deg/rev * 1 min/60 sec
}

static inline float deg_per_sec_to_rpm(float deg_per_sec) {
    return deg_per_sec / 6.0f;
}

// Motor-specific conversions using SSOT configuration
static inline float encoder_counts_to_degrees(uint16_t counts) {
    return ((float)counts / 4095.0f) * 360.0f;  // AS5600 12-bit = 4095 counts
}

static inline uint16_t degrees_to_encoder_counts(float degrees) {
    return (uint16_t)((degrees / 360.0f) * 4095.0f + 0.5f);  // Round to nearest
}
```

### Measurement Validation and Filtering
```c
/**
 * @brief Validate measurement within expected range
 * @param measurement Measurement to validate
 * @param min_value Minimum valid value
 * @param max_value Maximum valid value
 * @return true if valid, false otherwise
 */
bool validate_measurement(const Measurement_t* measurement, 
                         float min_value, float max_value) {
    if (!measurement->valid) {
        return false;
    }
    
    if (measurement->value < min_value || measurement->value > max_value) {
        return false;
    }
    
    // Check timestamp freshness (within last 100ms)
    uint32_t current_time = HAL_GetTick();
    if ((current_time - measurement->timestamp_ms) > 100) {
        return false;
    }
    
    return true;
}

/**
 * @brief Apply low-pass filter to measurement
 * @param new_measurement New measurement value
 * @param filtered_value Previous filtered value  
 * @param alpha Filter coefficient (0.0 to 1.0)
 * @return Filtered measurement value
 */
float apply_low_pass_filter(float new_measurement, float filtered_value, float alpha) {
    return alpha * new_measurement + (1.0f - alpha) * filtered_value;
}
```

## Hardware Abstraction Guidelines

### Peripheral Initialization Order
```c
/**
 * @brief Initialize hardware peripherals in correct order
 */
void hardware_init_sequence(void) {
    // 1. System clock configuration (highest priority)
    system_clock_config();
    
    // 2. Enable CPU caches
    cpu_cache_init();
    
    // 3. Initialize GPIO clocks and basic pins
    gpio_init_system_pins();
    
    // 4. Initialize communication peripherals
    spi_init_l6470_interface();
    i2c_init_encoder_interfaces();
    uart_init_debug_interface();
    
    // 5. Initialize motor control hardware
    l6470_init_drivers();
    as5600_init_encoders();
    
    // 6. Initialize safety and monitoring systems
    safety_system_init();
    watchdog_init();
    
    // 7. Start control loops and interrupts
    motor_control_timer_init();
    enable_motor_control_interrupts();
}
```

### Error Handling for Hardware Operations
```c
/**
 * @brief Safe GPIO write with error checking
 * @param port GPIO port
 * @param pin GPIO pin
 * @param state Desired pin state
 * @return SystemError_t Success or error code
 */
SystemError_t gpio_write_safe(GPIO_TypeDef* port, uint32_t pin, GPIO_PinState state) {
    // Validate parameters
    if (port == NULL) {
        return ERROR_NULL_POINTER;
    }
    
    if (!is_valid_gpio_pin(pin)) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Check if GPIO clock is enabled
    if (!is_gpio_clock_enabled(port)) {
        return ERROR_NOT_INITIALIZED;
    }
    
    // Perform write operation
    HAL_GPIO_WritePin(port, pin, state);
    
    // Verify write operation (read back)
    GPIO_PinState actual_state = HAL_GPIO_ReadPin(port, pin);
    if (actual_state != state) {
        return ERROR_HARDWARE_FAILURE;
    }
    
    return SYSTEM_OK;
}
```

Remember: **Precise hardware configuration enables reliable motor control, proper memory management ensures performance, validated measurements guarantee safety.**
