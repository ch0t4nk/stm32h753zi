"""Parse Clang JSON AST files and extract TypedefDecl/EnumDecl/RecordDecl names and locations.

Usage: python scripts/parse_clang_json_ast.py --input <path-to-json> [--csv-out <path>]

This script is lightweight and only needs Python's stdlib.
"""
import json
import argparse
from pathlib import Path

KIND_NAMES = {"TypedefDecl", "EnumDecl", "RecordDecl"}


def walk(node):
    if not isinstance(node, dict):
        return
    kind = node.get("kind")
    if kind in KIND_NAMES:
        name = node.get("name")
        loc = node.get("loc") or {}
        file = loc.get("file")
        line = loc.get("line")
        col = loc.get("col")
        yield {"kind": kind, "name": name, "file": file, "line": line, "col": col}
    for v in node.values():
        if isinstance(v, list):
            for item in v:
                yield from walk(item)
        elif isinstance(v, dict):
            yield from walk(v)


def parse_file(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    results = list(walk(data))
    return results


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("--input", required=True, help="Clang JSON AST file to parse")
    p.add_argument("--csv-out", default=None, help="Optional CSV output path")
    args = p.parse_args()
    res = parse_file(args.input)
    if args.csv_out:
        import csv
        outp = Path(args.csv_out)
        outp.parent.mkdir(parents=True, exist_ok=True)
        with open(outp, "w", newline="", encoding="utf-8") as cf:
            writer = csv.DictWriter(cf, fieldnames=["kind", "name", "file", "line", "col"])
            writer.writeheader()
            for r in res:
                writer.writerow(r)
        print(f"Wrote CSV: {outp}")
    else:
        for r in res:
            print(r)

    print(f"Total declarations: {len(res)}")
