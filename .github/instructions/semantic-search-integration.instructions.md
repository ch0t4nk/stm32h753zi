```instructions
---
applyTo: "**/*.{py,sh,md,json,yaml,yml}"
description: "Semantic search system integration and documentation management for STM32H753ZI stepper motor project"
---

# Semantic Search System Integration Instructions

## Overview
This instruction file provides comprehensive guidance for using and maintaining the semantic search system integrated into the STM32H753ZI stepper motor control project. The system enables AI-powered documentation search across all project assets using real embeddings.

**PROJECT STATUS**: ✅ **PRODUCTION READY** - Semantic search system fully operational with 981 documents indexed using ChromaDB + Ollama embeddings.

## 🔍 Semantic Search Architecture

### 🤖 AI-Powered Documentation Search
**Intelligent search with real embeddings across all project documentation:**

- **AI Understanding**: Uses Ollama mxbai-embed-large embeddings for context-aware search
- **Multi-Collection Database**: ChromaDB with organized document collections
- **Production Scale**: Processes 981 documents from real workspace (27 source files)
- **GPU Acceleration**: CUDA 12.1 support with automatic GPU detection
- **Integration Ready**: Available via scripts and Python API

### 📚 Database Collections
**Organized documentation in semantic vector database (`docs/semantic_vector_db/`):**

```bash
# Collection Structure (981 total documents)
stm32_hal      # 763 documents: STM32H7 HAL, Nucleo BSP, reference manuals
motor_control  # 218 documents: X-CUBE-SPN2 L6470 stepper driver documentation
safety_systems # Future: Safety system specific documentation
project_code   # Future: Source code semantic analysis
instruction_guides # Future: Instruction file semantic search
```

### 🎯 Search Capabilities
**Professional search patterns for development assistance:**

```bash
# PRODUCTION SEMANTIC SEARCH (Preferred Method)

# Recommended: Use wrapper script (auto-handles virtual environment)
./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7  
./scripts/stm32_search.sh function "L6470" --scope L6470
./scripts/stm32_search.sh peripheral "SPI" --scope all
./scripts/stm32_search.sh concept "stepper motor control" --scope all

# Alternative: Direct virtual environment usage
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "GPIO configuration" --scope STM32H7
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py function "HAL_GPIO_Init" --scope STM32H7
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py peripheral "SPI" --scope all

# Rebuild semantic database after documentation updates
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "test" --rebuild
```

## 🛠️ Using the Semantic Search System

### Basic Search Patterns
**Common search patterns for different documentation types:**

```bash
# STM32H7 HAL Functions - Use concept and function searches
./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7
./scripts/stm32_search.sh peripheral "UART" --scope STM32H7

# L6470 Motor Driver - Use function and concept patterns  
./scripts/stm32_search.sh function "L6470" --scope L6470    # ✅ Works well
./scripts/stm32_search.sh concept "stepper driver" --scope L6470
./scripts/stm32_search.sh register "ABS_POS" --scope L6470

# Nucleo BSP Functions - Use exact function names
./scripts/stm32_search.sh function "BSP_LED_Init" --scope NUCLEO_BSP  # ✅ Works well
./scripts/stm32_search.sh concept "board support" --scope NUCLEO_BSP

# Cross-platform concept searches (most effective for complex topics)
./scripts/stm32_search.sh concept "stepper configuration" --scope all
./scripts/stm32_search.sh concept "peripheral setup" --scope all
./scripts/stm32_search.sh concept "safety_systems" --scope all
```

### Advanced Search Techniques
**Optimize search results for different use cases:**

```bash
# Development Tasks
./scripts/stm32_search.sh concept "GPIO pin configuration" --scope STM32H7
./scripts/stm32_search.sh concept "SPI master setup" --scope STM32H7  
./scripts/stm32_search.sh concept "I2C device communication" --scope STM32H7

# Motor Control Tasks
./scripts/stm32_search.sh concept "stepper motor initialization" --scope L6470
./scripts/stm32_search.sh concept "position control algorithms" --scope all
./scripts/stm32_search.sh concept "encoder feedback" --scope all

# Debugging and Troubleshooting
./scripts/stm32_search.sh concept "SPI communication error" --scope all
./scripts/stm32_search.sh concept "motor fault detection" --scope L6470
./scripts/stm32_search.sh concept "error handling patterns" --scope all

# Integration and Architecture
./scripts/stm32_search.sh concept "HAL abstraction" --scope all
./scripts/stm32_search.sh concept "RTOS integration" --scope all
./scripts/stm32_search.sh concept "safety system design" --scope all
```

### Scope Selection Guidelines
**Choose appropriate search scope for optimal results:**

```python
# Scope Options and Use Cases:
--scope STM32H7      # STM32H7 HAL functions, peripherals, reference manual
--scope L6470        # L6470 stepper driver, X-CUBE-SPN2 documentation  
--scope NUCLEO_BSP   # Nucleo-144 board support package functions
--scope all          # Cross-platform search across all collections

# Case-insensitive scope matching:
--scope stm32h7      # Same as STM32H7
--scope nucleo_bsp   # Same as NUCLEO_BSP
--scope l6470        # Same as L6470
```

## 🔧 System Maintenance and Administration

### Database Management
**Maintain and update the semantic search database:**

```bash
# Check database status and statistics
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "test" --scope all

# Rebuild database after documentation updates
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "test" --rebuild

# Add new documentation collection (for future expansion)
# Edit scripts/stm32_semantic_search.py to add new collections
# Update docs/semantic_vector_db/ with new content
```

### Performance Optimization
**Monitor and optimize search performance:**

```python
# GPU Acceleration Check
import torch
print(f"CUDA Available: {torch.cuda.is_available()}")
print(f"CUDA Version: {torch.version.cuda}")

# ChromaDB Performance Monitoring
collection = client.get_collection("stm32_hal")
print(f"Collection size: {collection.count()} documents")

# Search Response Time Optimization
import time
start_time = time.time()
results = collection.query(query_texts=["GPIO configuration"], n_results=5)
end_time = time.time()
print(f"Search time: {end_time - start_time:.3f} seconds")
```

### Database Backup and Recovery
**Protect semantic search data:**

```bash
# Backup semantic database
tar -czf semantic_db_backup_$(date +%Y%m%d).tar.gz docs/semantic_vector_db/

# Restore from backup
tar -xzf semantic_db_backup_YYYYMMDD.tar.gz

# Verify database integrity after restore
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "test" --scope all
```

## 🚀 Integration with Development Workflow

### VS Code Integration
**Integrate semantic search with development environment:**

```json
// Add to .vscode/tasks.json
{
    "label": "Search Documentation (Semantic)",
    "type": "shell", 
    "command": "./scripts/stm32_search.sh",
    "args": [
        "concept",
        "${input:searchQuery}",
        "--scope",
        "${input:searchScope}"
    ],
    "group": "test",
    "options": {
        "cwd": "${workspaceFolder}"
    },
    "inputs": [
        {
            "id": "searchQuery",
            "description": "Enter search query",
            "default": "GPIO configuration",
            "type": "promptString"
        },
        {
            "id": "searchScope", 
            "description": "Choose search scope",
            "default": "all",
            "type": "pickString",
            "options": ["STM32H7", "L6470", "NUCLEO_BSP", "all"]
        }
    ]
}
```

### GitHub Copilot Enhancement
**Leverage semantic search for better AI assistance:**

```bash
# Use semantic search to provide context for Copilot
# Example workflow:
1. ./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
2. Copy relevant results to provide context in Copilot chat
3. Ask Copilot to implement based on found documentation

# Common patterns for Copilot enhancement:
./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7  # Get function details
./scripts/stm32_search.sh concept "SPI master setup" --scope STM32H7  # Get setup procedures
./scripts/stm32_search.sh concept "motor control loop" --scope all    # Get algorithm patterns
```

### Automated Documentation Updates
**Keep semantic search current with project changes:**

```bash
#!/bin/bash
# update_semantic_search.sh - Run after documentation changes

# Check if documentation has changed
if git diff --name-only HEAD~1 HEAD | grep -E "\.(md|txt|rst)$"; then
    echo "Documentation changes detected, updating semantic search..."
    
    # Rebuild semantic database
    /workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "test" --rebuild
    
    # Verify update
    if [ $? -eq 0 ]; then
        echo "Semantic search database updated successfully"
    else
        echo "ERROR: Semantic search database update failed"
        exit 1
    fi
fi
```

## 📊 System Architecture Details

### Technical Implementation
**Understanding the semantic search infrastructure:**

```python
# Core Components:
ChromaDB         # Vector database for document storage and retrieval
Ollama           # Local LLM inference for embedding generation  
mxbai-embed-large # Embedding model (1024-dimensional vectors)
CUDA 12.1        # GPU acceleration for embedding computation

# Performance Specifications:
Database Size: 981 documents (763 STM32H7 + 218 L6470)
Vector Dimensions: 1024 (mxbai-embed-large)
Search Response: <2 seconds for typical queries
Memory Usage: ~500MB for full database
GPU Memory: ~2GB during embedding computation
```

### Collection Organization Strategy
**How documents are organized for optimal search:**

```python
# Collection Strategy:
stm32_hal:      # Primary STM32H7 documentation
  - STM32H7 HAL drivers and reference manual
  - Nucleo-144 BSP documentation  
  - Core peripheral documentation
  - Memory management and optimization guides

motor_control:  # Motor-specific documentation
  - X-CUBE-SPN2 L6470 stepper driver docs
  - Motor control algorithms and examples
  - Position feedback and encoder integration
  - Safety systems for motor control

# Future Collections (Expansion Ready):
safety_systems: # Safety-critical system documentation
project_code:   # Source code semantic analysis
instruction_guides: # Development instruction files
```

### Error Handling and Resilience
**Robust operation with graceful degradation:**

```python
# Error Handling Patterns:
try:
    # Primary search using semantic embeddings
    results = semantic_search(query, scope)
except OllamaConnectionError:
    # Fallback to keyword search if Ollama unavailable
    results = fallback_keyword_search(query, scope)
    logging.warning("Using fallback search - Ollama unavailable")
except ChromaDBError:
    # Database corruption or access issues
    logging.error("Semantic database error - rebuild required")
    return error_message_with_rebuild_instructions()

# Graceful degradation ensures system remains usable
# even if semantic features are temporarily unavailable
```

## 🔮 Future Enhancements

### Planned System Expansions
**Roadmap for semantic search system evolution:**

```bash
# Phase 5A Integration:
# - Add FreeRTOS documentation collection
# - Integrate RTOS-specific search patterns
# - Task and synchronization primitive search

# Advanced Features:
# - Source code semantic analysis
# - Instruction file semantic search  
# - Interactive query refinement
# - Context-aware result ranking

# Integration Enhancements:
# - VS Code extension for inline search
# - GitHub Copilot context automation
# - CI/CD documentation validation
# - Automated knowledge base updates
```

### Performance Optimization Roadmap
**Continuous improvement of search capabilities:**

```python
# Optimization Targets:
Search Speed: Target <1 second response time
Result Quality: Improve relevance scoring
Memory Efficiency: Reduce database memory footprint
GPU Utilization: Optimize embedding computation

# Advanced Features:
Contextual Search: Use conversation history for better results  
Multi-Modal Search: Include code snippets and diagrams
Real-Time Updates: Continuous database updates
Federated Search: Search across multiple project repositories
```

## 💡 Best Practices

### Search Query Optimization
**Write effective queries for best results:**

```bash
# Effective Query Patterns:
✅ ./scripts/stm32_search.sh concept "GPIO pin configuration"     # Specific and descriptive
✅ ./scripts/stm32_search.sh function "HAL_SPI_Transmit"         # Exact function names
✅ ./scripts/stm32_search.sh peripheral "I2C master mode"       # Include context

# Avoid These Patterns:
❌ ./scripts/stm32_search.sh concept "GPIO"                     # Too generic
❌ ./scripts/stm32_search.sh function "init"                    # Too common
❌ ./scripts/stm32_search.sh concept "motor"                    # Lacks specificity
```

### Scope Selection Strategy
**Choose optimal scope for different tasks:**

```bash
# Development Phase - Use specific scopes:
./scripts/stm32_search.sh concept "peripheral setup" --scope STM32H7
./scripts/stm32_search.sh function "L6470_Init" --scope L6470

# Integration Phase - Use broader scopes:
./scripts/stm32_search.sh concept "motor control integration" --scope all
./scripts/stm32_search.sh concept "safety system design" --scope all

# Debugging Phase - Start specific, expand if needed:
./scripts/stm32_search.sh concept "SPI timeout error" --scope STM32H7
# If no results: ./scripts/stm32_search.sh concept "SPI timeout" --scope all
```

### Result Interpretation
**Make the most of search results:**

```python
# Understanding Search Results:
Relevance Score: Higher scores indicate better semantic matches
Document Source: Check if result is from HAL, driver, or BSP docs
Context Snippet: Review surrounding text for implementation details
Cross-Reference: Use multiple searches to build complete understanding

# Workflow for Complex Topics:
1. Start with concept search for overview
2. Use function search for specific implementations  
3. Use peripheral search for hardware configuration
4. Cross-reference results across different scopes
```

Remember: **Semantic search transforms documentation into an AI-powered development assistant. Use it early and often for better development outcomes.**

```
