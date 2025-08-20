# Communications

> **Domain Doc** – communications.md  
> **Last Updated:** 2025‑08‑20  
> **Status:** ✔️ In‑progress (firmware builds complete)  
> **Related Anchors:**  
> - [SSOT](src/config/ssot.yaml)  
> - [STATUS](docs/STATUS.md)  
> - [ARCHIVE](archive/outdated_docs/DOCS_BKUP_08-18-2025)  
> - [DOMAINS](docs/domains.md) (core_software.md, telemetry.md, motor_control.md, safety.md, build_testing.md, simulation.md, documentation_ssot.md)

---

## Overview

The **Communications** domain implements the stack that allows the STM32H753ZI firmware to exchange data with peripherals, host systems, and external modules.  It supports multiple physical layers (UART, I²C, SPI, CAN, and Ethernet) and provides a uniform API for higher‑level modules such as telemetry, motor control, and safety.  All communication protocols are configured through the Single Source Of Truth (SSOT) headers in `src/config/`, ensuring that register settings, baud rates, and buffer sizes remain consistent across the build.

---

## Detailed Description

| Layer | Physical Interface | Driver | Use‑Case | Key Features |
|-------|--------------------|--------|----------|--------------|
| **UART** | USART3 / USART2 | HAL + DMA | Host console, telemetry logging | Asynchronous, configurable baud, FIFO buffer, half‑duplex support |
| **I²C** | I2C1 | HAL | Sensor data, EEPROM, I/O expanders | SMBus compatibility, 7/10‑bit addressing, interrupt‑driven |
| **SPI** | SPI1, SPI2 | HAL | Motor driver, display, external RF | Full‑duplex, configurable bit‑rate, DMA support |
| **CAN** | CAN1 | HAL | Vehicle network, fault‑diagnostics | 500 kbit/s, 1 Mbit/s, FIFO mailboxes, error‑state monitoring |
| **Ethernet** | MII via RMII (STM32H7‑ETH) | HAL + LwIP | OTA updates, network telemetry | DHCP, TCP/UDP sockets, LwIP lightweight IP stack |

All drivers expose a common `Comm_Interface` struct containing function pointers for initialization, send/receive, and status queries.  This abstraction enables plug‑in replacement (e.g., testing with virtual back‑ends) and simplifies the implementation of higher‑level protocols such as MAVLink or custom JSON over TCP.

---

## Algorithm Details

### 1. Message Framing & Buffer Management

```c
typedef struct {
    uint8_t  header;   // 0xAA for sync
    uint8_t  type;     // command / telemetry
    uint16_t length;   // payload length
    uint8_t  payload[256];
    uint8_t  checksum; // CRC‑8
} Comm_Packet;

void Comm_Send(const Comm_Packet *pkt) {
    uint8_t buffer[264];
    memcpy(buffer, pkt, pkt->length + 5);   // header + type + len + payload + checksum
    HAL_UART_Transmit_DMA(&huart3, buffer, pkt->length + 5);
}
```

* **Cyclic redundancy check (CRC‑8)** is computed over `type | length | payload`.  It is verified in the receive interrupt; on mismatch the packet is discarded and a NACK is sent.

* **DMA + double‑buffering** ensures zero‑copy transmission.  While DMA transfers data out, the next packet is prepared in a secondary buffer.

### 2. Flow Control

* **Hardware**: UART CTS/RTS pins used when enabled in `Comm_Config`.  
* **Software**: XON/XOFF flow control for UART streams; back‑pressure in SPI/CAN FIFO thresholds triggers `Comm_Poll()` to halt sending until space is available.

### 3. Priority Handling

Higher‑priority frames (e.g., safety commands) are queued in a dedicated ring buffer.  A watchdog checks that a safety packet is sent within a configurable timeout (`COMM_SAFETY_TIMEOUT_MS`).

---

## Best Practices

| Practice | Rationale | Implementation |
|----------|-----------|----------------|
| **Use HAL + DMA** | Minimizes CPU load and avoids blocking calls | Configure `HAL_UART_Init()` with `UART_DMA_TX`, `HAL_SPI_Transmit_DMA()` |
| **Avoid blocking in ISRs** | Prevents missed interrupts and stack overflow | Only set flags or enqueue packets in the interrupt handler |
| **Keep packet size <= DMA buffer** | Prevents fragmentation and DMA errors | Define `MAX_PKT_LEN` in `comm_config.h` and assert at compile time |
| **Validate SSOT settings** | Ensures consistency across modules | Run `scripts/run_python_configurable.ps1` before each build |
| **Implement checksum verification** | Detects corruption early | Use `crc8()` function from `comm_utils.c` |
| **Leverage LwIP for Ethernet** | Provides robust TCP/IP stack without reinventing the wheel | Enable `LWIP_TCPIP_CORE_LOCKING` for thread safety |
| **Use FIFO mailboxes for CAN** | Guarantees message ordering and loss detection | Configure `CAN_FilterConfig` for 64‑bit IDs |
| **Document all protocol changes** | Maintains traceability | Update `comm_protocol.md` and reference in `CHANGELOG.md` |

---

## Reference Links

- **SSOT Manifest** – [src/config/ssot.yaml](src/config/ssot.yaml)  
- **Project Status** – [docs/STATUS.md](docs/STATUS.md)  
- **Archived Docs** – [archive/outdated_docs/DOCS_BKUP_08-18-2025](archive/outdated_docs/DOCS_BKUP_08-18-2025)  
- **Domain List** – [docs/domains.md](docs/domains.md) (core_software.md, telemetry.md, motor_control.md, safety.md, build_testing.md, simulation.md, documentation_ssot.md)  
- **HAL Reference** – STM32Cube H7 HAL Documentation  
- **LwIP API Guide** – https://savannah.nongnu.org/projects/lwip/  
- **CAN Stack Overview** – STM32Cube CAN Driver Documentation  
- **UART Flow Control Specification** – IEEE 802.3 / RS‑232 standards  

---
