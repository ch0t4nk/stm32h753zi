SSOT Verification Report

This PR includes the repository SSOT verification run. The CI job `SSOT Verification (clang AST)` will run automatically on this PR and produce the following artifacts:

- `build/all_decls.csv` — full list of typedef/enum/record declarations.
- `build/overlaps.csv` — overlap report between `src/config` and other `src` files.

Interpretation checklist for reviewers

- [ ] `overlaps.csv` is empty (no overlaps), or
- [ ] If `overlaps.csv` contains rows, verify `other_files` column: rows pointing outside `src/config` must be reviewed and resolved before merge.
- [ ] If the overlap is macro-generated, consider deferring to the code owner and/or running a preprocessed AST analysis.

If you need help interpreting the artifacts, ping the maintainer team and reference `docs/SSOT_VERIFICATION.md`.
