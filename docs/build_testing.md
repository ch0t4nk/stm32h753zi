# Build & Testing

> **Anchor:** `build_testing.md`  
> **Location:** `docs/build_testing.md`  
> **Last Updated:** 2025‑08‑20 (mirrored in [STATUS.md](../docs/STATUS.md))

---

## Overview

The **Build & Testing** domain defines the end‑to‑end workflow that compiles the STM32 firmware, produces artifacts, and validates them through automated host‑side tests. It bridges the **firmware build** process (ARM GCC, CMake/Ninja) with the **host‑test** pipeline (native compiler, unit‑test framework, and regression suites). The domain guarantees reproducible builds, deterministic test results, and continuous integration (CI) visibility.

Key components:
| Component | Purpose | Trigger |
|-----------|---------|---------|
| `CMakeLists.txt` | Firmware & host‑test build scripts | `git commit`, CI pipeline |
| `CMakePresets.json` | Build configuration presets (Debug/Release) | CI configuration |
| `build_host_tests` | Host‑test output tree | `make test` |
| `scripts/` | Helper utilities for toolchain discovery, artifact validation, and reporting | CI / local runs |
| `src/config/ssot.yaml` | SSOT (single source of truth) manifest | Generated nightly |

---

## Detailed Description

### 1. Firmware Build Flow
1. **Source Collection** – The `src/` directory contains all HAL, application, and configuration sources.  
2. **SSOT Integration** – `src/config/workspace_config.h` is generated from the SSOT manifest (`ssot.yaml`) and injected into the CMake toolchain.  
3. **CMake Configuration** – The root `CMakeLists.txt` selects the target (`STM32H753ZI`) and compiles with the STM32CubeCLT 1.19.0 cross‑compiler.  
4. **Artifacts** – The build produces `firmware.elf`, `firmware.bin`, and `firmware.hex`.  
5. **Post‑Build Validation** – `scripts/validate_firmware.sh` verifies ELF checksums and size limits.

### 2. Host‑Test Build Flow
1. **Separate CMake List** – `host_tests/CMakeLists.txt` compiles C++/C tests using the host compiler (GCC/Clang).  
2. **Mocks & Stubs** – Generated test mocks from `scripts/generate_mocks.py` replace HAL interfaces.  
3. **Test Execution** – `ctest` runs the suite; results are captured in `build_host_tests/Testing/Temporary/LastTest.log`.  
4. **Coverage & Analysis** – `lcov`/`gcovr` report code coverage; `scripts/analyze_pll.py` inspects telemetry consistency.

### 3. CI Pipeline
- **Trigger:** Push to `main` or a feature branch.  
- **Stages:**  
  1. *Build Firmware* – `make` (Debug preset).  
  2. *Build Host Tests* – `make test`.  
  3. *Upload Artifacts* – `firmware.hex` to Artifactory.  
  4. *Publish Test Results* – `Jenkins`/`GitHub Actions` test matrix.  

The pipeline is defined in `.github/workflows/build_test.yml` (GitHub Actions) and `Jenkinsfile` (Jenkins). It automatically fetches the latest SSOT, ensures deterministic builds, and fails fast on any mismatch.

---

## Algorithm Details

### 1. SSOT Manifest Generation
```yaml
# src/config/ssot.yaml
last_updated: 2025-08-20
ssot_sources:
  firmware_headers:
    - build_config.h
    - hardware_config.h
    - motor_config.h
    - safety_config.h
    - telemetry_config.h
    - comm_config.h
    - workspace_config.h
  docs_dir: docs
  reference_dir: 00_reference
notes: |
  This manifest points tooling at the canonical SSOT header files under src/config/.
  It is used by doc-generation and validation scripts when a YAML SSOT manifest is required.
```
- The script `scripts/generate_ssot.py` scans `src/config/` for `.h` files and updates timestamps.  
- The manifest is committed nightly to ensure that downstream tooling always references the same header set.

### 2. Build Determinism Algorithm
1. **Seed the RNG** – `CMake` passes `-DSEED=12345` to each compiler to produce deterministic binaries.  
2. **Strip Unnecessary Sections** – Post‑link strip removes `.debug` sections, ensuring identical binary size.  
3. **Hash Comparison** – `scripts/compare_hashes.py` runs `sha256sum` on both `firmware.bin` and the reference from the previous build.  
4. **CI Decision** – If hashes diverge, the CI marks the build as **FAIL**; otherwise it proceeds to test.

### 3. Test Mock Generation
- `scripts/generate_mocks.py` parses the SSOT to find all HAL function prototypes.  
- For each, it creates a mock with `extern "C"` linkage and an inline counter.  
- Mocks are compiled into a static library `libmocks.a`, linked into every host test binary.

---

## Best Practices

| Practice | Rationale | Implementation |
|----------|-----------|----------------|
| **Use CMakePresets** | Consistent build flags across developers and CI | `CMakePresets.json` includes `Debug` & `Release` presets |
| **Pin Toolchain Versions** | Avoid ABI mismatches | `tools/STM32CubeCLT-1.19.0` is bundled in the repo |
| **Avoid Hard‑Coded Paths** | Cross‑platform portability | Scripts use `STM32_PROGRAMMER_CLI` env var or `Get-WorkflowToolchain.ps1` |
| **Run Tests Locally First** | Catch errors early | `scripts/run_host_tests.ps1` provides a single‑command wrapper |
| **Archive Outdated Docs** | Preserve legacy knowledge | `archive/outdated_docs/DOCS_BKUP_08-18-2025` holds last stable docs |
| **Automate SSOT Sync** | Prevent stale headers | Nightly GitHub Action updates `ssot.yaml` and triggers doc rebuild |
| **Validate Artifacts** | Detect build corruption | `scripts/validate_firmware.sh` verifies size and checksum |

---

## Reference Links

- **Status Overview** – [STATUS.md](../docs/STATUS.md)
- **SSOT Manifest** – [src/config/ssot.yaml](../src/config/ssot.yaml)
- **Archive of Outdated Docs** – [archive/outdated_docs/DOCS_BKUP_08-18-2025](../archive/outdated_docs/DOCS_BKUP_08-18-2025)
- **Domain Index** – [docs/domains.md](../docs/domains.md)
- **Build Scripts** – `scripts/`
- **CI Configuration** – [.github/workflows/build_test.yml](../.github/workflows/build_test.yml) & `Jenkinsfile`
- **Related Domains** –  
  - [Core Software](core_software.md)  
  - [Communications](communications.md)  
  - [Telemetry](telemetry.md)  
  - [Motor Control](motor_control.md)  
  - [Safety](safety.md)  
  - [Simulation](simulation.md)  
  - [Documentation SSOT](documentation_ssot.md)

--- 

*End of document.*
