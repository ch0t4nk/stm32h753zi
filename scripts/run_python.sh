#!/bin/bash
# Cross-platform Python script runner for VS Code tasks
# Usage: ./scripts/run_python.sh script_name.py [args...]

# Get the workspace root
WORKSPACE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Try to find Python executable
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    # Windows
    if [[ -f "$WORKSPACE_ROOT/.venv/Scripts/python.exe" ]]; then
        PYTHON_EXE="$WORKSPACE_ROOT/.venv/Scripts/python.exe"
    elif [[ -f "$WORKSPACE_ROOT/.venv/bin/python.exe" ]]; then
        PYTHON_EXE="$WORKSPACE_ROOT/.venv/bin/python.exe"
    elif [[ -f "$WORKSPACE_ROOT/.venv/bin/python" ]]; then
        PYTHON_EXE="$WORKSPACE_ROOT/.venv/bin/python"
    else
        # Better Windows fallback paths
        WINDOWS_PYTHON_PATHS=(
            "C:/Python/python.exe"
            "C:/Python311/python.exe"
            "C:/Python312/python.exe"
            "C:/Python313/python.exe"
            "$LOCALAPPDATA/Programs/Python/Python311/python.exe"
            "$LOCALAPPDATA/Programs/Python/Python312/python.exe"
            "$LOCALAPPDATA/Programs/Python/Python313/python.exe"
        )
        
        PYTHON_EXE=""
        for python_path in "${WINDOWS_PYTHON_PATHS[@]}"; do
            if [[ -f "$python_path" ]]; then
                PYTHON_EXE="$python_path"
                break
            fi
        done
        
        # Final fallback
        if [[ -z "$PYTHON_EXE" ]]; then
            PYTHON_EXE="python"
        fi
    fi
else
    # Linux/macOS
    if [[ -f "$WORKSPACE_ROOT/.venv/bin/python" ]]; then
        PYTHON_EXE="$WORKSPACE_ROOT/.venv/bin/python"
    elif [[ -f "$WORKSPACE_ROOT/.venv/Scripts/python.exe" ]]; then
        PYTHON_EXE="$WORKSPACE_ROOT/.venv/Scripts/python.exe"
    else
        # Better Unix fallback paths
        UNIX_PYTHON_PATHS=(
            "/usr/bin/python3"
            "/usr/bin/python"
            "/usr/local/bin/python3"
            "/usr/local/bin/python"
            "/opt/python/bin/python3"
            "$HOME/.local/bin/python3"
        )
        
        PYTHON_EXE=""
        for python_path in "${UNIX_PYTHON_PATHS[@]}"; do
            if [[ -f "$python_path" ]]; then
                PYTHON_EXE="$python_path"
                break
            fi
        done
        
        # Final fallback
        if [[ -z "$PYTHON_EXE" ]]; then
            PYTHON_EXE="python3"
        fi
    fi
fi

# Run the Python script with all arguments
exec "$PYTHON_EXE" "$@"
