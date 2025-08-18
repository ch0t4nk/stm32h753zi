# Testing Framework Guide

**Documentation Owner:** GitHub Copilot (auto-assigned)

---

## Overview
This guide describes the testing framework and best practices for the STM32H753ZI project, including unit, integration, and simulation testing for both firmware and host builds.

## Test Types
- **Unit Tests:** Validate individual modules (drivers, algorithms) using mocks/stubs
- **Integration Tests:** Test system-level interactions, including RTOS and hardware abstraction
- **Simulation Tests:** Use host builds to simulate hardware behavior and validate logic

## Frameworks Used
- **Unity** (C): Unit testing for embedded C code
- **GoogleTest** (C++): Host-based and integration testing
- **Custom Mocks:** For hardware abstraction and peripheral simulation

## Running Tests

### Host Tests (Recommended for fast feedback)
```bash
rm -rf build_host_tests/
cmake -S host_tests -B build_host_tests
cmake --build build_host_tests
cd build_host_tests && ctest
```

### Firmware Tests (on target)
- Use `build/` and run tests via hardware debug or CI

## Best Practices
- Write tests for all new modules and features
- Use mocks for hardware dependencies
- Integrate tests into CI pipeline
- Use `scripts/validate_ssot.py` to check configuration consistency

## References
- See `docs/DEVELOPER_HANDBOOK.md` for consolidated testing info
- For simulation, see `SIMULATION_TESTING_WORKFLOWS.md`

---

For more, see the Developer Handbook and project instructions.
