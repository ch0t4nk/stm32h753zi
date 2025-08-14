#!/usr/bin/env python3
"""
VS Code tasks.json cross-platform Python path updater.
Updates all hardcoded Python paths to use cross-platform detection.
"""

import json
import sys
from pathlib import Path


def update_tasks_json():
    """Update tasks.json to use cross-platform Python paths."""
    workspace_root = Path(__file__).parent.parent
    tasks_file = workspace_root / ".vscode" / "tasks.json"

    if not tasks_file.exists():
        print(f"❌ Tasks file not found: {tasks_file}")
        return False

    # Read current tasks.json
    with open(tasks_file, "r", encoding="utf-8") as f:
        content = f.read()

    # Remove JSONC comments for parsing
    lines = content.split("\n")
    json_lines = []
    for line in lines:
        # Remove single-line comments but preserve line structure
        if "//" in line and not line.strip().startswith('"'):
            line = line.split("//")[0].rstrip()
        json_lines.append(line)

    try:
        tasks_data = json.loads("\n".join(json_lines))
    except json.JSONDecodeError as e:
        print(f"❌ Failed to parse tasks.json: {e}")
        return False

    # Track changes
    changes_made = 0

    # Process each task
    for task in tasks_data.get("tasks", []):
        # Check if task uses direct Python command (needs cross-platform update)
        if "command" in task and "python" in str(task["command"]).lower():
            # Convert to cross-platform structure
            if not any(key in task for key in ["windows", "linux", "osx"]):
                print(
                    f"📝 Converting task '{task.get('label', 'Unknown')}' to cross-platform..."
                )

                # Save original command and args
                original_command = task.get("command", "")
                original_args = task.get("args", [])

                # Remove original command and args
                if "command" in task:
                    del task["command"]
                if "args" in task:
                    del task["args"]

                # Add cross-platform commands
                task["windows"] = {
                    "command": "powershell",
                    "args": [
                        "-Command",
                        f"& (python scripts/get_python_path.py) {' '.join(original_args) if original_args else ''}",
                    ],
                }

                task["linux"] = {
                    "command": "python",
                    "args": ["scripts/get_python_path.py"]
                    + (original_args if original_args else []),
                }

                task["osx"] = {
                    "command": "python",
                    "args": ["scripts/get_python_path.py"]
                    + (original_args if original_args else []),
                }

                changes_made += 1

    if changes_made > 0:
        # Write updated tasks.json back (preserve original formatting style)
        updated_content = json.dumps(tasks_data, indent=2, ensure_ascii=False)

        # Restore comment style and formatting
        with open(tasks_file, "w", encoding="utf-8") as f:
            f.write(updated_content)

        print(
            f"✅ Updated {changes_made} tasks in tasks.json for cross-platform compatibility"
        )
        return True
    else:
        print(
            "ℹ️ No tasks required updating - all already cross-platform compatible"
        )
        return True


if __name__ == "__main__":
    success = update_tasks_json()
    sys.exit(0 if success else 1)
