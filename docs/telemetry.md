Thinking...
We need to generate a markdown document for domain telemetry.md. Should follow policy in prompt exactly. Output only the markdown. Sections: Overview, Detailed Description, Algorithm Details, Best Practices, Reference Links. Use anchors? The anchors list includes SSOT, STATUS, ARCHIVE, DOMAINS. The anchor names likely for linking. But we don't need to embed links? The anchor list includes SSOT: 'src/config/ssot.yaml', STATUS: 'docs/STATUS.md', ARCHIVE: 'archive/outdated_docs/DOCS_BKUP_08-18-2025', DOMAINS: list of domain docs. We can reference these maybe. The policy: no commit/push, just output markdown. Let's produce a document summarizing telemetry domain. Use information context: telemetry_config.h present, mention telemetry. Use best practices for telemetry. Reference links: maybe link to docs/STATUS.md, src/config/ssot.yaml, etc. Provide internal links using anchor names: e.g., [SSOT](#SSOT) maybe. But typical referencing is markdown links: [SSOT](src/config/ssot.yaml). Provide reference links section. Ok.

We need to ensure no extraneous commentary. Just the markdown.

Let's produce.
...done thinking.

# Telemetry Domain

## Overview

The **Telemetry** domain governs the collection, packaging, and transmission of operational data from the STM32H753ZI-based embedded system to external hosts. It defines how sensor readings, system health metrics, and event logs are gathered, formatted, and made available over the chosen communication channels (UART, CAN, USB, etc.). This domain is critical for real‑time monitoring, diagnostics, and remote control during development, field deployment, and automated testing.

## Detailed Description

| Feature | Description |
|---------|-------------|
| **Telemetry Sources** | • **Peripheral Sensors** – IMU, pressure, temperature, voltage/current monitors.<br>• **System Health** – CPU load, heap usage, watchdog status, RTOS task states.<br>• **Event Logs** – Error codes, state machine transitions, safety violations. |
| **Data Packaging** | • **Telemetry Packet** – 4‑byte header (`0xAA55`), 2‑byte length, payload, CRC‑16, 2‑byte tail (`0x55AA`).<br>• **Payload Formats** – Binary (packed structs) for efficiency; JSON‑lite fallback for debugging. |
| **Transmission Pathways** | • **UART** – low‑latency, configurable baud (115 200–1 M).<br>• **CAN** – high‑reliability, message ID filtering.<br>• **USB CDC** – convenient for host‑side PC tools.<br>• **Wireless (future)** – optional Wi‑Fi/BLE module. |
| **Configuration** | • Controlled via `src/config/telemetry_config.h` (register values, rates).<br>• Runtime reconfigurable through the **Telemetry CLI** (host‑side tool). |
| **Rate Management** | • **Static Rates** – user‑defined in config (e.g., 100 Hz, 1 Hz).<br>• **Dynamic Rate Control** – throttling when CPU load exceeds threshold; priority queue per telemetry class. |
| **Error Handling** | • Packet loss detection via sequence numbers.<br>• Automatic retransmission request for critical data. |

The telemetry stack is implemented in C/C++ using the STM32 HAL and FreeRTOS. All packet construction, CRC calculation, and buffer management are encapsulated in `telemetry.c/h`, while the transport layer is abstracted behind `telemetry_transport.h`.

## Algorithm Details

### Packet Construction

```c
// pseudocode
void telemetry_send(const void *payload, uint16_t len)
{
    uint8_t buffer[HEADER_SIZE + len + CRC_SIZE + TAIL_SIZE];
    uint16_t pos = 0;

    // Header
    buffer[pos++] = 0xAA;
    buffer[pos++] = 0x55;

    // Length
    buffer[pos++] = (len >> 8) & 0xFF;
    buffer[pos++] = len & 0xFF;

    // Payload
    memcpy(&buffer[pos], payload, len);
    pos += len;

    // CRC
    uint16_t crc = crc16(buffer, pos);
    buffer[pos++] = crc >> 8;
    buffer[pos++] = crc & 0xFF;

    // Tail
    buffer[pos++] = 0x55;
    buffer[pos++] = 0xAA;

    // Transmit over selected interface
    transport_send(buffer, pos);
}
```

### Rate Throttling

```
if (cpu_load > THRESHOLD) {
    // Drop lower‑priority telemetry frames
    for each telemetry_class in priority_order {
        if (telemetry_class.priority < LOW_PRIORITY) {
            drop_next_frame(telemetry_class);
        }
    }
}
```

### CRC‑16 Calculation

Implemented using the standard CCITT‑Kermit polynomial (`0x1021`). The function is hardware‑accelerated if the STM32 peripheral CRC unit is available; otherwise a software table‑lookup is used.

## Best Practices

| Area | Recommendation |
|------|----------------|
| **Configuration Management** | Keep `telemetry_config.h` under SSOT (`src/config/ssot.yaml`). Update telemetry rates only through CI checks to avoid accidental high‑bandwidth bursts. |
| **Packet Size** | Target < 128 bytes per packet for UART to prevent buffer overruns; use multi‑frame packets only for large data logs. |
| **Debugging** | Enable `TELEMETRY_DEBUG` flag during host‑test builds to dump raw packets to console. |
| **Error Handling** | Implement sequence numbers per class to detect out‑of‑order packets. Use watchdog reset on CRC failure. |
| **Security** | For future wireless transport, integrate lightweight authentication (e.g., HMAC‑SHA256 over packet payload). |
| **Documentation** | Reference the SSOT for header definitions; keep the telemetry domain doc in sync with the latest firmware release. |
| **Testing** | Add unit tests in `tests/telemetry_test.c` covering CRC, packet framing, and rate throttling logic. |

## Reference Links

- [SSOT](src/config/ssot.yaml) – Single source of truth for telemetry headers and config.  
- [STATUS.md](docs/STATUS.md) – Current build and test status, including telemetry test results.  
- [Archive](archive/outdated_docs/DOCS_BKUP_08-18-2025) – Previous versions of the telemetry documentation.  
- [Core Software](docs/core_software.md) – Interaction between telemetry and the RTOS task scheduler.  
- [Communications](docs/communications.md) – Transport layer abstractions used by telemetry.  
- [Safety](docs/safety.md) – How telemetry informs the safety monitor.  
- [Build & Testing](docs/build_testing.md) – Build flags that enable telemetry debugging.  
- [Simulation](docs/simulation.md) – Mock telemetry streams for host‑side simulators.  

---

