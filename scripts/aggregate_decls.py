"""Aggregate declarations from clang JSON AST files and compare src/config vs others.

Usage: python scripts/aggregate_decls.py --ast-dir build/ast-json --out-csv build/all_decls.csv --overlap build/overlaps.csv
"""
import json
import argparse
from pathlib import Path

KIND_NAMES = {"TypedefDecl", "EnumDecl", "RecordDecl"}


def walk(node, results):
    if not isinstance(node, dict):
        return
    kind = node.get("kind")
    if kind in KIND_NAMES:
        name = node.get("name")
        loc = node.get("loc") or {}
        file = loc.get("file")
        line = loc.get("line")
        col = loc.get("col")
        results.append({"kind": kind, "name": name, "file": file, "line": line, "col": col})
    for v in node.values():
        if isinstance(v, list):
            for item in v:
                walk(item, results)
        elif isinstance(v, dict):
            walk(v, results)


def parse_json(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except Exception:
        return []
    results = []
    walk(data, results)
    return results


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--ast-dir", required=True)
    p.add_argument("--out-csv", required=True)
    p.add_argument("--overlap", required=True)
    args = p.parse_args()
    ast_dir = Path(args.ast_dir)
    all_decls = []
    for jf in ast_dir.rglob("*.json"):
        rel = jf.relative_to(ast_dir)
        decls = parse_json(jf)
        # Normalize file path: prefer the actual file path in AST 'file' if present, else reconstruct from JSON filename
        for d in decls:
            if not d.get("file"):
                # try to recover original filename from json filename
                name = str(rel).replace('_', '\\').rsplit('.json',1)[0]
                # best-effort, not perfect
                d['file'] = name
        all_decls.extend(decls)

    # write all_decls CSV
    import csv
    outp = Path(args.out_csv)
    outp.parent.mkdir(parents=True, exist_ok=True)
    with open(outp, "w", newline='', encoding='utf-8') as cf:
        writer = csv.DictWriter(cf, fieldnames=["kind", "name", "file", "line", "col"])
        writer.writeheader()
        for r in all_decls:
            writer.writerow(r)
    print(f"Wrote all declarations: {outp} (count={len(all_decls)})")

    # Build maps
    config_decls = {}
    other_decls = {}
    for d in all_decls:
        name = d.get('name')
        if not name:
            continue
        file = d.get('file','') or ''
        key = name
        if '\\src\\config\\' in file.replace('/', '\\').lower() or 'src_config' in file.lower():
            config_decls.setdefault(key, []).append(d)
        else:
            other_decls.setdefault(key, []).append(d)

    overlaps = []
    for name, clist in config_decls.items():
        if name in other_decls:
            overlaps.append({"name": name, "config_files": ';'.join(sorted({c.get('file') or '' for c in clist})), "other_files": ';'.join(sorted({o.get('file') or '' for o in other_decls[name]}))})

    out_ov = Path(args.overlap)
    with open(out_ov, 'w', newline='', encoding='utf-8') as of:
        writer = csv.DictWriter(of, fieldnames=["name","config_files","other_files"])
        writer.writeheader()
        for o in overlaps:
            writer.writerow(o)
    print(f"Wrote overlaps: {out_ov} (count={len(overlaps)})")
