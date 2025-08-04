#!/usr/bin/env python3
"""
Enhanced Documentation Search Tool
Supports both STM32H7 and L6470 documentation search
Uses SSOT documentation configuration for all paths
"""

import json
import os
import sys
from pathlib import Path

# SSOT Documentation Configuration - matches documentation_config.h
WORKSPACE_ROOT = Path('/workspaces/code')
DOC_INDEX_STM32H7_FULL = WORKSPACE_ROOT / \
    "docs" / "indexes" / "STM32H7_FULL_INDEX.json"
DOC_INDEX_L6470_SEARCH = WORKSPACE_ROOT / \
    "docs" / "indexes" / "L6470_SEARCH_INDEX.json"
DOC_INDEX_STM32H7_COPILOT_JSON = WORKSPACE_ROOT / \
    "docs" / "indexes" / "STM32H7_COPILOT_INDEX.json"


def load_stm32h7_index():
    """Load STM32H7 documentation index using SSOT path"""
    try:
        with open(DOC_INDEX_STM32H7_FULL, 'r') as f:
            return json.load(f)
    except FileNotFoundError:
        print(
            f"❌ STM32H7 index not found at SSOT path: {DOC_INDEX_STM32H7_FULL}")
        return None


def load_l6470_index():
    """Load L6470 documentation index using SSOT path"""
    try:
        with open(DOC_INDEX_L6470_SEARCH, 'r') as f:
            return json.load(f)
    except (FileNotFoundError, IOError):
        print(
            f"❌ L6470 index not found at SSOT path: {DOC_INDEX_L6470_SEARCH}")
        return None


def search_stm32h7(search_type, query, index):
    """Search STM32H7 documentation"""
    if not index:
        return []

    results = []
    query_lower = query.lower()

    if search_type == "peripheral":
        for peripheral, files in index.get('peripherals', {}).items():
            if query_lower in peripheral.lower():
                results.extend(files)

    elif search_type == "function":
        for function, files in index.get('functions', {}).items():
            if query_lower in function.lower():
                results.extend(files)

    elif search_type == "concept":
        for keyword, files in index.get('keywords', {}).items():
            if query_lower in keyword.lower():
                results.extend(files)

    return list(set(results))  # Remove duplicates


def search_l6470(search_type, query, index):
    """Search L6470 documentation"""
    if not index:
        return []

    results = []
    query_lower = query.lower()

    if search_type == "function":
        for function, files in index.get('functions', {}).items():
            if query_lower in function.lower():
                results.extend(files)

    elif search_type == "register":
        for register, files in index.get('registers', {}).items():
            if query_lower in register.lower():
                results.extend(files)

    elif search_type == "concept":
        for concept, files in index.get('concepts', {}).items():
            if query_lower in concept.lower():
                results.extend(files)
        # Also search keywords
        for keyword, files in index.get('keywords', {}).items():
            if query_lower in keyword.lower():
                results.extend(files)

    return list(set(results))  # Remove duplicates


def format_results(results, search_type, query, doc_type):
    """Format search results for display"""
    if not results:
        print(
            f"❌ {search_type.title()} '{query}' not found in {doc_type} documentation")
        return

    print(
        f"✅ Found {len(results)} {doc_type} {search_type} results for '{query}':")
    print("=" * 60)

    for i, file_path in enumerate(results[:10], 1):  # Limit to top 10
        filename = Path(file_path).name
        print(f"{i:2d}. {filename}")
        print(f"    📁 {file_path}")

    if len(results) > 10:
        print(f"\n... and {len(results) - 10} more results")


def show_usage():
    """Show usage information"""
    print("Enhanced Documentation Search Tool")
    print("=" * 40)
    print("Usage: python3 search_enhanced_docs.py <type> <query> [scope]")
    print()
    print("Search Types:")
    print("  peripheral <name>  - Search STM32H7 peripherals (GPIO, SPI, I2C, etc.)")
    print("  function <name>    - Search functions in both STM32H7 and L6470 docs")
    print("  register <name>    - Search L6470 registers (ABS_POS, SPEED, etc.)")
    print("  concept <name>     - Search concepts in both documentation sets")
    print()
    print("Scope (optional):")
    print("  stm32h7           - Search only STM32H7 documentation")
    print("  l6470             - Search only L6470 documentation")
    print("  all               - Search both (default)")
    print()
    print("Examples:")
    print("  python3 search_enhanced_docs.py peripheral GPIO")
    print("  python3 search_enhanced_docs.py function L6470_Init")
    print("  python3 search_enhanced_docs.py register ABS_POS")
    print("  python3 search_enhanced_docs.py concept stepper l6470")


def main():
    if len(sys.argv) < 3:
        show_usage()
        return

    search_type = sys.argv[1].lower()
    query = sys.argv[2]
    scope = sys.argv[3].lower() if len(sys.argv) > 3 else "all"

    # Load indexes
    stm32h7_index = load_stm32h7_index()
    l6470_index = load_l6470_index()

    # Validate scope
    if scope not in ["all", "stm32h7", "l6470"]:
        print(f"❌ Invalid scope: {scope}. Use 'all', 'stm32h7', or 'l6470'")
        return

    # Search based on type and scope
    if search_type == "peripheral":
        if scope in ["all", "stm32h7"]:
            results = search_stm32h7("peripheral", query, stm32h7_index)
            format_results(results, "peripheral", query, "STM32H7")
        else:
            print("❌ Peripheral search only available for STM32H7 documentation")

    elif search_type == "function":
        if scope in ["all", "stm32h7"]:
            results = search_stm32h7("function", query, stm32h7_index)
            if results:
                format_results(results, "function", query, "STM32H7")

        if scope in ["all", "l6470"]:
            results = search_l6470("function", query, l6470_index)
            if results:
                format_results(results, "function", query, "L6470")

        if scope == "all" and not any([
            search_stm32h7("function", query, stm32h7_index),
            search_l6470("function", query, l6470_index)
        ]):
            print(
                f"❌ Function '{query}' not found in either documentation set")

    elif search_type == "register":
        if scope in ["all", "l6470"]:
            results = search_l6470("register", query, l6470_index)
            format_results(results, "register", query, "L6470")
        else:
            print("❌ Register search only available for L6470 documentation")

    elif search_type == "concept":
        if scope in ["all", "stm32h7"]:
            results = search_stm32h7("concept", query, stm32h7_index)
            if results:
                format_results(results, "concept", query, "STM32H7")

        if scope in ["all", "l6470"]:
            results = search_l6470("concept", query, l6470_index)
            if results:
                format_results(results, "concept", query, "L6470")

        if scope == "all" and not any([
            search_stm32h7("concept", query, stm32h7_index),
            search_l6470("concept", query, l6470_index)
        ]):
            print(f"❌ Concept '{query}' not found in either documentation set")

    else:
        print(f"❌ Invalid search type: {search_type}")
        print("Use: peripheral, function, register, or concept")


if __name__ == "__main__":
    main()
