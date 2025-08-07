#!/usr/bin/env python3
"""
Create search index for STM32H7xx Nucleo BSP documentation
"""

import json
from pathlib import Path


def create_nucleo_bsp_index():
    """Create search index for Nucleo BSP documentation"""

    bsp_docs_dir = Path(
        "/workspaces/code/00_reference/stm32h7xx_nucleo_bsp_md"
    )
    index_file = Path(
        "/workspaces/code/docs/indexes/STM32H7xx_Nucleo_BSP_INDEX.json"
    )

    print(f"🔍 Creating Nucleo BSP search index...")
    print(f"📁 Source: {bsp_docs_dir}")
    print(f"📄 Output: {index_file}")

    # Create index structure
    index = {
        "metadata": {
            "documentation_set": "STM32H7xx_Nucleo_BSP",
            "description": "STM32H7xx Nucleo-144 Board Support Package Documentation",
            "version": "STM32H7 HAL Version",
            "total_files": 0,
            "keywords_count": 0,
            "creation_date": "2025-08-04",
        },
        "files": {},
        "functions": {},
        "constants": {},
        "concepts": {},
    }

    # Ensure index directory exists
    index_file.parent.mkdir(parents=True, exist_ok=True)

    # Process markdown files
    functions_found = set()
    constants_found = set()

    for md_file in bsp_docs_dir.glob("*.md"):
        try:
            with open(md_file, "r", encoding="utf-8") as f:
                content = f.read()

            # Extract file info
            relative_path = str(md_file.relative_to(Path("/workspaces/code")))
            file_info = {
                "title": md_file.stem.replace("_", " ").title(),
                "path": relative_path,
                "size": len(content),
                "keywords": [],
            }

            # Look for BSP functions
            import re

            bsp_functions = re.findall(r"BSP_[A-Z_]+[a-zA-Z_]*", content)
            for func in bsp_functions:
                functions_found.add(func)
                file_info["keywords"].append(func)

                if func not in index["functions"]:
                    index["functions"][func] = []
                index["functions"][func].append(
                    {"file": relative_path, "title": file_info["title"]}
                )

            # Look for LED/BUTTON constants
            led_constants = re.findall(r"LED[0-9_A-Z]*|BUTTON[_A-Z]*", content)
            for const in led_constants:
                constants_found.add(const)
                file_info["keywords"].append(const)

                if const not in index["constants"]:
                    index["constants"][const] = []
                index["constants"][const].append(
                    {"file": relative_path, "title": file_info["title"]}
                )

            # Add to files index
            index["files"][relative_path] = file_info

        except Exception as e:
            print(f"⚠️  Error processing {md_file}: {e}")

    # Add concept mappings
    index["concepts"] = {
        "board_support": list(index["files"].keys()),
        "nucleo_144": [
            f for f in index["files"].keys() if "nucleo" in f.lower()
        ],
        "bsp_drivers": [
            f for f in index["files"].keys() if "bsp" in f.lower()
        ],
        "led_control": [
            f for f in index["files"].keys() if "led" in f.lower()
        ],
        "button_control": [
            f for f in index["files"].keys() if "button" in f.lower()
        ],
    }

    # Update metadata
    index["metadata"]["total_files"] = len(index["files"])
    index["metadata"]["keywords_count"] = len(functions_found) + len(
        constants_found
    )

    # Save index
    with open(index_file, "w", encoding="utf-8") as f:
        json.dump(index, f, indent=2)

    print(f"✅ Index created successfully!")
    print(f"📄 Files indexed: {len(index['files'])}")
    print(f"🔧 Functions found: {len(functions_found)}")
    print(f"📋 Constants found: {len(constants_found)}")
    print(f"🎯 Total keywords: {index['metadata']['keywords_count']}")

    return index


if __name__ == "__main__":
    create_nucleo_bsp_index()
