/**
 * @file hal_abstraction_demo.c
 * @brief Demonstration of HAL Abstraction Layer Usage
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * This file demonstrates how to use the HAL abstraction layer
 * for clean, testable embedded code.
 */

#include "common/error_codes.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "drivers/as5600/as5600_driver.h"
#include "hal_abstraction/hal_abstraction.h"

/**
 * @brief Demo function showing GPIO operations through HAL abstraction
 */
SystemError_t demo_gpio_operations(void) {
  SystemError_t result;

  // Configure LED pin as output
  HAL_GPIO_Config_t led_config = {.pin = LED_USER_PIN,
                                  .mode = GPIO_MODE_OUTPUT_PP,
                                  .pull = GPIO_NOPULL,
                                  .speed = GPIO_SPEED_FREQ_LOW,
                                  .alternate = 0};

  result = HAL_Abstraction_GPIO_Init(LED_USER_PORT, &led_config);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Turn LED on
  result = HAL_Abstraction_GPIO_Write(LED_USER_PORT, LED_USER_PIN,
                                      HAL_GPIO_STATE_SET);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Wait 1 second
  HAL_Abstraction_Delay(1000);

  // Turn LED off
  result = HAL_Abstraction_GPIO_Write(LED_USER_PORT, LED_USER_PIN,
                                      HAL_GPIO_STATE_RESET);

  return result;
}

/**
 * @brief Demo function showing SPI operations through HAL abstraction
 */
SystemError_t demo_spi_operations(void) {
  SystemError_t result;

  // Test SPI communication (simulated L6470 command)
  uint8_t tx_data[] = {L6470_CMD_GETPARAM, L6470_CMD_NOP,
                       L6470_CMD_NOP}; // GETPARAM command
  uint8_t rx_data[3] = {0};

  HAL_SPI_Transaction_t transaction = {.tx_data = tx_data,
                                       .rx_data = rx_data,
                                       .size = sizeof(tx_data),
                                       .timeout_ms = 1000};

  result =
      HAL_Abstraction_SPI_TransmitReceive(MOTOR1_SPI_INSTANCE, &transaction);

  return result;
}

/**
 * @brief Demo function showing I2C operations through HAL abstraction
 */
SystemError_t demo_i2c_operations(void) {
  SystemError_t result;

  // Test I2C communication (simulated AS5600 read)
  uint8_t register_addr = AS5600_REG_RAW_ANGLE_H; // AS5600 angle register
  uint8_t angle_data[2] = {0};

  HAL_I2C_Transaction_t transaction = {.device_address = ENCODER1_I2C_ADDRESS,
                                       .tx_data = &register_addr,
                                       .tx_size = 1,
                                       .rx_data = angle_data,
                                       .rx_size = sizeof(angle_data),
                                       .timeout_ms = 1000};

  result =
      HAL_Abstraction_I2C_TransmitReceive(ENCODER1_I2C_INSTANCE, &transaction);

  return result;
}

/**
 * @brief Demo function showing timer operations through HAL abstraction
 */
SystemError_t demo_timer_operations(void) {
  SystemError_t result;

  // Configure timer for 1ms interrupts
  HAL_Timer_Config_t timer_config = {
      .prescaler = 200,                          // Assuming 200MHz clock
      .period = (uint32_t)DEMO_TIMER_PERIOD_1MS, // 1ms period
      .clock_division = 0,
      .counter_mode = TIMER_COUNTERMODE_UP};

  result = HAL_Abstraction_Timer_Init(HAL_TIMER_2, &timer_config);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Start timer
  result = HAL_Abstraction_Timer_Start(HAL_TIMER_2);

  return result;
}

/**
 * @brief Main demonstration function
 */
SystemError_t hal_abstraction_demo(void) {
  SystemError_t result;

  // Initialize HAL abstraction
  result = HAL_Abstraction_Init();
  if (result != SYSTEM_OK) {
    return result;
  }

  // Demo GPIO operations
  result = demo_gpio_operations();
  if (result != SYSTEM_OK) {
    return result;
  }

  // Demo SPI operations
  result = demo_spi_operations();
  if (result != SYSTEM_OK) {
    return result;
  }

  // Demo I2C operations
  result = demo_i2c_operations();
  if (result != SYSTEM_OK) {
    return result;
  }

  // Demo timer operations
  result = demo_timer_operations();

  return result;
}
