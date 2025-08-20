---

mode: agent
task: regenerate_full_domain_docs
priority: high
reasoning: deep # Demand multi‑pass reasoning before writing
anchors:

- SSOT: src/config/ssot.yaml
- STATUS: docs/STATUS.md
- ARCHIVE: archive/outdated_docs/DOCS_BKUP_08-18-2025
- DOMAINS:
  - core_software.md
  - communications.md
  - telemetry.md
  - motor_control.md
  - safety.md
  - build_testing.md
  - simulation.md
  - documentation_ssot.md

preconditions:

- "exists('docs/STATUS.md')"
- "exists('src/config/ssot.yaml')"
- "codebase_indexed()"
- "reference_materials_available('00_reference/\*_/_.txt')"

steps:

- id: ingest_context
  run:

  - "read_file('docs/STATUS.md')"
  - "load_yaml('src/config/ssot.yaml')"
  - "read_all('.github/instructions')"
  - "review_archive('archive/outdated_docs/DOCS_BKUP_08-18-2025')"
  - "review_reference_specs('00_reference/\*_/_.txt')"
    ensure:
  - "anchors_present(['SSOT','HAL','RTOS','BUILD'])"

- id: analyse_codebase
  run:

  - "follow_code_chains(start_points=all_public_APIs)"
  - "map_algorithms_in_codebase()"
  - "link_symbols_to_sources_and_headers()"
  - "detect_design_patterns_and_anti_patterns()"
  - "cross_validate_with_reference_specs()"

- id: regenerate_documentation
  for_each: "{DOMAINS}"
  run:

  - "write_full_domain_doc(file=current_item, include_full_API_reference=true, include_algorithm_explanations=true, include_code_examples=true, crosslink_to_related_docs=true, add_mermaid_diagrams=true, add_last_updated_timestamp=true)"
    ensure:
  - "doc_length_above_threshold(min_words=800)"
  - "contains(['Overview','Detailed Description','Algorithm Details','Best Practices','Reference Links'])"

- id: validate_docs
  run:

  - "python scripts/validate_ssot.py"
  - "check_crosslinks('docs/documentation_ssot.md')"
    on_fail:
  - "halt('Validation failed; manual fix required')"

- id: commit_changes
  run:
  - "git add docs/\*.md"
  - "git commit -m 'docs: regenerate full domain docs with deep code+spec analysis'"
  - "git push"

acceptance:

- "all_domain_docs_present_and_non_stub()"
- "passes_ssot_validation()"
- "mermaid_diagrams_render_in_markdown()"

notes:

- "Follow code-paths until leaf functions are documented."
- "Extract and explain algorithms, not just list them."
- "Validate HAL and RTOS integration against manufacturer specs."
- "Stub-level output fails acceptance; regenerate until complete."
