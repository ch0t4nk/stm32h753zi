# SSOT Exclusions for Telemetry and Hardware Constraints

This document lists hardware-driven constraints identified from manufacturer documentation that prevent certain items from being abstracted via the Single Source of Truth (SSOT) configuration system. Each exclusion is cited with the relevant documentation and a brief rationale.

---

## 1. SPI Daisy-Chain Configuration & Physical Pin Assignments

- **Constraint**: The SPI daisy-chain wiring (nCS, SCK, SDI, SDO) and their physical pin assignments are determined by the X-NUCLEO-IHM02A1 board layout and solder bridges (e.g., SB23, SB32, SB26, SB8, SB12).
- **Documentation**: UM1964_X-NUCLEO-IHM02A1.txt, Table 1 (ST morpho connector table)
- **Reason**: These are hardware-wired and can only be changed by physically modifying the board, not by firmware or SSOT headers.

## 2. L6470 Register Addresses & Reset Values

- **Constraint**: L6470 register addresses (e.g., STATUS, ADC_OUT, ABS_POS) and their reset values are fixed by the L6470 chip design.
- **Documentation**: UM1963_X-CUBE-SPN2.txt, Table 3 (L6470 registers)
- **Reason**: These are defined by the silicon and cannot be changed or abstracted in SSOT.

## 3. Maximum Ratings (Voltage, Current)

- **Constraint**: Maximum supply voltage (8V–45V), phase current (up to 3A r.s.m.), and other electrical limits are set by the L6470 and board hardware.
- **Documentation**: UM1964_X-NUCLEO-IHM02A1.txt, Section 1 (Getting started)
- **Reason**: These are physical hardware limits and must be respected in code, but cannot be made configurable in SSOT.

## 4. Board-Specific Quirks (e.g., SB15 Removal)

- **Constraint**: Some quirks, such as the requirement to remove SB15 on NUCLEO-F401RE for proper SPI operation, are not abstractable in SSOT.
- **Documentation**: UM1963_X-CUBE-SPN2.txt, Section 3.3.1 (Hardware setup)
- **Reason**: These are physical board modifications, not software-configurable parameters.

## 5. Hardware-Determined Register Values (ADC_OUT, STATUS)

- **Constraint**: The reset values of certain registers (e.g., ADC_OUT, STATUS) are determined by hardware startup conditions, not by firmware.
- **Documentation**: UM1963_X-CUBE-SPN2.txt, Table 3 (L6470 registers)
- **Reason**: These values are set by the chip at power-on and cannot be overridden by SSOT.

---

**Summary**: The above items are excluded from SSOT abstraction due to hardware constraints. All other configurable parameters should be referenced via SSOT headers wherever possible.

---

_Generated from review of manufacturer documentation in 00_reference/ for FTR-020 telemetry system completion._
