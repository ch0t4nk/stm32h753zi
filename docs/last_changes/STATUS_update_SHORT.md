STATUS.md updated: clarified that firmware ARM build succeeded and that host-tests are currently blocked by test mock syntax errors in `tests/mocks/mock_hal_abstraction.c`.

Next steps recorded: fix mock file (missing brace, nested functions), add math include for M_PI, rebuild host tests in `build_host_tests`, run `ctest -V`, then update STATUS.md with test results and proceed with hardware flashing.

Suggested commit message:

feat(status): update STATUS.md with current build/test state; note host-test blockers

Files changed:

- STATUS.md
- docs/last_changes/STATUS_update_SHORT.md
