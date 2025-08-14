# Dual Build System Instructions

**Documentation Owner:** GitHub Copilot (auto-assigned)

---

## Overview
This project uses a dual build system to support both ARM firmware compilation and host-based testing. This approach ensures robust development, easy debugging, and reliable CI.

## Dual Build Architecture

### 1. ARM Firmware Compilation (Primary Target)
- **Purpose:** Build production firmware for STM32H753ZI hardware
- **Toolchain:** ARM GCC (`arm-none-eabi-gcc`)
- **Config:** `CMakePresets.json` (ARM-specific)
- **Build Dir:** `build/`

### 2. Host Testing Compilation (Secondary Target)
- **Purpose:** Build drivers and logic for local/unit testing with mocks
- **Toolchain:** System GCC (`gcc`)
- **Config:** `host_tests/CMakeLists.txt`
- **Build Dir:** `build_host_tests/`

## Setup & Usage

### ARM Firmware Build (Recommended)
```bash
rm -rf build/
cmake --preset Debug
cmake --build build/Debug
```

### Host Test Build
```bash
rm -rf build_host_tests/
cmake -S host_tests -B build_host_tests
cmake --build build_host_tests
```

## Troubleshooting
- Always clean build directories before switching toolchains
- Use CMake presets for ARM builds
- See `scripts/fix_cmake.sh` for automated recovery

## Environment Validation
- Use `scripts/validate_build_env.sh` to check toolchain, CMake, Ninja, and config files

## References
- See `.github/instructions/cmake-build-system.instructions.md` for full details
- All build config is SSOT: never hardcode toolchain paths outside config files

---

For more, see the Developer Handbook and project instructions.
