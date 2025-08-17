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

#include <stdbool.h>
#include <stdint.h>

#include "common/error_codes.h"
#include "config/freertos_config_ssot.h"
#include "rtos/dynamic_task_tuning.h"
#include "rtos/power_management.h"

/* ============================================================================
 */
/* Configuration Constants (SSOT compliant) */
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

typedef struct {
    uint32_t can_messages_tx;
    uint32_t can_messages_rx;
    uint32_t can_errors;
    uint32_t uart_bytes_tx;
    uint32_t uart_bytes_rx;
    uint32_t uart_errors;
    uint32_t usb_bytes_tx;
    uint32_t usb_bytes_rx;
    uint32_t usb_errors;
    bool can_connected;
    bool uart_connected;
    bool usb_connected;
    uint32_t last_update_ms;
} CommTelemetryData_t;

typedef struct {
    bool emergency_stop_active;
    bool watchdog_active;
    uint32_t safety_violations;
    uint32_t warning_count;
    float max_motor_current_a;
    float max_cpu_temperature_c;
    uint32_t fault_recovery_count;
    uint32_t system_resets;
    uint32_t last_safety_event_ms;
    const char *last_safety_message;
    uint32_t last_update_ms;
} SafetyTelemetryData_t;

typedef struct {
    uint32_t motor_control_frequency;
    uint32_t motor_control_jitter_us;
    uint32_t safety_check_frequency;
    uint32_t communication_frequency;
    uint32_t max_interrupt_latency_us;
    uint32_t memory_fragmentation_pct;
    uint32_t disk_usage_percent;
    uint32_t network_latency_ms;
    uint32_t last_update_ms;
} PerformanceTelemetryData_t;

typedef struct {
    MotorTelemetryData_t motors[2];
    SystemTelemetryData_t system;
    CommTelemetryData_t communication;
    SafetyTelemetryData_t safety;
    PerformanceTelemetryData_t performance;
    uint32_t snapshot_timestamp;
    uint32_t snapshot_id;
} TelemetrySnapshot_t;

typedef struct {
    char method[16];
    char path[256];
    char query_string[512];
    char *body;
    uint32_t body_length;
    uint32_t connection_id;
} HttpRequest_t;

typedef struct {
    uint16_t status_code;
    char content_type[64];
    char *body;
    uint32_t body_length;
    bool connection_close;
} HttpResponse_t;

typedef struct {
    bool enabled;
    uint32_t update_interval_ms;
    uint32_t history_depth;
    bool enable_motor_telemetry;
    bool enable_system_telemetry;
    bool enable_safety_telemetry;
    bool enable_performance_telemetry;
    bool enable_web_interface;
    bool enable_json_api;
    const char *dashboard_title;
} DashboardConfiguration_t;

// Public API Functions
SystemError_t telemetry_dashboard_init(const DashboardConfiguration_t *config);
SystemError_t telemetry_dashboard_start(void);
SystemError_t telemetry_dashboard_stop(void);
SystemError_t telemetry_dashboard_update(void);
SystemError_t telemetry_dashboard_process_request(const HttpRequest_t *request,
                                                  HttpResponse_t *response);
SystemError_t telemetry_dashboard_get_snapshot(TelemetrySnapshot_t *snapshot);
uint32_t telemetry_dashboard_generate_json(char *json_buffer,
                                           uint32_t buffer_size,
                                           bool include_history);
uint32_t telemetry_dashboard_generate_html(char *html_buffer,
                                           uint32_t buffer_size);
SystemError_t telemetry_dashboard_send_usb(const TelemetrySnapshot_t *data,
                                           const char *format);
SystemError_t telemetry_dashboard_configure(const char *parameter,
                                            const char *value);
SystemError_t telemetry_dashboard_get_statistics(uint32_t *total_requests,
                                                 uint32_t *successful_requests,
                                                 uint32_t *error_requests,
                                                 uint32_t *bytes_transmitted);
SystemError_t telemetry_parse_http_request(const char *buffer,
                                           uint32_t buffer_size,
                                           HttpRequest_t *request);
uint32_t telemetry_format_http_response(const HttpResponse_t *response,
                                        char *buffer, uint32_t buffer_size);
SystemError_t telemetry_url_decode(const char *input, char *output,
                                   uint32_t output_size);
SystemError_t telemetry_json_escape(const char *input, char *output,
                                    uint32_t output_size);
SystemError_t telemetry_enable_realtime_updates(bool enable);
SystemError_t telemetry_add_dashboard_widget(const char *widget_name,
                                             const char *widget_config);
uint32_t telemetry_generate_dashboard_metrics(char *metrics_buffer,
                                              uint32_t buffer_size);
void telemetry_dashboard_print_status(void);
SystemError_t telemetry_dashboard_self_test(void);
bool telemetry_validate_data_consistency(void);

#endif // TELEMETRY_DASHBOARD_H
