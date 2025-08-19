#!/usr/bin/env python3
"""
Simple brace balance checker for C-like files.
Prints counts and first line where balance goes negative.
"""
import sys
from pathlib import Path


def check_file(path: Path):
    if not path.exists():
        print(f"File not found: {path}")
        return 2
    s = path.read_text(encoding='utf-8', errors='ignore')
    open_count = s.count('{')
    close_count = s.count('}')
    print(f"open {open_count} close {close_count} diff {open_count - close_count}")
    bal = 0
    for i, line in enumerate(s.splitlines(), start=1):
        bal += line.count('{') - line.count('}')
        if bal < 0:
            print(f"negative balance at line {i}")
            break
    print(f"final balance {bal}")
    return 0 if bal == 0 and open_count == close_count else 1


def main(argv):
    if len(argv) < 2:
        print("Usage: check_braces.py <path-to-file>")
        return 2
    return check_file(Path(argv[1]))


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))
