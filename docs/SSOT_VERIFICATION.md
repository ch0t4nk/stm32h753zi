# SSOT Verification (clang AST)

This document explains the `SSOT Verification` workflow added to the repository.

What it enforces

- Ensures types (typedefs, enums, records/structs) defined in `src/config/` are not redefined in other `src/` files.
- Uses clang's JSON AST (`-Xclang -ast-dump=json`) to extract `TypedefDecl`, `EnumDecl`, and `RecordDecl` nodes across translation units.
- Aggregates declarations and reports overlaps in `build/overlaps.csv`.

Files produced by the verification

- `build/ast-json/` — per-file clang JSON AST files.
- `build/all_decls.csv` — CSV of all declarations extracted across the repository.
- `build/overlaps.csv` — CSV of overlapping declaration names between `src/config` and other `src/` files.

How to run locally

1. Ensure `clang` is installed and available in PATH.
2. Activate the repo virtualenv if you use Python tools:

```powershell
& ".\.venv\Scripts\Activate.ps1"
```

3. Generate JSON ASTs and aggregate:

```powershell
$dst='build\ast-json'; New-Item -ItemType Directory -Force -Path $dst
Get-ChildItem -Path src -Recurse -Include *.h,*.c,*.hpp,*.cpp |
  ForEach-Object {
    $f=$_.FullName
    $rel=$f.Substring((Get-Location).Path.Length+1) -replace '[\\/]', '_'
    $out=Join-Path $dst ($rel + '.json')
    clang -Xclang -ast-dump=json -fsyntax-only -I src "$f" > "$out" 2>&1
  }

& ".\.venv\Scripts\python.exe" scripts\aggregate_decls.py --ast-dir build/ast-json --out-csv build/all_decls.csv --overlap build/overlaps.csv
```

Interpreting `build/overlaps.csv`

- The CSV columns: `name,config_files,other_files`.
- `name` is the declared symbol (typedef/enum/record).
- `config_files` lists locations under `src/config` where the symbol is defined.
- `other_files` lists locations outside `src/config` where the symbol is also defined.

If `other_files` is empty, the symbol only appears in `src/config`. If `other_files` contains paths not under `src/config`, that indicates a potential SSOT violation.

CI behavior

- The repository has a GitHub Actions workflow `.github/workflows/ssot-verify.yml` that runs this check on PRs and manually via `workflow_dispatch`.
- The job uploads artifacts for auditing.
- The job fails only when an overlap references non-`src/config` files (to avoid false-positives for types that are legitimately only in `src/config`).

Follow-up

- For macro-generated types or TU-specific macro behavior, consider running a stricter preprocessed AST pass (Option B). See `docs/README.md` or contact the maintainers if you need this stricter enforcement.
