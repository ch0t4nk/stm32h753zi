#ifdef UNITY_TESTING

#include "config/error_codes.h"
#include "optimization_telemetry.h"
#include <stdio.h> // for snprintf used in host stub
#include <string.h>

// Minimal host stubs to satisfy test linker dependencies. These are
// intentionally conservative and return SYSTEM_OK or simple fallbacks so
// unit tests can link and run without the full telemetry/export logic.

SystemError_t optimization_telemetry_calculate_thermal_performance(
    const OptimizationTelemetryPacket_t *packet, float *thermal_score) {
    if (packet == NULL || thermal_score == NULL)
        return ERROR_NULL_POINTER;
    // Simple heuristic: lower current -> better thermal score
    float current = packet->motor_current_a;
    if (current <= 0.0f)
        *thermal_score = 1.0f;
    else if (current >= 3.0f)
        *thermal_score = 0.0f;
    else
        *thermal_score = 1.0f - (current / 3.0f);
    return SYSTEM_OK;
}

SystemError_t
optimization_telemetry_export_json(const CharacterizationDataSet_t *dataset,
                                   char *json_buffer, size_t buffer_size,
                                   size_t *output_size) {
    if (dataset == NULL || json_buffer == NULL || output_size == NULL)
        return ERROR_NULL_POINTER;
    // Produce a tiny JSON summary suitable for unit tests
    /* Produce a minimal JSON including sample_count and a samples array so
     * unit tests that look for keys and size see reasonable output. */
    int written =
        snprintf(json_buffer, buffer_size,
                 "{\"motor_id\":%u,\"sample_count\":%u,\"samples\":[",
                 dataset->motor_id, (unsigned)dataset->sample_count);
    for (uint32_t i = 0; i < dataset->sample_count; i++) {
        if ((size_t)written < buffer_size - 50) {
            int n = snprintf(json_buffer + written, buffer_size - written,
                             "{\"timestamp_us\":%u,\"position\":%.1f}",
                             dataset->samples[i].timestamp_us,
                             dataset->samples[i].position_degrees);
            if (n > 0)
                written += n;
            if (i + 1 < dataset->sample_count) {
                if ((size_t)written < buffer_size - 2) {
                    json_buffer[written++] = ',';
                }
            }
        }
    }
    if ((size_t)written < buffer_size - 2)
        json_buffer[written++] = ']';
    if ((size_t)written < buffer_size - 1)
        json_buffer[written++] = '}';
    if (written < 0)
        return ERROR_OPERATION_FAILED; // use canonical defined error code
    *output_size = (size_t)written;
    return SYSTEM_OK;
}

// Compatibility stub for estop_trigger used by telemetry.
void estop_trigger(int source) {
    // For host tests, just record the event in a global mock state if needed.
    (void)source;
}

#endif // UNITY_TESTING
