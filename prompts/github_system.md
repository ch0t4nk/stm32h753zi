# 🔄 Offload Regeneration of Technical Docs via GitHub Action

## 🧠 Task Summary

Offload the regeneration of domain-driven technical documentation to GitHub Actions. Use Copilot++ to package the task, push it to the repo, and trigger the workflow remotely.

## 📂 Context Anchors

- SSOT config: `src/config/ssot.yaml`
- Workspace summary: `docs/STATUS.md`
- Archived docs: `# 🔄 Offload Regeneration of Technical Docs via GitHub Action

## 🧠 Task Summary

Offload the regeneration of domain-driven technical documentation to GitHub Actions. Use Copilot++ to package the task, push it to the repo, and trigger the workflow remotely.

## 📂 Context Anchors

- SSOT config: `src/config/ssot.yaml`
- Workspace summary: `STATUS.md`
- Archived docs: `archive/outdated_docs/DOCS_BKUP_08-18-2025`
- Output target: `docs/`

## 🧾 Regeneration Instructions

1. Ingest `STATUS.md` as primary context anchor.
2. Reference archived docs in `archive/outdated_docs/DOCS_BKUP_08-18-2025` for historical continuity.
3. read all instructions .github/instructions/ ensure SSOT.
4. Regenerate each domain doc in `docs/`:
   - `core_software.md`
   - `communications.md`
   - `telemetry.md`
   - `motor_control.md`
   - `safety.md`
   - `build_testing.md`
   - `simulation.md`
   - `documentation_ssot.md`
5. Optimize for human + AI consumption:
   - Semantic anchors (`## Domain`, `### Subsystem`)
   - Inline summaries for AI handoff
   - `last_updated:` timestamp at top

## 🧪 Validation

- Confirm SSOT alignment via scripts\validate_ssot.py
- Validate modularity via cross-links in `documentation_ssot.md`

## ⚙️ GitHub Action Offload

- Push this prompt to `.copilot-tasks/regenerate_docs.md`
- Trigger `regenerate-docs.yml` workflow in `.github/workflows/`
- Monitor output in `docs/` and commit results with squash strategy

## ✅ Output Format

- Markdown files only
- Regenerated files overwrite existing ones in `docs/`
- Include commit message: `docs: regenerate domain docs via Copilot++ offload`

## 🧠 Notes

- Use GPT-5 Mini for regeneration
- Respect SSOT overrides and workspace references
- Preserve traceability and modularity`
- Output target: `docs/`

## 🧾 Regeneration Instructions

1. Ingest `docs/STATUS.md` as primary context anchor.
2. Reference archived docs in `docs_archive/2025-08-18/` for historical continuity.
3. Apply SSOT-driven overrides from `src/config/ssot.yaml`.
4. Regenerate each domain doc in `docs/`:
   - `core_software.md`
   - `communications.md`
   - `telemetry.md`
   - `motor_control.md`
   - `safety.md`
   - `build_testing.md`
   - `documentation_ssot.md`
5. Optimize for human + AI consumption:
   - Semantic anchors (`## Domain`, `### Subsystem`)
   - Inline summaries for AI handoff
   - `last_updated:` timestamp at top

## 🖼️ Mermaid Diagram Generation

- Generate Mermaid diagrams, or vertical flow-charts, etc. for each domain where architecture, flow, or relationships are relevant:
- FreeRTOS task layout
- Hardware topology (MCU, drivers, sensors)
- Communication protocols (CAN, UART, SPI/I2C)
- Safety system layers
- Build/test pipelines
- Use GitHub-compatible syntax:
- Avoid htmlLabels: true
- Prefer flowchart TD or graph LR
- Use simple node labels and avoid unsupported styling
- Embed diagrams using fenced code blocks:

```mermaid
graph LR
  MCU --> SPI --> L6470
  L6470 --> MOTOR
```

- Place diagrams directly under relevant headings with a short caption
- Validate rendering in GitHub Markdown preview (e.g. VS Code)

## 🧪 Validation

- Confirm SSOT alignment via checksum in `ssot.yaml`
- Validate modularity via cross-links in `documentation_ssot.md`

## ⚙️ GitHub Action Offload

- Push this prompt to `.copilot-tasks/regenerate_docs.md`
- Trigger `regenerate-docs.yml` workflow in `.github/workflows/`
- Monitor output in `docs/` and commit results with squash strategy

## ✅ Output Format

- Markdown files only
- Regenerated files overwrite existing ones in `docs/`
- Include commit message: `docs: regenerate domain docs via Copilot++ offload`

## 🧠 Notes

- Use GPT-5 Mini for regeneration
- Respect SSOT overrides and workspace references
- Preserve traceability and modularity
- Ensure Mermaid diagrams are GitHub-renderable and contextually placed
