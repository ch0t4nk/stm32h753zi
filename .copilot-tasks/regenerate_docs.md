# regenerate_docs prompt (generated from prompts/github_system.md)

Source: `prompts/github_system.md` processed on 2025-08-18.

## Intent

Prepare repository artifacts to offload domain-driven documentation regeneration via a GitHub Action. This file captures the regeneration instructions and anchors.

## Anchors

- SSOT config: `src/config/ssot.yaml`
- Workspace status: `STATUS.md`
- Archived docs: `archive/outdated_docs/DOCS_BKUP_08-18-2025`
- Output target: `docs/`

## Regeneration plan (extracted)

1. Ingest `STATUS.md` as primary context anchor.
2. Reference archived docs for historical continuity.
3. Ensure `.github/instructions/` are respected and SSOT is followed.
4. Regenerate domain docs (overwrite):
   - `core_software.md`
   - `communications.md`
   - `telemetry.md`
   - `motor_control.md`
   - `safety.md`
   - `build_testing.md`
   - `documentation_ssot.md`
5. Optimize for human+AI consumption: semantic anchors, inline summaries, `last_updated:` header.
6. Validate SSOT via `scripts/validate_ssot.py`.
7. Workflow: `.github/workflows/regenerate-docs.yml` (trigger: workflow_dispatch).
8. Commit with message: `docs: regenerate domain docs via Copilot++ offload`.

---

NOTE: This is a local prepared task file. To fully offload to GitHub Actions, push these changes and trigger the workflow via the Actions UI or `gh workflow run`.
