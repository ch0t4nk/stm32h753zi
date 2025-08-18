# Semantic Search API Documentation

## Overview

The STM32H753ZI Semantic Search API provides high-performance AI-powered search capabilities across project documentation, source code, and reference materials. The system uses a lightweight client-server architecture with ChromaDB and Instructor-XL embeddings for intelligent document retrieval.

**Last Updated**: August 11, 2025  
**Version**: 2.0 (Lightweight Client Architecture)  
**Service Status**: ✅ **PRODUCTION** - Complete semantic search infrastructure

## Architecture

### Client-Server Design

```
┌─────────────────┐    HTTP/REST     ┌─────────────────────┐
│  Lightweight    │ ◄──────────────► │ Background Service  │
│  Search Client  │    Port 8080     │ (semantic_search_   │
│                 │                  │  service.py)        │
└─────────────────┘                  └─────────────────────┘
                                               │
                                               ▼
                                     ┌─────────────────────┐
                                     │ ChromaDB Vector DB  │
                                     │ 252K+ Documents     │
                                     │ 5 Collections       │
                                     │ 1024-dim Embeddings │
                                     └─────────────────────┘
```

### Performance Characteristics

- **Query Response Time**: ~2-20s (depending on scope)
- **Cache Hit Response**: 0.0ms (instant)
- **Document Coverage**: 252,416 documents across 5 collections
- **Embedding Model**: mxbai-embed-large (1024 dimensions)
- **Memory Usage**: ~101MB for full database
- **Service Management**: Daemon lifecycle with health monitoring

## REST API Endpoints

### Base URL
```
http://localhost:8080
```

### 1. Search Query

**Endpoint**: `POST /query`

**Description**: Performs semantic search across one or more document collections.

**Request Body**:
```json
{
    "query": "GPIO configuration",
    "search_type": "concept",
    "scope": "STM32H7",
    "max_results": 5
}
```

**Parameters**:
- `query` (string, required): Search query text
- `search_type` (string, required): Type of search
  - `"concept"`: Semantic concept search (most flexible)
  - `"function"`: Function/API specific search
  - `"peripheral"`: Hardware peripheral search
  - `"register"`: Register/configuration search
- `scope` (string, required): Search scope
  - `"STM32H7"`: STM32H7 HAL documentation
  - `"L6470"` or `"motor_control"`: L6470 stepper driver docs
  - `"NUCLEO_BSP"`: Nucleo board support package
  - `"project_code"`: Project source code
  - `"all"`: All collections
- `max_results` (integer, optional): Maximum results to return (default: 5)

**Response**:
```json
{
    "results": [
        {
            "content": "GPIO configuration example content...",
            "source_file": "stm32h7xx_hal_gpio.h",
            "similarity_score": 0.89,
            "collection": "stm32_hal",
            "metadata": {
                "file_path": "/path/to/file",
                "section": "GPIO_Init",
                "category": "hal_function"
            }
        }
    ],
    "query_time_ms": 2234.5,
    "total_results": 5,
    "cached": false,
    "collections_searched": ["stm32_hal"]
}
```

### 2. Service Health

**Endpoint**: `GET /health`

**Description**: Returns service health and status information.

**Response**:
```json
{
    "status": "healthy",
    "service": "STM32 Semantic Search Service",
    "version": "1.0",
    "uptime_seconds": 3600,
    "memory_usage_mb": 101.2,
    "collections": {
        "stm32_hal": {
            "documents": 55884,
            "status": "ready"
        },
        "motor_control": {
            "documents": 1311,
            "status": "ready"
        },
        "project_code": {
            "documents": 187,
            "status": "ready"
        },
        "instruction_guides": {
            "documents": 1643,
            "status": "ready"
        },
        "safety_systems": {
            "documents": 0,
            "status": "empty"
        }
    },
    "embedding_model": "mxbai-embed-large",
    "embedding_dimensions": 1024
}
```

### 3. Service Status

**Endpoint**: `GET /status`

**Description**: Returns detailed service status and collection information.

**Response**:
```json
{
    "service_status": "running",
    "collections_loaded": 5,
    "total_documents": 252416,
    "cache_size": 150,
    "cache_hit_rate": 0.85,
    "last_query_time": "2025-08-11T23:45:30Z",
    "collections": {
        "stm32_hal": {
            "name": "STM32H7 HAL Documentation",
            "documents": 55884,
            "size_mb": 45.2,
            "last_updated": "2025-08-11T20:30:00Z"
        },
        "motor_control": {
            "name": "L6470 Motor Control Documentation", 
            "documents": 1311,
            "size_mb": 1.8,
            "last_updated": "2025-08-11T20:30:00Z"
        }
    }
}
```

## Client Library Usage

### Python Client

The lightweight Python client (`stm32_semantic_search.py`) provides a simple interface:

```python
from scripts.stm32_semantic_search import SemanticSearchClient

# Initialize client
client = SemanticSearchClient()

# Perform searches
results = client.search(
    query="GPIO configuration",
    search_type="concept", 
    scope="STM32H7",
    max_results=10
)

# Process results
for result in results:
    print(f"File: {result.source_file}")
    print(f"Score: {result.similarity_score:.2f}")
    print(f"Content: {result.content[:200]}...")
```

### Command Line Interface

```bash
# Search STM32H7 documentation
python scripts/stm32_semantic_search.py concept "GPIO configuration" --scope STM32H7

# Search L6470 motor control
python scripts/stm32_semantic_search.py function "L6470" --scope motor_control

# Search all collections
python scripts/stm32_semantic_search.py concept "safety systems" --scope all

# Wrapper script (handles virtual environment)
./scripts/stm32_search.sh concept "peripheral setup" --scope all
```

## Document Collections

### Collection Overview

| Collection | Documents | Size | Description |
|-----------|-----------|------|-------------|
| `stm32_hal` | 55,884 | 241MB | STM32H7 HAL drivers, peripherals, reference manuals |
| `motor_control` | 1,311 | 9MB | X-CUBE-SPN2 L6470 stepper driver documentation |
| `project_code` | 187 | 1MB | Project source code, headers, implementation files |
| `instruction_guides` | 1,643 | 18MB | Development instruction files and guidelines |
| `safety_systems` | 0 | 0MB | Safety system documentation (placeholder) |

### Scope Mapping

- **STM32H7**: `stm32_hal` collection
- **L6470**, **motor_control**: `motor_control` collection  
- **NUCLEO_BSP**: Subset of `stm32_hal` (Nucleo-specific)
- **project_code**: `project_code` collection
- **all**: All available collections

## Service Management

### Starting the Service

```bash
# Start semantic search service as daemon
./scripts/start_semantic_service.sh

# Check service status
curl http://localhost:8080/health
```

### Stopping the Service

```bash
# Stop semantic search service gracefully
./scripts/stop_semantic_service.sh
```

### Service Logs

Service logs are available at:
- **Service Log**: `logs/semantic_search_service.log`
- **Error Log**: `logs/semantic_search_error.log`
- **PID File**: `logs/semantic_search_service.pid`

## Performance Optimization

### Caching Strategy

The service implements intelligent caching:

- **Query Hashing**: Identical queries return cached results (0.0ms)
- **Collection Pre-warming**: All collections loaded at startup
- **Memory Efficiency**: Optimized document chunking and storage
- **LRU Cache**: Automatic cache management with configurable size

### Search Performance Tips

1. **Use Specific Scopes**: Target specific collections for faster results
2. **Optimize Query Length**: 3-10 word queries work best
3. **Leverage Caching**: Repeated queries return instantly
4. **Function Searches**: Use `function` type for API/function searches
5. **Concept Searches**: Use `concept` type for broader topic searches

## Error Handling

### Common HTTP Status Codes

- **200 OK**: Successful search
- **400 Bad Request**: Invalid query parameters
- **404 Not Found**: Endpoint not found
- **500 Internal Server Error**: Service error
- **503 Service Unavailable**: Service not ready/database not loaded

### Error Response Format

```json
{
    "error": "Invalid search scope",
    "message": "Scope 'invalid_scope' not supported. Valid scopes: STM32H7, L6470, motor_control, NUCLEO_BSP, project_code, all",
    "status_code": 400,
    "timestamp": "2025-08-11T23:45:30Z"
}
```

## Migration from Legacy System

### Architectural Changes

**Previous (Monolithic)**:
- 632-line script with embedded database logic
- Heavy ChromaDB initialization on every search
- 3-5 second startup time per query

**Current (Client-Server)**:
- ~200-line lightweight HTTP client
- Background service with persistent database
- Instant queries via HTTP requests
- 0.0ms response time for cached queries

### Migration Benefits

1. **Performance**: 10-100x faster query response
2. **Resource Efficiency**: Shared database across sessions
3. **Scalability**: Multiple clients can use same service
4. **Reliability**: Service monitoring and automatic restart
5. **Development Efficiency**: No database setup per query

## API Examples

### Basic Concept Search

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: application/json" \
  -d '{
    "query": "GPIO configuration examples",
    "search_type": "concept",
    "scope": "STM32H7",
    "max_results": 3
  }'
```

### Function-Specific Search

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: application/json" \
  -d '{
    "query": "HAL_GPIO_Init",
    "search_type": "function", 
    "scope": "STM32H7",
    "max_results": 5
  }'
```

### Cross-Collection Search

```bash
curl -X POST http://localhost:8080/query \
  -H "Content-Type: application/json" \
  -d '{
    "query": "motor control safety systems",
    "search_type": "concept",
    "scope": "all",
    "max_results": 10
  }'
```

## Integration Notes

### With GitHub Copilot

The semantic search system is optimized for GitHub Copilot integration:

- **Context-Aware**: Results include relevant code snippets and documentation
- **Token-Efficient**: Optimized result formatting for LLM consumption  
- **Development-Focused**: Prioritizes actionable code examples and API references
- **Real-Time**: Fast enough for interactive development workflows

### With VS Code Tasks

Semantic search is integrated into VS Code development tasks:

```json
{
    "label": "🔍 SEMANTIC: Search STM32 Docs",
    "type": "shell",
    "command": "${workspaceFolder}/.venv/bin/python",
    "args": [
        "scripts/stm32_semantic_search.py",
        "concept",
        "${input:searchQuery}",
        "--scope",
        "${input:searchScope}"
    ]
}
```

## Future Enhancements

### Planned Features

1. **Real-Time Indexing**: Automatic document updates when source files change
2. **Multi-Language Support**: Extended language model support
3. **Advanced Filtering**: Date ranges, file types, content categories
4. **Analytics Dashboard**: Query analytics and usage metrics
5. **Federated Search**: Integration with external documentation sources

### API Evolution

The API is designed for backward compatibility with planned enhancements:

- **Version Headers**: Future API versioning support
- **Extended Metadata**: Richer result metadata and categorization
- **Batch Queries**: Multiple query processing in single request
- **Streaming Results**: Real-time result streaming for large queries

---

*For technical support or API questions, refer to the project documentation or development team.*
