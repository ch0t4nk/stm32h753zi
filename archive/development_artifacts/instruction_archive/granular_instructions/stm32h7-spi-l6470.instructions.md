---
applyTo: "src/drivers/l6470_driver.c"
description: "STM32H7 SPI HAL implementation for L6470 stepper drivers with daisy-chain support"
---

# STM32H7 SPI HAL Implementation for L6470 Drivers

## Overview
This instruction file provides STM32H753ZI-specific SPI HAL implementation guidance for L6470 stepper motor driver communication, with focus on daisy-chain operation required by X-NUCLEO-IHM02A1 shield.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive SPI and L6470 documentation:
- **STM32H7 SPI Documentation**: Complete SPI peripheral coverage with semantic categorization
- **Copilot-Optimized SPI Search**:
  ```bash
  # Search SPI peripheral with token efficiency
  python3 scripts/search_enhanced_docs.py peripheral SPI --scope STM32H7
  
  # Find specific SPI HAL functions
  python3 scripts/search_enhanced_docs.py function HAL_SPI_Transmit --scope STM32H7
  
  # Search SPI daisy-chain concepts across all documentation
  python3 scripts/search_enhanced_docs.py concept "spi_daisy_chain" --scope all
  
  # Find L6470-specific API functions (NOTE: functions are concatenated)
  python3 scripts/search_enhanced_docs.py function L6470 --scope L6470
  ```
- **Search Pattern Guidelines**:
  - ✅ STM32H7: Use exact function names `HAL_SPI_Init`, `HAL_SPI_Transmit`
  - ✅ L6470: Use broad patterns `L6470` (functions like `voidL6470_GoHome` are concatenated)
  - ✅ Concepts: Use semantic terms `spi_daisy_chain`, `motor_control`
  - ❌ Don't search for `L6470_Init` - it's stored as `voidL6470_Init` in index
- **Cross-Domain Integration**: Unified STM32H7 SPI + L6470 driver documentation search
- **Token-Efficient Lookup**: Semantic categorization reduces Copilot search overhead for SPI implementation

## Integration with Existing Instructions
- **Extends**: `l6470-registers.instructions.md` (L6470 configuration)
- **Extends**: `hardware-pins.instructions.md` (SPI pin assignments)
- **References**: `ssot-config.instructions.md` (SSOT principles)
- **Documentation**: Based on enhanced search system with STM32H7 + L6470 coverage

## Critical SPI Configuration for L6470

### SPI Handle Configuration (from structSPI__InitTypeDef.md)
```c
#include "config/hardware_config.h"
#include "config/motor_config.h"

SPI_HandleTypeDef hspi2;

// L6470 requires SPI Mode 3 (CPOL=1, CPHA=1)
SystemError_t spi_l6470_init(void) {
    hspi2.Instance = SPI2;                          // From SSOT hardware_config.h
    hspi2.Init.Mode = SPI_MODE_MASTER;              // MCU is master
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;    // Full duplex
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;        // 8-bit data
    hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;     // CPOL=1 for L6470
    hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;          // CPHA=1 for L6470
    hspi2.Init.NSS = SPI_NSS_SOFT;                  // Software CS control
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;  // ~7.5MHz from 240MHz
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;         // MSB first
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    
    // STM32H7-specific features
    hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;    // No NSS pulse
    hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
    hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
    
    if (HAL_SPI_Init(&hspi2) != HAL_OK) {
        return ERROR_SPI_INIT_FAILED;
    }
    
    return SYSTEM_OK;
}
```

### L6470 Daisy-Chain Communication Protocol
```c
// Daisy-chain requires all devices receive command simultaneously
// Motor 1 is first in chain, Motor 2 is second

typedef struct {
    uint8_t motor1_command;     // First L6470 (closest to MCU)
    uint8_t motor2_command;     // Second L6470 (end of chain)
    uint8_t motor1_data[3];     // Command data for motor 1
    uint8_t motor2_data[3];     // Command data for motor 2
} L6470_DaisyChain_t;

// Send command to both motors in daisy-chain
SystemError_t l6470_send_daisy_command(uint8_t motor1_cmd, uint32_t motor1_data,
                                       uint8_t motor2_cmd, uint32_t motor2_data) {
    uint8_t tx_buffer[8];
    uint8_t rx_buffer[8];
    
    // Build transmission buffer (motor 2 data sent first in daisy-chain)
    tx_buffer[0] = motor2_cmd;                    // Motor 2 command
    tx_buffer[1] = (motor2_data >> 16) & 0xFF;    // Motor 2 data high
    tx_buffer[2] = (motor2_data >> 8) & 0xFF;     // Motor 2 data mid
    tx_buffer[3] = motor2_data & 0xFF;            // Motor 2 data low
    tx_buffer[4] = motor1_cmd;                    // Motor 1 command
    tx_buffer[5] = (motor1_data >> 16) & 0xFF;    // Motor 1 data high
    tx_buffer[6] = (motor1_data >> 8) & 0xFF;     // Motor 1 data mid
    tx_buffer[7] = motor1_data & 0xFF;            // Motor 1 data low
    
    // Assert CS (active low)
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_RESET);
    
    // SPI transaction with timeout
    HAL_StatusTypeDef spi_status = HAL_SPI_TransmitReceive(&hspi2, tx_buffer, rx_buffer, 8, SPI_TIMEOUT_MS);
    
    // Deassert CS
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_SET);
    
    if (spi_status != HAL_OK) {
        return ERROR_SPI_TRANSMISSION_FAILED;
    }
    
    return SYSTEM_OK;
}
```

### Single Motor Command (Daisy-Chain Compatible)
```c
// Send command to single motor while maintaining daisy-chain
SystemError_t l6470_send_single_command(uint8_t motor_id, uint8_t command, uint32_t data) {
    if (motor_id == 1) {
        // Command motor 1, send NOP to motor 2
        return l6470_send_daisy_command(command, data, L6470_NOP, 0);
    } else if (motor_id == 2) {
        // Command motor 2, send NOP to motor 1
        return l6470_send_daisy_command(L6470_NOP, 0, command, data);
    } else {
        return ERROR_MOTOR_INVALID_ID;
    }
}
```

## STM32H753ZI SPI Timing Considerations

### Clock Configuration Validation
```c
// Ensure SPI clock doesn't exceed L6470 maximum (5MHz)
SystemError_t validate_spi_clock_frequency(void) {
    uint32_t pclk1_freq = HAL_RCC_GetPCLK1Freq();  // Get peripheral clock
    uint32_t spi_freq = pclk1_freq / 32;            // Our prescaler setting
    
    if (spi_freq > L6470_MAX_SPI_FREQUENCY) {       // 5MHz from SSOT
        return ERROR_SPI_FREQUENCY_TOO_HIGH;
    }
    
    return SYSTEM_OK;
}
```

### DMA Configuration for High-Performance SPI
```c
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi2_rx;

// Configure DMA for efficient SPI transfers
void spi_configure_dma(void) {
    // Enable DMA clock
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    // SPI2 TX DMA configuration
    hdma_spi2_tx.Instance = DMA1_Stream4;
    hdma_spi2_tx.Init.Request = DMA_REQUEST_SPI2_TX;
    hdma_spi2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi2_tx.Init.Mode = DMA_NORMAL;
    hdma_spi2_tx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_spi2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    HAL_DMA_Init(&hdma_spi2_tx);
    __HAL_LINKDMA(&hspi2, hdmatx, hdma_spi2_tx);
    
    // Configure RX DMA similarly...
}
```

## L6470 Status Reading with Error Handling

### Status Register Reading
```c
// Read L6470 status registers with comprehensive error checking
SystemError_t l6470_read_status(uint8_t motor_id, uint16_t* status) {
    uint8_t tx_data[2] = {L6470_GET_STATUS, 0x00};
    uint8_t rx_data[2];
    
    // Assert CS
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_RESET);
    
    // Small delay for L6470 setup time (from datasheet)
    HAL_Delay_us(1);  // Custom microsecond delay function
    
    HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(&hspi2, tx_data, rx_data, 2, SPI_TIMEOUT_MS);
    
    // Deassert CS
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_SET);
    
    if (result != HAL_OK) {
        return ERROR_SPI_TRANSMISSION_FAILED;
    }
    
    *status = (rx_data[0] << 8) | rx_data[1];
    
    // Check for L6470 error flags
    if (*status & L6470_STATUS_UVLO) {
        return ERROR_MOTOR_UNDERVOLTAGE;
    }
    if (*status & L6470_STATUS_OCD) {
        return ERROR_MOTOR_OVERCURRENT;
    }
    if (*status & L6470_STATUS_TH_SD) {
        return ERROR_MOTOR_THERMAL_SHUTDOWN;
    }
    
    return SYSTEM_OK;
}
```

## Integration with Safety Systems

### SPI Communication Monitoring
```c
#include "config/safety_config.h"

// Monitor SPI communication health
typedef struct {
    uint32_t transaction_count;
    uint32_t error_count;
    uint32_t last_error_time;
    SystemError_t last_error_code;
} SPI_HealthMonitor_t;

SPI_HealthMonitor_t spi_health = {0};

SystemError_t spi_safe_transaction(uint8_t* tx_data, uint8_t* rx_data, uint16_t size) {
    // Feed watchdog before critical SPI operation
    HAL_IWDG_Refresh(&hiwdg);
    
    spi_health.transaction_count++;
    
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_RESET);
    HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(&hspi2, tx_data, rx_data, size, SPI_TIMEOUT_MS);
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_SET);
    
    if (result != HAL_OK) {
        spi_health.error_count++;
        spi_health.last_error_time = HAL_GetTick();
        spi_health.last_error_code = ERROR_SPI_TRANSMISSION_FAILED;
        
        // Check if error rate is too high
        if (spi_health.error_count * 100 / spi_health.transaction_count > SPI_MAX_ERROR_RATE_PERCENT) {
            system_trigger_safe_stop();  // From safety_config.h
            return ERROR_SPI_RELIABILITY_DEGRADED;
        }
        
        return ERROR_SPI_TRANSMISSION_FAILED;
    }
    
    return SYSTEM_OK;
}
```

## Performance Optimization

### Interrupt-Based SPI for Real-Time Operation
```c
// Use SPI interrupts for non-blocking operation
volatile uint8_t spi_transfer_complete = 0;

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI2) {
        spi_transfer_complete = 1;
        HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_SET);
    }
}

SystemError_t l6470_send_command_async(uint8_t motor_id, uint8_t command, uint32_t data) {
    if (spi_transfer_complete == 0) {
        return ERROR_SPI_BUSY;  // Previous transfer still in progress
    }
    
    // Prepare data and start transfer
    // ... prepare tx_buffer ...
    
    spi_transfer_complete = 0;
    HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_RESET);
    
    if (HAL_SPI_TransmitReceive_IT(&hspi2, tx_buffer, rx_buffer, buffer_size) != HAL_OK) {
        HAL_GPIO_WritePin(MOTOR_SPI_CS_PORT, MOTOR_SPI_CS_PIN, GPIO_PIN_SET);
        spi_transfer_complete = 1;
        return ERROR_SPI_TRANSMISSION_FAILED;
    }
    
    return SYSTEM_OK;
}
```

## Documentation References
Based on STM32H753ZI documentation analysis (199 SPI-related files):

**Key Reference Files:**
- `group__SPI__Exported__Functions__Group1.md` (39.2KB) - Primary SPI functions
- `structSPI__InitTypeDef.md` (27.7KB) - SPI initialization structure
- `group__SPIEx__Exported__Functions__Group1.md` (10.8KB) - Extended SPI functions
- `group__HAL__Exported__Functions.md` (60.6KB) - General HAL functions
- `group__DMA__Exported__Functions__Group3.md` (7.1KB) - DMA integration

**Total Documentation Coverage**: 144.7KB of focused SPI implementation guidance from STM32H753ZI-specific HAL documentation.

## Critical L6470 Timing Requirements
- **CS Setup Time**: ≥1µs before first clock edge
- **CS Hold Time**: ≥1µs after last clock edge  
- **Maximum SPI Clock**: 5MHz (datasheet limit)
- **Inter-byte Delay**: None required for continuous operation
- **Status Flag Update**: Allow 1ms after command for status flag updates
