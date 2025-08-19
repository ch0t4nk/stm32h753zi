STATUS regeneration helper

This helper script composes a draft `STATUS.md` from `.instructions` and
`.github/instructions/*.md` sources. It's intentionally conservative: it
produces a draft and does not commit anything.

Usage (dry-run):

```bash
python scripts/regenerate_status.py --dry-run
```

Write draft to build folder:

```bash
python scripts/regenerate_status.py --out build/STATUS.draft.md
```

When you're happy with the draft, stage and commit manually. Recommended
commit message:

```
chore(ssot): regenerate STATUS.md from .instructions + manual curation
```

Review the draft before committing — `STATUS.md` is a primary context anchor
for the project's automation and must be validated for accuracy.
