#!/bin/bash
# STM32 Semantic Search Wrapper Script
# Ensures virtual environment is always used for ChromaDB dependencies

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
WORKSPACE_ROOT="$(dirname "$SCRIPT_DIR")"
VENV_PYTHON="$WORKSPACE_ROOT/.venv/bin/python"
SEARCH_SCRIPT="$WORKSPACE_ROOT/scripts/stm32_semantic_search.py"

# Check if virtual environment exists
if [ ! -f "$VENV_PYTHON" ]; then
    echo "❌ Virtual environment not found at $VENV_PYTHON"
    echo "💡 Run: python3 -m venv .venv && .venv/bin/pip install chromadb requests"
    exit 1
fi

# Check if semantic search script exists
if [ ! -f "$SEARCH_SCRIPT" ]; then
    echo "❌ Semantic search script not found at $SEARCH_SCRIPT"
    exit 1
fi

# Run semantic search with virtual environment python
exec "$VENV_PYTHON" "$SEARCH_SCRIPT" "$@"
