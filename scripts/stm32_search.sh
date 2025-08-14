#!/bin/bash
"""
High-Performance STM32H753ZI Semantic Search Wrapper

Fast semantic search with automatic virtual environment handling.
Provides sub-2-second query response times with lazy collection loading.

Usage:
    ./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
    ./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7
    ./scripts/stm32_search.sh peripheral "SPI" --scope all
    ./scripts/stm32_search.sh register "ABS_POS" --scope L6470

Performance Features:
- Lazy collection loading (only loads needed collections)
- Intelligent scope mapping (STM32H7 -> stm32_hal collection)
- Persistent ChromaDB connections
- Sub-2-second query response times

Author: STM32H753ZI Project
Date: 2025-08-11
Feature: FTR-014 High-Performance Semantic Search System
"""

set -e  # Exit on any error

# Project paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VENV_PATH="$PROJECT_ROOT/.venv"
SEMANTIC_SCRIPT="$SCRIPT_DIR/semantic_search_optimized.py"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if virtual environment exists
if [ ! -f "$VENV_PATH/bin/python" ]; then
    print_error "Virtual environment not found at $VENV_PATH"
    print_error "Run: python -m venv .venv && .venv/bin/pip install -r requirements.txt"
    exit 1
fi

# Check if semantic database exists
DB_PATH="$PROJECT_ROOT/docs/semantic_vector_db"
if [ ! -d "$DB_PATH" ]; then
    print_error "Semantic database not found at $DB_PATH"
    print_error "Run: .venv/bin/python scripts/init_vector_db.py"
    exit 1
fi

# Check if optimized search script exists
if [ ! -f "$SEMANTIC_SCRIPT" ]; then
    print_error "Optimized search script not found: $SEMANTIC_SCRIPT"
    exit 1
fi

# Show usage if no arguments
if [ $# -eq 0 ]; then
    echo "STM32H753ZI High-Performance Semantic Search"
    echo "============================================="
    echo ""
    echo "Usage: $0 <query_type> <query> [options]"
    echo ""
    echo "Query Types:"
    echo "  concept    - Search for concepts and topics"
    echo "  function   - Search for specific functions"  
    echo "  peripheral - Search for peripheral configurations"
    echo "  register   - Search for register information"
    echo ""
    echo "Scopes:"
    echo "  STM32H7      - STM32H7 HAL documentation (241k docs)"
    echo "  L6470        - L6470/motor control documentation (9k docs)"
    echo "  project      - Project source code (187 docs)"
    echo "  instructions - Development guides (1.8k docs)"
    echo "  all          - All collections (252k docs)"
    echo ""
    echo "Examples:"
    echo "  $0 concept 'GPIO configuration' --scope STM32H7"
    echo "  $0 function 'HAL_GPIO_Init' --scope STM32H7"
    echo "  $0 peripheral 'SPI' --scope all"
    echo "  $0 register 'ABS_POS' --scope L6470"
    echo ""
    echo "Options:"
    echo "  --scope <scope>        Search scope (default: all)"
    echo "  --max-results <num>    Maximum results (default: 10)"
    echo "  --no-content          Hide result content"
    echo "  --json                Output as JSON"
    echo ""
    echo "Performance: Target <2 seconds per query with lazy loading"
    exit 0
fi

# Validate query type
QUERY_TYPE="$1"
case "$QUERY_TYPE" in
    concept|function|peripheral|register)
        ;;
    *)
        print_error "Invalid query type: $QUERY_TYPE"
        print_error "Valid types: concept, function, peripheral, register"
        exit 1
        ;;
esac

# Check if query is provided
if [ $# -lt 2 ]; then
    print_error "Query text required"
    print_error "Usage: $0 $QUERY_TYPE '<query_text>' [options]"
    exit 1
fi

# Show quick performance info for first-time users
if [ ! -f "$HOME/.stm32_search_info_shown" ]; then
    print_status "STM32H753ZI High-Performance Semantic Search"
    print_status "Performance: Sub-2-second queries with lazy collection loading"
    print_status "Collections: STM32H7 (241k), L6470 (9k), Project (187), Guides (1.8k)"
    echo ""
    touch "$HOME/.stm32_search_info_shown"
fi

# Execute semantic search with virtual environment
print_status "Executing semantic search..."
exec "$VENV_PATH/bin/python" "$SEMANTIC_SCRIPT" "$@"
