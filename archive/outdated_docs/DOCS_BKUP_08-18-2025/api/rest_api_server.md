# REST API Server Documentation

## Overview

The STM32H753ZI Semantic Search REST API Server provides high-performance HTTP endpoints for AI-powered document search across the project ecosystem. Built with `aiohttp` for async performance and ChromaDB for vector database operations.

**Service File**: `scripts/semantic_search_service.py`  
**Port**: 8080  
**Protocol**: HTTP/1.1  
**Architecture**: Asynchronous Python with ChromaDB backend

## Server Architecture

### Core Components

```
┌─────────────────────────────────────┐
│         aiohttp Web Server          │
│              Port 8080              │
├─────────────────────────────────────┤
│  HTTP Request Router & Handlers     │
│  ┌─────────────┬─────────────────┐  │
│  │ /query      │ POST - Search   │  │
│  │ /health     │ GET - Health    │  │
│  │ /status     │ GET - Status    │  │
│  └─────────────┴─────────────────┘  │
├─────────────────────────────────────┤
│       EmbeddingGenerator            │
│  ┌─────────────────────────────────┐ │
│  │ Ollama Integration              │ │
│  │ Model: mxbai-embed-large        │ │
│  │ Dimensions: 1024                │ │
│  │ Batch Processing Support        │ │
│  └─────────────────────────────────┘ │
├─────────────────────────────────────┤
│          ChromaDB Client            │
│  ┌─────────────────────────────────┐ │
│  │ Vector Database Operations      │ │
│  │ 5 Collections Managed          │ │
│  │ 252K+ Documents Indexed         │ │
│  │ Similarity Search Engine        │ │
│  └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### Service Features

- **Async Architecture**: Non-blocking HTTP request handling
- **Connection Management**: Persistent ChromaDB connections
- **Embedding Generation**: On-demand Ollama model integration
- **Health Monitoring**: Comprehensive service health checks
- **Error Handling**: Graceful error responses with proper HTTP codes
- **CORS Support**: Cross-origin request handling
- **Request Validation**: Input parameter validation and sanitization

## HTTP Endpoints

### 1. Search Query Endpoint

**Route**: `POST /query`  
**Handler**: `handle_query`  
**Timeout**: 30 seconds

#### Request Processing Flow

1. **Input Validation**: Validate required parameters
2. **Scope Resolution**: Map scope to ChromaDB collections
3. **Embedding Generation**: Generate query embeddings via Ollama
4. **Vector Search**: Perform similarity search in ChromaDB
5. **Result Formatting**: Format and return search results

#### Request Schema

```json
{
    "query": "string (required)",
    "search_type": "concept|function|peripheral|register (required)",
    "scope": "STM32H7|L6470|motor_control|NUCLEO_BSP|project_code|all (required)",
    "max_results": "integer (optional, default: 5, max: 50)"
}
```

#### Response Schema

```json
{
    "results": [
        {
            "content": "string",
            "source_file": "string", 
            "similarity_score": "float (0.0-1.0)",
            "collection": "string",
            "metadata": {
                "file_path": "string",
                "section": "string",
                "category": "string"
            }
        }
    ],
    "query_time_ms": "float",
    "total_results": "integer",
    "cached": "boolean",
    "collections_searched": ["string"]
}
```

#### Error Responses

```json
{
    "error": "string",
    "message": "string", 
    "status_code": "integer",
    "timestamp": "ISO8601 string"
}
```

### 2. Health Check Endpoint

**Route**: `GET /health`  
**Handler**: `handle_health`  
**Timeout**: 5 seconds

#### Health Check Components

- **Service Status**: Overall service health
- **Database Connectivity**: ChromaDB connection status
- **Collection Availability**: Per-collection document counts
- **Memory Usage**: Current memory consumption
- **Uptime Tracking**: Service uptime in seconds
- **Model Status**: Ollama embedding model availability

#### Response Example

```json
{
    "status": "healthy",
    "service": "STM32 Semantic Search Service",
    "version": "1.0",
    "uptime_seconds": 7200,
    "memory_usage_mb": 101.2,
    "collections": {
        "stm32_hal": {
            "documents": 55884,
            "status": "ready",
            "size_mb": 241.5
        },
        "motor_control": {
            "documents": 1311,
            "status": "ready", 
            "size_mb": 9.2
        }
    },
    "embedding_model": "mxbai-embed-large",
    "embedding_dimensions": 1024,
    "ollama_status": "connected"
}
```

### 3. Status Information Endpoint

**Route**: `GET /status`  
**Handler**: `handle_status`  
**Timeout**: 5 seconds

#### Detailed Status Metrics

- **Operational Metrics**: Request counts, response times
- **Cache Performance**: Hit rates, cache size
- **Collection Statistics**: Document counts, index sizes
- **Resource Usage**: Memory, CPU utilization
- **Error Tracking**: Recent error counts and types

## Service Implementation Details

### EmbeddingGenerator Class

**Purpose**: Manages Ollama integration for text embedding generation

#### Key Methods

```python
class EmbeddingGenerator:
    def __init__(self, model_name: str = "mxbai-embed-large"):
        """Initialize with specified Ollama model"""
        
    async def generate_embedding(self, text: str) -> List[float]:
        """Generate 1024-dimensional embedding for text"""
        
    async def generate_batch_embeddings(self, texts: List[str]) -> List[List[float]]:
        """Generate embeddings for multiple texts efficiently"""
        
    def is_available(self) -> bool:
        """Check if Ollama service is available"""
```

#### Configuration

- **Model**: `mxbai-embed-large` (1024 dimensions)
- **Ollama URL**: `http://localhost:11434`
- **Timeout**: 30 seconds per embedding request
- **Batch Size**: Up to 10 texts per batch request
- **Retry Logic**: 3 attempts with exponential backoff

### ChromaDB Integration

**Database Path**: `/workspaces/code/docs/semantic_vector_db`  
**Collections**: 5 active collections with 252K+ total documents

#### Collection Management

```python
COLLECTION_CONFIG = {
    "stm32_hal": {
        "name": "STM32H7 HAL Documentation",
        "documents": 55884,
        "description": "STM32H7 HAL drivers, peripherals, reference manuals"
    },
    "motor_control": {
        "name": "L6470 Motor Control Documentation",
        "documents": 1311, 
        "description": "X-CUBE-SPN2 L6470 stepper driver documentation"
    },
    "project_code": {
        "name": "Project Source Code",
        "documents": 187,
        "description": "Project source code, headers, implementation files"
    },
    "instruction_guides": {
        "name": "Development Instruction Files",
        "documents": 1643,
        "description": "Development instruction files and guidelines"
    },
    "safety_systems": {
        "name": "Safety System Documentation", 
        "documents": 0,
        "description": "Safety system documentation (placeholder)"
    }
}
```

### Request/Response Handlers

#### Query Handler (`handle_query`)

```python
async def handle_query(request):
    """
    Process semantic search requests
    
    1. Validate input parameters
    2. Map scope to collections
    3. Generate query embeddings
    4. Perform vector similarity search
    5. Format and return results
    """
```

**Input Validation**:
- Required fields: `query`, `search_type`, `scope`
- Optional fields: `max_results` (default: 5, max: 50)
- Scope validation: Must be valid collection scope
- Query length: Maximum 1000 characters

**Processing Pipeline**:
1. Extract and validate JSON request body
2. Generate embedding for search query
3. Map scope to ChromaDB collection names
4. Execute similarity search with specified parameters
5. Format results with metadata and similarity scores
6. Return JSON response with performance metrics

#### Health Handler (`handle_health`)

```python
async def handle_health(request):
    """
    Return comprehensive service health information
    
    1. Check ChromaDB connectivity
    2. Validate collection availability
    3. Test Ollama model accessibility
    4. Calculate memory usage and uptime
    5. Return detailed health status
    """
```

#### Status Handler (`handle_status`)

```python  
async def handle_status(request):
    """
    Return detailed operational status and metrics
    
    1. Gather collection statistics
    2. Calculate performance metrics
    3. Report cache efficiency
    4. Return comprehensive status
    """
```

## Service Lifecycle Management

### Startup Process

1. **Initialize ChromaDB Client**: Connect to vector database
2. **Load Collections**: Verify all collections are accessible
3. **Initialize Ollama Client**: Connect to embedding service
4. **Start HTTP Server**: Bind to port 8080 and begin serving
5. **Health Check**: Perform initial service validation

### Shutdown Process

1. **Graceful Shutdown**: Stop accepting new requests
2. **Complete Active Requests**: Allow in-flight requests to finish
3. **Close Database Connections**: Cleanly disconnect from ChromaDB
4. **Release Resources**: Clean up memory and file handles

### Error Recovery

- **Database Reconnection**: Automatic retry on ChromaDB connection loss
- **Ollama Fallback**: Error handling for embedding generation failures
- **Request Timeouts**: 30-second timeout for all search operations
- **Memory Management**: Automatic garbage collection and resource cleanup

## Performance Characteristics

### Response Time Benchmarks

| Operation | Typical Time | Maximum Time | Cache Hit |
|-----------|--------------|--------------|-----------|
| Simple Query (STM32H7) | 2-5 seconds | 15 seconds | 0.0ms |
| Complex Query (all) | 5-20 seconds | 30 seconds | 0.0ms |
| Health Check | 50-200ms | 5 seconds | N/A |
| Status Check | 100-500ms | 5 seconds | N/A |

### Memory Usage

- **Base Service**: ~50MB
- **ChromaDB Cache**: ~50-100MB
- **Peak Usage**: ~200MB during complex queries
- **Ollama Overhead**: ~10-20MB per active embedding request

### Throughput Characteristics

- **Concurrent Requests**: Up to 10 simultaneous queries
- **Queue Management**: Request queuing for overload protection
- **Rate Limiting**: Built-in protection against excessive requests
- **Connection Pooling**: Efficient database connection reuse

## Configuration Options

### Environment Variables

```bash
# Service Configuration
SEMANTIC_SEARCH_PORT=8080
SEMANTIC_SEARCH_HOST=localhost
SEMANTIC_SEARCH_TIMEOUT=30

# ChromaDB Configuration  
CHROMADB_PATH=/workspaces/code/docs/semantic_vector_db
CHROMADB_COLLECTION_LIMIT=5

# Ollama Configuration
OLLAMA_URL=http://localhost:11434
OLLAMA_MODEL=mxbai-embed-large
OLLAMA_TIMEOUT=30

# Logging Configuration
LOG_LEVEL=INFO
LOG_FILE=logs/semantic_search_service.log
ERROR_LOG_FILE=logs/semantic_search_error.log
```

### Service Tuning Parameters

```python
# aiohttp Server Configuration
SERVER_CONFIG = {
    "host": "localhost",
    "port": 8080,
    "keepalive_timeout": 30,
    "client_timeout": 30,
    "client_max_size": 1024 * 1024  # 1MB max request size
}

# ChromaDB Query Configuration
QUERY_CONFIG = {
    "max_results_per_collection": 50,
    "similarity_threshold": 0.1,
    "include_metadata": True,
    "include_documents": True
}

# Embedding Configuration
EMBEDDING_CONFIG = {
    "model_name": "mxbai-embed-large",
    "dimensions": 1024,
    "batch_size": 10,
    "timeout_seconds": 30
}
```

## Monitoring and Debugging

### Logging Structure

**Service Log (`logs/semantic_search_service.log`)**:
```
2025-08-11 23:45:30 [INFO] Service starting on localhost:8080
2025-08-11 23:45:31 [INFO] ChromaDB connected: 5 collections loaded
2025-08-11 23:45:31 [INFO] Ollama model mxbai-embed-large ready
2025-08-11 23:45:32 [INFO] Service ready for requests
2025-08-11 23:46:15 [INFO] Query: "GPIO configuration" -> 5 results (2.3s)
```

**Error Log (`logs/semantic_search_error.log`)**:
```
2025-08-11 23:47:22 [ERROR] ChromaDB query failed: Collection not found
2025-08-11 23:47:22 [ERROR] Request failed: 500 Internal Server Error
2025-08-11 23:48:10 [WARN] Ollama embedding timeout, retrying...
```

### Health Monitoring

```bash
# Check service health
curl http://localhost:8080/health

# Monitor service logs
tail -f logs/semantic_search_service.log

# Check service process
ps aux | grep semantic_search_service
```

### Performance Monitoring

```bash
# Monitor service metrics
curl http://localhost:8080/status | jq '.collections'

# Check memory usage
curl http://localhost:8080/health | jq '.memory_usage_mb'

# Monitor response times
curl -w "@curl-format.txt" -s -o /dev/null http://localhost:8080/health
```

## Security Considerations

### Current Security Posture

- **Local Network Only**: Service bound to localhost (127.0.0.1)
- **No Authentication**: Designed for local development use
- **Input Validation**: Basic parameter validation and sanitization
- **Resource Limits**: Request size and timeout protections
- **Error Handling**: No sensitive information in error messages

### Recommended Security Enhancements

1. **Authentication**: API key or token-based authentication
2. **Rate Limiting**: Per-client request rate limiting
3. **Input Sanitization**: Enhanced XSS and injection protection
4. **HTTPS Support**: TLS encryption for production deployment
5. **Access Logging**: Comprehensive request and access logging
6. **Resource Quotas**: Per-client resource usage limits

## Troubleshooting Guide

### Common Issues

**Issue**: Service won't start on port 8080  
**Solution**: Check for existing process `lsof -i :8080` and kill if necessary

**Issue**: ChromaDB connection errors  
**Solution**: Verify database path exists and permissions are correct

**Issue**: Ollama embedding failures  
**Solution**: Ensure Ollama service is running with `./scripts/start_ollama.sh`

**Issue**: Slow query responses  
**Solution**: Check system resources and consider restarting service

**Issue**: Memory usage grows over time  
**Solution**: Restart service to clear caches: `./scripts/stop_semantic_service.sh && ./scripts/start_semantic_service.sh`

### Debug Commands

```bash
# Test service connectivity
curl -f http://localhost:8080/health || echo "Service not responding"

# Test basic query
curl -X POST http://localhost:8080/query \
  -H "Content-Type: application/json" \
  -d '{"query":"test","search_type":"concept","scope":"STM32H7"}'

# Monitor service logs in real-time
tail -f logs/semantic_search_service.log logs/semantic_search_error.log

# Check process status
pgrep -f semantic_search_service || echo "Service not running"
```

---

*This documentation covers the REST API server implementation for the STM32H753ZI Semantic Search system. For client usage and integration examples, see `semantic_search_api.md`.*
