generate_docs_runner

This script prepares prompts from `.copilot-tasks/generate_docs.md` and supports calling a local Ollama instance to generate domain markdown files.

Usage (dry-run, default):

```powershell
python .\scripts\generate_docs_runner.py --prompt .copilot-tasks\generate_docs.md --dry-run
```

To execute against local Ollama and commit locally to a branch (no push):

```powershell
python .\scripts\generate_docs_runner.py --prompt .copilot-tasks\generate_docs.md --execute --commit --branch docs/regeneration
```

Notes:

- Script will create `.copilot-tasks/prompts/` with per-domain prompt files.
- Ollama must be installed and the model name available locally (default `gpt-OSS:20b`).
- The script will not push commits remotely. Use your normal git workflow to open PRs.
