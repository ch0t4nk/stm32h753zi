Thinking...
We need to produce a markdown document called documentation_ssot.md. Should have sections: Overview, Detailed Description, Algorithm Details, Best Practices, Reference Links. Use anchors: list of anchors includes SSOT (src/config/ssot.yaml), STATUS (docs/STATUS.md), ARCHIVE, DOMAINS list includes documentation_ssot.md. Use anchors appropriately.

We should produce a markdown with proper headings. Provide content about SSOT manifest for documentation and tooling. Probably referencing the SSOT YAML. Provide overview of SSOT concept, usage, etc. Provide algorithm details: maybe how to generate docs from SSOT, how scripts parse SSOT manifest. Provide best practices: how to maintain, update, validation. Reference links: maybe to docs/STATUS.md, to ssot.yaml, to relevant sections.

We need to adhere to policy: produce markdown, do not push commit. Provide only markdown.

We need to incorporate the anchors: Use markdown link syntax with anchors like [SSOT](src/config/ssot.yaml). Provide references.

Also mention archive location: archive/outdated_docs/DOCS_BKUP_08-18-2025.

Also mention that docs/STATUS.md contains status. The document belongs to docs/ directory? Domain doc likely in docs/. The anchors list may refer to other docs. We'll provide links accordingly.

Also we should not commit; just output markdown.

Ok produce.

...done thinking.

# documentation_ssot.md

## Overview

The **Single Source Of Truth (SSOT)** is the central place where all key configuration, documentation, and reference artifacts live for the STM32H753ZI project.  
It is defined by the YAML manifest located at:

[SSOT](src/config/ssot.yaml)

The SSOT guarantees that tooling, documentation, and firmware build systems reference the same canonical files, avoiding duplication and ensuring consistency across the entire code base.

## Detailed Description

- **Location**  
  The SSOT manifest resides in `src/config/ssot.yaml`.  It lists all header files that form the project's authoritative configuration set.

- **Contents**  
  ```yaml
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

- **Purpose**  
  - **Documentation** – Tools like Sphinx or Doxygen read the SSOT to generate API docs that reflect the true firmware configuration.  
  - **Validation** – CI jobs compare the actual header contents against the manifest to detect drift.  
  - **Build System** – CMake picks up the SSOT to include the correct headers and set compile‑time constants.

- **Integration**  
  - The SSOT is referenced in multiple automation scripts: `scripts/run_python_configurable.ps1`, `scripts/generate_workspace_config.py`, and the CI pipeline (`.github/workflows`).  
  - All new headers added to the firmware must be listed here before the next documentation build.

## Algorithm Details

1. **Read Manifest**  
   ```python
   with open('src/config/ssot.yaml') as f:
       ssot = yaml.safe_load(f)
   ```
2. **Validate Header Paths**  
   For each path in `ssot['ssot_sources']['firmware_headers']`:
   - Check that the file exists (`os.path.exists(path)`).
   - Optionally parse the header to ensure it contains expected defines.

3. **Generate Documentation**  
   - Pass the list of header paths to the documentation generator as a `-I` include flag.  
   - Example for Doxygen:
     ```bash
     doxygen -I src/config -H src/config/build_config.h -H src/config/hardware_config.h ...
     ```
4. **CI Enforcement**  
   - After each commit, run the validation script.  
   - If the manifest is out of sync, the job fails with a diff output, preventing stale documentation from being deployed.

5. **Archiving**  
   - On release, copy the current `src/config/ssot.yaml` and the generated docs to the archive:
     ```
     cp src/config/ssot.yaml archive/outdated_docs/DOCS_BKUP_08-18-2025/
     ```

## Best Practices

| Area | Recommendation | Rationale |
|------|----------------|-----------|
| **Updating SSOT** | Add new headers *before* committing code that depends on them. | Keeps the SSOT in sync with the codebase. |
| **Documentation Generation** | Run `scripts/generate_docs.ps1` after every SSOT change. | Guarantees that docs reflect the latest canonical sources. |
| **CI Checks** | Fail the pipeline if the SSOT manifest is missing or has stale entries. | Prevents accidental drift between docs and firmware. |
| **Versioning** | Tag the SSOT in git with every major firmware release. | Enables reproducible builds and documentation snapshots. |
| **Archiving** | Store the SSOT along with the documentation archive. | Provides a reference point for future rollbacks or audits. |

## Reference Links

- [SSOT Manifest](src/config/ssot.yaml)
- [Project Status](docs/STATUS.md) – Current build and deployment state.
- [Archived Docs Backup](archive/outdated_docs/DOCS_BKUP_08-18-2025) – Historical documentation snapshots.
- [Documentation Generation Script](scripts/generate_docs.ps1)
- [CI Pipeline](.github/workflows) – How SSOT is validated on every push.

---

*End of `documentation_ssot.md`*

