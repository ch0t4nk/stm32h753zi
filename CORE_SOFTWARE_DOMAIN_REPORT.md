# Core Software Domain Report

**Date:** 2025-08-13
**Domain:** Core Software (controllers, common, main application logic)

---

## Incomplete Code
- Numerous TODOs in `src/application/main.c` for SSOT clock config, APB divisions, safety shutdown, and peripheral initialization.
- Several references to instruction files for L6470 and AS5600 integration (awaiting further implementation).
- Some functions in main application logic are stubs or have placeholder comments.

## Code Blocked by Hardware
- No direct hardware gating in core logic, but some initialization and integration steps (e.g., L6470/AS5600) require hardware for full validation.

## Code Awaiting New Features
- Some TODOs reference future features or enhancements (e.g., advanced safety shutdown, full SSOT integration).
- Comments reference feature tracking (FTR-XXX) for future work.

## Recommendations
- Complete all SSOT-based configuration in main application logic.
- Implement and test L6470 and AS5600 integration as hardware becomes available.
- Replace stubs with real logic as soon as hardware and dependencies are ready.
- Regularly review TODOs and update with feature tracker references.

---

**Next Steps:**
- Prioritize completion of SSOT integration and safety shutdown logic.
- Coordinate with hardware team for L6470/AS5600 bring-up.
- Use feature tracker to manage pending enhancements.
