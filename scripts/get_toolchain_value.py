#!/usr/bin/env python3
"""Query the JSON workflow_toolchain.json for a key and print it.
Usage: python scripts/get_toolchain_value.py arm_gcc_executable
"""
import json
import sys
import re
import os


def expand_vars(s, ctx):
    if not isinstance(s, str):
        return s
    pattern = re.compile(r"\$\{([^}]+)\}")
    prev = None
    out = s
    for _ in range(10):
        prev = out
        def repl(m):
            key = m.group(1)
            return str(ctx.get(key, m.group(0)))
        out = pattern.sub(repl, out)
        if out == prev:
            break
    return out


def main():
    key = sys.argv[1] if len(sys.argv) > 1 else None
    if not key:
        print("Usage: get_toolchain_value.py <key>", file=sys.stderr)
        sys.exit(2)

    with open('config/workflow_toolchain.json', 'r', encoding='utf-8') as f:
        j = json.load(f)

    # Build a simple context from string values for expansion
    ctx = {k: v for k, v in j.items() if isinstance(v, str)}
    # Expand nested placeholders in context
    for k in list(ctx.keys()):
        ctx[k] = expand_vars(ctx[k], ctx)

    val = j.get(key)
    if val is None:
        print('', end='')
        sys.exit(1)

    if isinstance(val, str):
        out = expand_vars(val, ctx)
        print(out)
    elif isinstance(val, list) and len(val) > 0:
        # Expand candidates and pick the first that exists
        for cand in val:
            if not isinstance(cand, str):
                continue
            expanded = expand_vars(cand, ctx)
            # If running in WSL, attempt to map Windows paths to WSL mount points
            if os.name != 'nt' and ':' in expanded and expanded[1:3] == ':\\':
                # Convert C:\path -> /mnt/c/path (simple conversion)
                drive = expanded[0].lower()
                rest = expanded[3:].replace('\\', '/')
                wsl_path = f"/mnt/{drive}/{rest}"
                if os.path.exists(wsl_path):
                    print(wsl_path)
                    break
            if os.path.exists(expanded):
                print(expanded)
                break
        else:
            # No candidate exists; print expanded first candidate
            first = expand_vars(val[0], ctx)
            print(first)
    else:
        # fallback printing raw value
        print(val)


if __name__ == '__main__':
    main()
