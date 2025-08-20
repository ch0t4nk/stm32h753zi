# STM32H753ZI Stepper Motor Control Project

![Version](https://img.shields.io/badge/version-0.00-success) ![Build Status](https://github.com/ch0t4nk/stm32h753zi/actions/workflows/ci.yml/badge.svg) ![SSOT Validation](https://github.com/ch0t4nk/stm32h753zi/actions/workflows/validate-ssot.yml/badge.svg) ![License: MIT](https://img.shields.io/badge/license-MIT-blue)

## 🎯 Cortex-M7 Stepper Positioning System

This repository provides a production-focused firmware stack for precise stepper motor positioning on the STM32H753ZI Nucleo platform. It combines L6470 motor drivers, AS5600 encoder feedback, a dual SSOT configuration system, CI/validation tooling, and host-test/simulation harnesses.

> See `STATUS.md` for the latest health, deployment readiness, and validation summary.

## 🚀 Key features

- Dual L6470 stepper motor control (SPI daisy-chain)
- Dual AS5600 magnetic encoder closed-loop feedback
- Safety-first architecture: emergency stop, watchdogs, fault monitoring
- SSOT-driven configuration in `src/config/` with automated validation
- GitHub Actions CI for build, SAST, and SSOT validation
- Host-tests and simulation harnesses for hardware-free validation

## 🛠️ Quickstart (optimized for GitHub viewers)

1. Clone the repository:

```bash
git clone https://github.com/ch0t4nk/stm32h753zi.git
cd stm32h753zi
```

2. Set up Python environment and install requirements (Windows PowerShell example):

```powershell
python -m venv .venv
.venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

3. Configure and build (recommended):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/fix_cmake.ps1
cmake --build build
```

4. Run the SSOT validator (optional):

```powershell
.venv\Scripts\python.exe scripts/validate_ssot.py
```

Notes:

- See `.github/workflows/` for CI definitions, and `.vscode/tasks` for convenient VS Code tasks.

## 🔧 Build & Flash (examples)

```bash
# Configure + build
cmake -S . -B build && cmake --build build

# Run unit/host tests
cd build && ctest

# Flash using OpenOCD (example)
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
        -c "program build/stm32h753_ihm02a1.elf verify reset exit"
```

Or use the included VS Code task "Flash STM32H753ZI" which calls STM32_Programmer_CLI on Windows.

## 📁 Project layout (top-level)

```
src/
├── config/              # SSOT configuration headers (hardware/motor/safety)
├── drivers/             # Hardware abstraction and driver code (L6470, AS5600, etc.)
├── controllers/         # Motion control and algorithms
├── communication/       # UART/CAN/Ethernet protocol layers
├── safety/              # Safety monitoring and response
└── application/         # Main application logic and integration

.github/instructions/    # Workspace instructions for Copilot and contributors
00_reference/            # Vendor/reference docs (read-only, not built)
scripts/                 # Build, validation, and helper scripts
docs/                    # Generated and source documentation
tests/                   # Unit and integration tests + host-tests
```

## 🧭 Architecture summary

- Primary framework: X-CUBE-SPN2 (stepper control stack)
- Enhancement layer: selective MCSDK algorithms (motion profiling, safety)
- Configuration: Dual SSOT (`src/config/` and `src/config/workflow_config.h`)

## 📡 API references

- REST: `api/openapi.yaml`
- CAN: `api/can_api.yaml`

## 🛡️ License

This project is licensed under the MIT License — see `LICENSE`.

## 👥 Contributors

- GitHub Copilot

---

If you'd like, I can also add a "Contributing" section, CODEOWNERS file, or a short maintenance checklist for releases and tagging. Would you like me to add those now?
