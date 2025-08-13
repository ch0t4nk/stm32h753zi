/**
 * @file telemetry_dashboard.h
 * @brief Real-Time Telemetry Dashboard and HTTP JSON API
 * @author STM32H753ZI Project Team
 * @date August 2025
 * @version 1.0
 *
 * This module provides a lightweight HTTP server with JSON API for real-time
 * motor control telemetry, performance metrics, and system diagnostics over
 * STLink USB CDC interface with industrial-grade monitoring capabilities.
 */

#ifndef TELEMETRY_DASHBOARD_H
#define TELEMETRY_DASHBOARD_H

/* ============================================================================
 */
/* Includes */
/* ============================================================================
 */

#include "common/error_codes.h"
#include "config/freertos_config_ssot.h"
#include "dynamic_task_tuning.h"
#include "power_management.h"

#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 */
/* Configuration Constants */
/* ============================================================================
 */

/// @brief Maximum HTTP request size
#define TELEMETRY_MAX_REQUEST_SIZE 2048

/// @brief Maximum HTTP response size
#define TELEMETRY_MAX_RESPONSE_SIZE 4096

/// @brief Maximum number of concurrent connections
#define TELEMETRY_MAX_CONNECTIONS 4

/// @brief HTTP server port (simulated over USB CDC)
#define TELEMETRY_HTTP_PORT 8080

/// @brief Telemetry update interval
#define TELEMETRY_UPDATE_INTERVAL_MS 500

/// @brief Maximum JSON payload size
#define TELEMETRY_MAX_JSON_SIZE 3072

/// @brief Dashboard web interface buffer size
#define TELEMETRY_WEB_BUFFER_SIZE 8192

/* ============================================================================
 */
/* Data Structures                                                           */
/* ============================================================================
 */

/// @brief Motor telemetry data
typedef struct {
  uint8_t motor_id;           ///< Motor identifier (0-1)
  float current_position_deg; ///< Current position in degrees
  float target_position_deg;  ///< Target position in degrees
  float current_speed_rpm;    ///< Current speed in RPM
  float target_speed_rpm;     ///< Target speed in RPM
  float motor_current_a;      ///< Motor current in amperes
  float motor_voltage_v;      ///< Motor voltage in volts
  uint32_t step_count;        ///< Total step count
  uint32_t fault_flags;       ///< L6470 fault flags
  bool enabled;               ///< Motor enabled status
  bool moving;                ///< Motor is moving
  bool at_target;             ///< Motor at target position
  uint32_t last_update_ms;    ///< Last update timestamp
} MotorTelemetryData_t;

/// @brief System telemetry data
typedef struct {
  uint32_t uptime_ms;                ///< System uptime in milliseconds
  uint32_t cpu_utilization_percent;  ///< CPU utilization percentage
  uint32_t free_heap_bytes;          ///< Free heap memory
  uint32_t min_stack_bytes;          ///< Minimum stack space across tasks
  float cpu_temperature_c;           ///< CPU temperature in Celsius
  float supply_voltage_v;            ///< Supply voltage in volts
  uint32_t context_switches_per_sec; ///< Context switches per second
  uint32_t total_tasks;              ///< Total number of tasks
  uint32_t active_tasks;             ///< Number of active tasks
  PowerMode_t power_mode;            ///< Current power mode
  uint32_t power_savings_mw;         ///< Estimated power savings in mW
  uint32_t last_update_ms;           ///< Last update timestamp
} SystemTelemetryData_t;

/// @brief Communication telemetry data
typedef struct {
  uint32_t can_messages_tx; ///< CAN messages transmitted
  uint32_t can_messages_rx; ///< CAN messages received
  uint32_t can_errors;      ///< CAN communication errors
  uint32_t uart_bytes_tx;   ///< UART bytes transmitted
  uint32_t uart_bytes_rx;   ///< UART bytes received
  uint32_t uart_errors;     ///< UART communication errors
  uint32_t usb_bytes_tx;    ///< USB CDC bytes transmitted
  uint32_t usb_bytes_rx;    ///< USB CDC bytes received
  uint32_t usb_errors;      ///< USB communication errors
  bool can_connected;       ///< CAN bus connected
  bool uart_connected;      ///< UART connected
  bool usb_connected;       ///< USB connected
  uint32_t last_update_ms;  ///< Last update timestamp
} CommTelemetryData_t;

/// @brief Safety telemetry data
typedef struct {
  bool emergency_stop_active;      ///< Emergency stop status
  bool watchdog_active;            ///< Watchdog status
  uint32_t safety_violations;      ///< Number of safety violations
  uint32_t warning_count;          ///< Number of warnings
  float max_motor_current_a;       ///< Maximum motor current observed
  float max_cpu_temperature_c;     ///< Maximum CPU temperature observed
  uint32_t fault_recovery_count;   ///< Number of fault recoveries
  uint32_t system_resets;          ///< Number of system resets
  uint32_t last_safety_event_ms;   ///< Last safety event timestamp
  const char *last_safety_message; ///< Last safety message
  uint32_t last_update_ms;         ///< Last update timestamp
} SafetyTelemetryData_t;

/// @brief Performance metrics telemetry
typedef struct {
  uint32_t motor_control_frequency;  ///< Motor control loop frequency
  uint32_t motor_control_jitter_us;  ///< Motor control timing jitter
  uint32_t safety_check_frequency;   ///< Safety check frequency
  uint32_t communication_frequency;  ///< Communication frequency
  uint32_t max_interrupt_latency_us; ///< Maximum interrupt latency
  uint32_t memory_fragmentation_pct; ///< Memory fragmentation percentage
  uint32_t disk_usage_percent; ///< Storage usage percentage (if applicable)
  uint32_t network_latency_ms; ///< Network latency (if applicable)
  uint32_t last_update_ms;     ///< Last update timestamp
} PerformanceTelemetryData_t;

/// @brief Complete telemetry snapshot
typedef struct {
  MotorTelemetryData_t motors[2];         ///< Motor telemetry for both motors
  SystemTelemetryData_t system;           ///< System telemetry
  CommTelemetryData_t communication;      ///< Communication telemetry
  SafetyTelemetryData_t safety;           ///< Safety telemetry
  PerformanceTelemetryData_t performance; ///< Performance telemetry
  uint32_t snapshot_timestamp;            ///< Snapshot timestamp
  uint32_t snapshot_id;                   ///< Unique snapshot identifier
} TelemetrySnapshot_t;

/// @brief HTTP request structure
typedef struct {
  char method[16];        ///< HTTP method (GET, POST, etc.)
  char path[256];         ///< Request path
  char query_string[512]; ///< Query string parameters
  char *body;             ///< Request body (for POST)
  uint32_t body_length;   ///< Body length
  uint32_t connection_id; ///< Connection identifier
} HttpRequest_t;

/// @brief HTTP response structure
typedef struct {
  uint16_t status_code;  ///< HTTP status code
  char content_type[64]; ///< Content type header
  char *body;            ///< Response body
  uint32_t body_length;  ///< Body length
  bool connection_close; ///< Close connection after response
} HttpResponse_t;

/// @brief Dashboard configuration
typedef struct {
  bool enabled;                      ///< Dashboard enabled
  uint32_t update_interval_ms;       ///< Telemetry update interval
  uint32_t history_depth;            ///< Historical data depth
  bool enable_motor_telemetry;       ///< Enable motor telemetry
  bool enable_system_telemetry;      ///< Enable system telemetry
  bool enable_safety_telemetry;      ///< Enable safety telemetry
  bool enable_performance_telemetry; ///< Enable performance telemetry
  bool enable_web_interface;         ///< Enable web interface
  bool enable_json_api;              ///< Enable JSON API
  const char *dashboard_title;       ///< Dashboard title
} DashboardConfiguration_t;

/* ============================================================================
 */
/* Public API Functions */
/* ============================================================================
 */

/**
 * @brief Initialize telemetry dashboard system
 * @param config Dashboard configuration
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_001: Initialize telemetry dashboard
 * @misra Rule 8.4: External function declarations
 * @sil SIL-2: Safe initialization with validation
 */
SystemError_t telemetry_dashboard_init(const DashboardConfiguration_t *config);

/**
 * @brief Start telemetry dashboard HTTP server
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_002: Start HTTP server for dashboard
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_start(void);

/**
 * @brief Stop telemetry dashboard HTTP server
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_003: Stop HTTP server
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_stop(void);

/**
 * @brief Update telemetry data
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_004: Update telemetry data periodically
 * @note Called from telemetry task to gather current data
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_update(void);

/**
 * @brief Process HTTP request
 * @param request HTTP request structure
 * @param response HTTP response structure to fill
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_005: Process HTTP requests
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_process_request(const HttpRequest_t *request,
                                                  HttpResponse_t *response);

/**
 * @brief Get current telemetry snapshot
 * @param snapshot Pointer to snapshot structure to fill
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_006: Provide telemetry snapshot
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_get_snapshot(TelemetrySnapshot_t *snapshot);

/**
 * @brief Generate JSON telemetry data
 * @param json_buffer Buffer to write JSON data
 * @param buffer_size Size of JSON buffer
 * @param include_history Include historical data
 * @return Number of bytes written, or 0 on error
 * @req REQ_TELEMETRY_007: Generate JSON telemetry
 * @misra Rule 8.4: External function declarations
 */
uint32_t telemetry_dashboard_generate_json(char *json_buffer,
                                           uint32_t buffer_size,
                                           bool include_history);

/**
 * @brief Generate HTML dashboard interface
 * @param html_buffer Buffer to write HTML data
 * @param buffer_size Size of HTML buffer
 * @return Number of bytes written, or 0 on error
 * @req REQ_TELEMETRY_008: Generate HTML dashboard
 * @misra Rule 8.4: External function declarations
 */
uint32_t telemetry_dashboard_generate_html(char *html_buffer,
                                           uint32_t buffer_size);

/**
 * @brief Send telemetry data over USB CDC
 * @param data Telemetry data to send
 * @param format Output format ("JSON", "CSV", "BINARY")
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_009: Send telemetry over USB
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_send_usb(const TelemetrySnapshot_t *data,
                                           const char *format);

/**
 * @brief Configure telemetry parameters
 * @param parameter Parameter name
 * @param value Parameter value
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_010: Configure telemetry parameters
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_configure(const char *parameter,
                                            const char *value);

/**
 * @brief Get telemetry statistics
 * @param total_requests Pointer to store total HTTP requests
 * @param successful_requests Pointer to store successful requests
 * @param error_requests Pointer to store error requests
 * @param bytes_transmitted Pointer to store total bytes transmitted
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_011: Provide telemetry statistics
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_get_statistics(uint32_t *total_requests,
                                                 uint32_t *successful_requests,
                                                 uint32_t *error_requests,
                                                 uint32_t *bytes_transmitted);

/* ============================================================================
 */
/* API Endpoints */
/* ============================================================================
 */

/**
 * @brief REST API Endpoints
 * @note Available HTTP endpoints for telemetry access
 *
 * GET /api/v1/telemetry/current - Current telemetry snapshot
 * GET /api/v1/telemetry/motors - Motor telemetry data
 * GET /api/v1/telemetry/system - System telemetry data
 * GET /api/v1/telemetry/safety - Safety telemetry data
 * GET /api/v1/telemetry/performance - Performance telemetry data
 * GET /api/v1/telemetry/history?duration=60 - Historical data (last 60
 * seconds) GET /api/v1/status - System status summary GET /api/v1/config -
 * System configuration POST /api/v1/config - Update system configuration GET
 * /api/v1/diagnostics - System diagnostics GET /dashboard - HTML dashboard
 * interface GET /dashboard/live - Live updating dashboard
 */

/* ============================================================================
 */
/* Utility Functions */
/* ============================================================================
 */

/**
 * @brief Parse HTTP request from buffer
 * @param buffer HTTP request buffer
 * @param buffer_size Size of buffer
 * @param request Parsed request structure to fill
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_012: Parse HTTP requests
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_parse_http_request(const char *buffer,
                                           uint32_t buffer_size,
                                           HttpRequest_t *request);

/**
 * @brief Format HTTP response to buffer
 * @param response Response structure
 * @param buffer Buffer to write HTTP response
 * @param buffer_size Size of buffer
 * @return Number of bytes written, or 0 on error
 * @req REQ_TELEMETRY_013: Format HTTP responses
 * @misra Rule 8.4: External function declarations
 */
uint32_t telemetry_format_http_response(const HttpResponse_t *response,
                                        char *buffer, uint32_t buffer_size);

/**
 * @brief URL decode string
 * @param input URL encoded input string
 * @param output Decoded output buffer
 * @param output_size Size of output buffer
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_014: URL decoding utility
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_url_decode(const char *input, char *output,
                                   uint32_t output_size);

/**
 * @brief JSON escape string
 * @param input Input string to escape
 * @param output Escaped output buffer
 * @param output_size Size of output buffer
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_015: JSON string escaping
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_json_escape(const char *input, char *output,
                                    uint32_t output_size);

/* ============================================================================
 */
/* Real-Time Dashboard Features */
/* ============================================================================
 */

/**
 * @brief Enable real-time dashboard updates
 * @param enable Enable (true) or disable (false) real-time updates
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_016: Real-time dashboard updates
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_enable_realtime_updates(bool enable);

/**
 * @brief Add custom dashboard widget
 * @param widget_name Widget name
 * @param widget_config Widget configuration JSON
 * @return SYSTEM_OK on success, error code on failure
 * @req REQ_TELEMETRY_017: Custom dashboard widgets
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_add_dashboard_widget(const char *widget_name,
                                             const char *widget_config);

/**
 * @brief Generate dashboard performance metrics
 * @param metrics_buffer Buffer to write metrics
 * @param buffer_size Size of buffer
 * @return Number of bytes written, or 0 on error
 * @req REQ_TELEMETRY_018: Dashboard performance metrics
 * @misra Rule 8.4: External function declarations
 */
uint32_t telemetry_generate_dashboard_metrics(char *metrics_buffer,
                                              uint32_t buffer_size);

/* ============================================================================
 */
/* Debug and Development Support */
/* ============================================================================
 */

/**
 * @brief Print telemetry dashboard status
 * @note Development and debugging function
 * @req REQ_TELEMETRY_019: Dashboard status reporting
 */
void telemetry_dashboard_print_status(void);

/**
 * @brief Test telemetry dashboard functionality
 * @return SYSTEM_OK if tests pass, error code if tests fail
 * @req REQ_TELEMETRY_020: Dashboard functionality testing
 * @misra Rule 8.4: External function declarations
 */
SystemError_t telemetry_dashboard_self_test(void);

/**
 * @brief Validate telemetry data consistency
 * @return true if data is consistent, false if corruption detected
 * @req REQ_TELEMETRY_021: Telemetry data validation
 * @sil SIL-2: Data integrity validation
 */
bool telemetry_validate_data_consistency(void);

#endif // TELEMETRY_DASHBOARD_H
