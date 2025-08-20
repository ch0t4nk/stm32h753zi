"""
AST-based verifier for typedef/struct/enum definitions.
Scans C header/source files under src/, parses with pycparser, and reports types defined in src/config vs elsewhere.
Usage:
    python scripts/ast_verify_types.py
"""
import os
import sys
from collections import defaultdict

try:
    from pycparser import c_parser, parse_file
    from pycparser.c_ast import Typedef, Struct, Enum
except Exception as e:
    print("pycparser is required. Install with: pip install pycparser")
    raise

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
SRC = os.path.join(ROOT, 'src')
CONFIG_DIR = os.path.join(SRC, 'config')

# Files to parse (headers and C files)
files = []
for dirpath, dirs, filenames in os.walk(SRC):
    for fn in filenames:
        if fn.endswith(('.h', '.c')):
            files.append(os.path.join(dirpath, fn))

parser = c_parser.CParser()

# Helper to parse a file into AST nodes (pycparser expects preprocessed C; we'll do a best-effort parse)
def parse_content(path):
    try:
        # Use parse_file with fake includes, rely on pycparser for typedef/struct/enum nodes
        ast = parse_file(path, use_cpp=True, cpp_args=['-E', r'-I' + os.path.join(sys.prefix, 'include')])
        return ast
    except Exception:
        # Fallback: try to read file and parse directly (may fail if complex macros are present)
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as fh:
                content = fh.read()
            ast = parser.parse(content, filename=path)
            return ast
        except Exception:
            return None

# Collect types per file
types_by_file = defaultdict(list)

for f in files:
    ast = parse_content(f)
    if ast is None:
        continue
    for node in ast.ext:
        if isinstance(node, Typedef):
            types_by_file[f].append(('typedef', node.name))
        elif isinstance(node, Struct):
            # named structs
            if node.name:
                types_by_file[f].append(('struct', node.name))
        elif isinstance(node, Enum):
            if node.name:
                types_by_file[f].append(('enum', node.name))

# Build maps
config_types = defaultdict(list)
other_types = defaultdict(list)
for f, items in types_by_file.items():
    rel = os.path.relpath(f, ROOT)
    bucket = config_types if os.path.commonpath([os.path.abspath(f), CONFIG_DIR]) == os.path.abspath(CONFIG_DIR) else other_types
    for t, name in items:
        bucket[name].append(rel)

# Compare
overlaps = []
for name in config_types:
    if name in other_types:
        overlaps.append((name, config_types[name], other_types[name]))

# Print summary
print(f"Config files scanned: {len([p for p in files if p.startswith(CONFIG_DIR)])}")
print(f"Other src files scanned: {len(files) - len([p for p in files if p.startswith(CONFIG_DIR)])}")
print(f"Config-defined types: {len(config_types)}")
print(f"Other-defined types: {len(other_types)}")
print(f"Overlapping type names: {len(overlaps)}\n")

if overlaps:
    print("Overlaps found:\n")
    for name, cfg_locs, other_locs in overlaps:
        print(f"Type: {name}")
        print("  In config:")
        for p in cfg_locs:
            print(f"    {p}")
        print("  In other src files:")
        for p in other_locs[:10]:
            print(f"    {p}")
        print()
else:
    print("No overlapping type definitions were found.")

# Exit status
sys.exit(0 if not overlaps else 2)
