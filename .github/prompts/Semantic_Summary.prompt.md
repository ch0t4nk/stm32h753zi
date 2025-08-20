name: semantic-summary
description: >
  Generate a structured, timestamped semantic summary of our conversation to date.
  The output must reflect domain state, decision anchors, and traceable reasoning paths.
  Use modular formatting to support downstream validation and multi-agent orchestration.

sections:
  - name: Decisions
    description: >
      Enumerate key architectural choices, workflow strategies, or design principles we've committed to.
      Include rationale or triggering context where relevant.

  - name: Code Snippets
    description: >
      Provide concise, context-anchored code fragments referenced or generated during the session.
      Annotate each snippet with its purpose and phase (e.g., bootloader failsafe, teardown script).

  - name: Open Questions
    description: >
      Identify unresolved issues, edge cases, or areas requiring further validation.
      Include any assumptions that need confirmation.

  - name: Next Steps
    description: >
      Outline concrete action items, modular tasks, or orchestration phases to proceed.
      Use semantic anchors and timestamps to support traceability.

formatting:
  use_section_headings: true
  use_bullet_points: true
  include_timestamps: true
  avoid_placeholders_or_stubs: true
  enforce_completeness: true
  maintain_SSOT_alignment: true
  support_validation_readiness: true

validation_criteria:
  - output_must_be_modular: true
  - output_must_be_traceable: true
  - output_must_support_multi_pass_workflows: true
  - all_sections_must_be_populated: true
  - summary_must_support_downstream_automation: true