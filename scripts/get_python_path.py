#!/usr/bin/env python3
"""
Cross-platform Python path detector for VS Code tasks.
Returns the correct Python executable path for the current platform.
"""

import os
import sys
import platform
from pathlib import Path


def get_workspace_root():
    """Find the workspace root directory."""
    current = Path(__file__).parent.parent

    # Look for STATUS.md as workspace indicator
    while current != current.parent:
        if (current / "STATUS.md").exists():
            return current
        current = current.parent

    # Fallback to current directory
    return Path.cwd()


def get_python_path():
    """Get the correct Python executable path for the current platform."""
    workspace_root = get_workspace_root()

    # Check if we're on Windows
    is_windows = platform.system() == "Windows" or os.name == "nt"

    if is_windows:
        # Windows: try Scripts directory first
        python_paths = [
            workspace_root / ".venv" / "Scripts" / "python.exe",
            workspace_root / ".venv" / "bin" / "python.exe",  # WSL-style
            workspace_root / ".venv" / "bin" / "python",  # Unix-style fallback
        ]
    else:
        # Unix-like: try bin directory
        python_paths = [
            workspace_root / ".venv" / "bin" / "python",
            workspace_root
            / ".venv"
            / "Scripts"
            / "python.exe",  # Windows-style fallback
        ]

    # Find the first existing Python executable
    for python_path in python_paths:
        if python_path.exists():
            return str(python_path)

    # Fallback to system Python if no venv found
    return sys.executable


if __name__ == "__main__":
    print(get_python_path())
