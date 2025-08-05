#!/bin/bash
"""
STM32 Semantic Search Wrapper
Ensures virtual environment is always used for semantic search operations
"""

# Get the workspace root directory
WORKSPACE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_PYTHON="${WORKSPACE_ROOT}/.venv/bin/python"

# Check if virtual environment exists
if [[ ! -f "$VENV_PYTHON" ]]; then
    echo "❌ Virtual environment not found at $VENV_PYTHON"
    echo "💡 Run: python3 -m venv .venv && .venv/bin/pip install chromadb requests"
    exit 1
fi

# Run the semantic search with virtual environment python
exec "$VENV_PYTHON" "${WORKSPACE_ROOT}/scripts/stm32_semantic_search.py" "$@"
