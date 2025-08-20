---

mode: agent
task: context_bootstrap
priority: high

anchors:

- SSOT: STATUS.md
- DOMAIN: HAL
- DOMAIN: RTOS
- DOMAIN: BUILD

context:
repo_root: "."
status_file: "STATUS.md"
instructions_dir: ".github/instructions"
docs_dir: "docs/"
reference_dir: "00_reference/"
archive_dir: "archive/old_hal_rtos/"
config_dir: "src/config/"

preconditions:

- check: "exists('{status_file}')"
- check: "exists('{instructions_dir}')"
- check: "exists('{docs_dir}')"
- check: "exists('{reference_dir}')"
- check: "exists('{archive_dir}')"
- check: "exists('{config_dir}')"

steps:

- id: ingest_status
  run:

  - "read_file('{status_file}')"
    ensure:
  - "contains_sections(['HAL integration','SSOT architecture','Build system status','Active issues'])"
  - "anchors_present(['SSOT','HAL','RTOS','BUILD'])"

- id: cross_reference
  run:

  - "follow_file_refs('{status_file}')"
  - "apply_instructions('{instructions_dir}')"
  - "review_docs('{docs_dir}', treat_as='legacy_context')"
  - "load_txt_refs('{reference_dir}')"
  - "retrieve_archive_logic('{archive_dir}')"

- id: enforce_critical
  ensure:
  - "ssot_config_dir('{config_dir}')"
  - "no_redefine_structs_outside('{config_dir}')"
  - "hal_integration_prioritizes(['RTOS compatibility','HAL module enablement','Safety system alignment'])"
  - "no_inference_without_validation()"

side_effects:

- reads: ["{status_file}","{instructions_dir}","{docs_dir}","{reference_dir}","{archive_dir}"]
- sets_context: ["HAL","SSOT","BUILD","Active_issues"]

acceptance:

- "context_initialized()"
- "all_referenced_paths_resolved()"
- "ssot_compliance_verified('{config_dir}')"
- "no_unvalidated_inference()"

errors:

- missing_status:
  action: ["halt('STATUS.md missing; cannot bootstrap')"]
- unresolved_refs:
  action: ["log('Unresolved file references in STATUS.md')", "proceed_with_partial_context()"]
- ssot_violation:
  action: ["log('SSOT compliance failure')", "require_manual_fix()"]

notes:

- "STATUS.md is the primary source of truth; downstream orchestration relies on its integrity."
- "Legacy docs are informative but may be outdated; prioritize STATUS.md and validated references."
- "Reference .txt files in {reference_dir} are authoritative for manufacturer specs/datasheets."
- "Archive assets are a fallback for missing configs."
