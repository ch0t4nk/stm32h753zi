Thinking...
We need to produce a markdown document for domain doc: build_testing.md, with sections: Overview, Detailed Description, Algorithm Details, Best Practices, Reference Links. Must follow policy: produce only markdown, no commit or push. Use anchors maybe? Provide content. Use context provided. Build testing refers to build system and testing. We have status, build system details, host-test build issues. We should reference SSOT, status, architecture. Provide algorithm details on how build and test are executed, maybe CMake, Ninja, cross-compilation, host tests. Best practices for building, testing. Reference links to docs like SSOT YAML, STATUS.md, etc. Provide anchor list maybe? No need to include anchor code. Just doc content. Provide thorough but concise. Ensure markdown formatting. Let's draft.

...done thinking.

# build_testing.md

## Overview

The **Build & Testing** domain describes the end‑to‑end process that turns the STM32H753ZI firmware source into a flashable binary and verifies its correctness through automated host‑side tests.  
Key points:

- **Cross‑compile** the target firmware with the ARM GCC toolchain (`STM32CubeCLT 1.19.0`) using **CMake + Ninja**.
- **Host‑tests** run on a native compiler (MSVC/Clang/GCC) and validate logic that is platform‑agnostic (e.g., telemetry, safety state machines).
- The build system is fully **SSOT‑aware** – all configuration headers are pulled from `src/config` and referenced by the `ssot.yaml` manifest.
- Continuous integration pipelines use PowerShell wrappers (`Get-WorkflowToolchain.ps1`, `scripts/run_python_configurable.ps1`) to discover toolchain paths and generate configuration overrides.

---

## Detailed Description

### 1. Build Architecture

| Layer | Tool | Purpose |
|-------|------|---------|
| **Source** | C/C++ files + HAL + FreeRTOS | Application logic |
| **Configuration** | `src/config/*.h`, `src/config/workspace_config.h` | Runtime & build parameters (clock, peripheral, safety) |
| **SSOT Integration** | `ssot.yaml` | Declares canonical header locations for tooling |
| **CMake** | `CMakeLists.txt` (root) | Generates build files for target and host‑tests |
| **Ninja** | `build/` | Fast incremental build engine |
| **ARM GCC** | `arm-none-eabi-gcc` | Cross‑compiler for firmware |
| **Host Compiler** | MSVC/Clang/GCC | Compiles host‑test binaries |
| **Linker** | `ld` | Produces `firmware.elf`, `firmware.hex` |
| **Flasher** | `STM32_Programmer_CLI.exe` | Program device over SWD/JTAG |

### 2. Build Flow

1. **Configure**  
   ```bash
   cmake --preset Debug
   ```  
   *CMake* reads `CMakePresets.json` which references `src/config/workspace_config.h`.  
   *`Get-WorkflowToolchain.ps1`* resolves `STM32_PROGRAMMER_CLI` and sets environment variables.

2. **Build Target**  
   ```bash
   cmake --build . --config Debug
   ```  
   Generates `firmware.elf`, `firmware.bin`, `firmware.hex` in `build/firmware/`.

3. **Generate Test Mocks**  
   Scripts under `scripts/` produce `test_mocks.generated.h` into `src/config/workspace_config.generated.h` which is conditionally included.

4. **Build Host Tests**  
   ```bash
   cmake --preset HostDebug
   cmake --build build_host_tests
   ```  
   Host tests are compiled with the native compiler, linked against the same code base but with `-DHOST_TEST` macro to stub out hardware access.

5. **Run Tests**  
   ```bash
   ./build_host_tests/host_tests
   ```  
   Test runner reports pass/fail, generates JUnit XML for CI dashboards.

6. **Flashing (Optional)**  
   ```powershell
   & $Env:STM32_PROGRAMMER_CLI -c PORT=COM3 -e -w firmware.hex
   ```  

---

## Algorithm Details

The build system is a two‑stage pipeline:

1. **Firmware Stage**  
   *Input*: `src/` + `src/config/`  
   *Output*: `firmware.elf`, `firmware.hex`  
   *Algorithm*:
   - **CMake Generator**: reads `CMakeLists.txt`, expands source lists, applies `-DCMAKE_TOOLCHAIN_FILE` to point to `toolchain-arm.cmake`.
   - **Ninja Build**: executes compile commands in parallel; tracks dependencies via timestamps.
   - **Linker**: `-Tstm32h753zi.ld` with `-Wl,-Map=firmware.map` to generate a map file for diagnostics.

2. **Host‑Test Stage**  
   *Input*: Same source, plus `-DHOST_TEST` and `test_mocks.generated.h`.  
   *Output*: Binary tests, JUnit XML.  
   *Algorithm*:
   - **CMake Host Preset** sets `CMAKE_CXX_STANDARD` to 17, uses `-DHAL_DISABLE` to omit HAL includes.
   - **Test Compilation**: each test file is compiled as a unit; `#include <gtest/gtest.h>`.
   - **Execution**: the test binary runs all `TEST()` cases, collecting results into `results.xml`.

---

## Best Practices

| Topic | Recommendation |
|-------|----------------|
| **SSOT Compliance** | Always include `src/config/workspace_config.h`.  Do not hard‑code any values in CMake or source files. |
| **Incremental Builds** | Use `cmake --preset Debug` + `cmake --build .` for fast iteration.  Clean only when necessary (`cmake --build . --target clean`). |
| **Host‑Test Isolation** | Guard hardware‑specific code with `#ifdef HOST_TEST`.  Keep mocks in `test_mocks.generated.h` to avoid accidental dependency on hardware. |
| **Continuous Integration** | CI jobs should run: (a) firmware build, (b) host tests, (c) flasher test on a hardware emulator.  Use the PowerShell wrappers for cross‑platform consistency. |
| **Logging & Reporting** | Emit `firmware.map` and `results.xml` to `artifacts/`.  Use `-Wl,-Map=` for linker map.  Include `--output-on-failure` in GTest for debugging. |
| **Error Handling** | Fail fast: if `cmake` or `make` fails, abort the pipeline.  Capture logs (`build_log.txt`). |
| **Documentation** | Update `STATUS.md` and `docs/STATUS.md` after each successful build.  Keep `ssot.yaml` current. |
| **Hardware Validation** | After host tests pass, schedule a *flash & run* test on a test rig to catch run‑time issues. |

---

## Reference Links

- **SSOT Manifest** – `src/config/ssot.yaml` – defines canonical header locations for tooling.  
- **Project Status** – `docs/STATUS.md` – shows last build results, test metrics, and deployment status.  
- **CMake Presets** – `CMakePresets.json` – configures Debug/Release and Host presets.  
- **Toolchain Resolution** – `scripts/Get-WorkflowToolchain.ps1` – selects the correct STM32 programmer CLI.  
- **Build Scripts** – `scripts/run_python_configurable.ps1` – orchestrates Python‑based config generation.  
- **Test Framework** – [Google Test](https://github.com/google/googletest) – used for host‑side unit tests.  
- **Documentation** – `docs/` – contains domain docs, build guides, and SSOT usage.  

---

*All paths and commands assume a working PowerShell session on Windows or a Bash shell on Linux/Mac with the ARM GCC toolchain installed.*

