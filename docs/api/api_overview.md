# API Overview

## STM32H753ZI Motor Control & Semantic Search API Suite

This document provides a comprehensive overview of the API ecosystem for the STM32H753ZI stepper motor control project with integrated semantic search capabilities.

**Last Updated**: August 11, 2025  
**API Version**: 2.0  
**Status**: ✅ **PRODUCTION-READY** (Semantic Search) | 🔄 **PLANNED** (Motor Control)

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                   API Gateway Layer                        │
│                   (Future Integration)                     │
└─────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              ▼               ▼               ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│ Semantic Search │ │ Motor Control   │ │ System Monitor  │
│ REST API        │ │ REST API        │ │ REST API        │
│ Port: 8080      │ │ Port: 3000      │ │ Port: 4000      │
│ ✅ PRODUCTION   │ │ 🔄 PLANNED      │ │ 🔄 PLANNED      │
└─────────────────┘ └─────────────────┘ └─────────────────┘
          │                   │                   │
          ▼                   ▼                   ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│ ChromaDB +      │ │ STM32H753ZI     │ │ System Health   │
│ Ollama Embedds  │ │ Hardware Layer  │ │ Metrics Layer   │
│ 252K+ Documents │ │ L6470 + AS5600  │ │ Performance Mon │
└─────────────────┘ └─────────────────┘ └─────────────────┘
```

## API Services

### 1. Semantic Search API ✅ PRODUCTION

**Service**: `scripts/semantic_search_service.py`  
**Port**: 8080  
**Status**: Fully operational with 252K+ indexed documents

#### Key Features
- **AI-Powered Search**: Instructor-XL embeddings with ChromaDB
- **Multi-Collection Support**: STM32H7, L6470, project code, documentation
- **High Performance**: 0.0ms cached responses, 2-20s semantic queries
- **RESTful Design**: Standard HTTP/JSON interface with proper error handling
- **Health Monitoring**: Comprehensive service health and status endpoints

#### Core Endpoints
- `POST /query` - Execute semantic search with AI embeddings
- `GET /health` - Service health check with collection status
- `GET /status` - Detailed operational metrics and statistics

#### Example Usage
```bash
# Search STM32H7 documentation
curl -X POST http://localhost:8080/query \
  -H "Content-Type: application/json" \
  -d '{
    "query": "GPIO configuration",
    "search_type": "concept",
    "scope": "STM32H7",
    "max_results": 5
  }'
```

### 2. Motor Control API 🔄 PLANNED

**Future Service**: Motor control REST interface  
**Port**: 3000 (planned)  
**Status**: Architecture defined, implementation pending

#### Planned Features
- **Motor Command Interface**: Position, speed, acceleration control
- **Real-Time Status**: Live motor state and encoder feedback
- **Safety Integration**: Emergency stop and fault monitoring
- **Motion Profiling**: Advanced trajectory planning and execution
- **Dual Motor Coordination**: Synchronized multi-motor operations

#### Planned Endpoints
- `GET /motors` - Motor status and position information
- `POST /motors/{id}/move` - Motor movement commands
- `POST /motors/emergency-stop` - Immediate safety stop
- `GET /motors/{id}/calibrate` - Motor calibration procedures

### 3. System Monitoring API 🔄 PLANNED

**Future Service**: System health and performance monitoring  
**Port**: 4000 (planned)  
**Status**: Interface design phase

#### Planned Features
- **Performance Metrics**: CPU, memory, temperature monitoring
- **Error Tracking**: Comprehensive error logging and analysis
- **Health Dashboard**: Real-time system health visualization
- **Diagnostic Tools**: System diagnostic and troubleshooting utilities
- **Integration Monitoring**: Cross-service health and communication status

## API Documentation Structure

### Complete Documentation Set

| Document | Purpose | Status |
|----------|---------|---------|
| **API Overview** (this document) | Comprehensive API ecosystem overview | ✅ Complete |
| **Semantic Search API** | Detailed semantic search API reference | ✅ Complete |
| **REST API Server** | Server implementation and configuration | ✅ Complete |
| **OpenAPI Specification** | Machine-readable API specification | ✅ Updated |
| **CAN API** (legacy) | CAN protocol specification | 📋 Archived |

### Documentation Locations

```
docs/api/
├── api_overview.md          # This document - comprehensive overview
├── semantic_search_api.md   # Complete semantic search API reference
├── rest_api_server.md       # Server implementation documentation
└── (motor_control_api.md)   # Future: Motor control API reference

api/
├── openapi.yaml            # Updated OpenAPI 3.1 specification
└── can_api.yaml            # Legacy CAN protocol (archived)
```

## Integration Patterns

### Client Integration Examples

#### Python Client (Semantic Search)
```python
from scripts.stm32_semantic_search import SemanticSearchClient

client = SemanticSearchClient()
results = client.search(
    query="GPIO configuration examples",
    search_type="concept",
    scope="STM32H7"
)

for result in results:
    print(f"Source: {result.source_file}")
    print(f"Score: {result.similarity_score:.2f}")
    print(f"Content: {result.content[:200]}...")
```

#### Command Line Interface
```bash
# Direct semantic search
python scripts/stm32_semantic_search.py concept "motor control" --scope all

# Wrapper script with environment handling
./scripts/stm32_search.sh function "L6470" --scope motor_control

# Service management
./scripts/start_semantic_service.sh  # Start background service
./scripts/stop_semantic_service.sh   # Stop background service
```

#### VS Code Integration
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

### Cross-Service Communication (Future)

#### Service Discovery Pattern
```python
# Future service registry implementation
class ServiceRegistry:
    services = {
        "semantic_search": "http://localhost:8080",
        "motor_control": "http://localhost:3000",
        "system_monitor": "http://localhost:4000"
    }
    
    def get_service_url(self, service_name: str) -> str:
        return self.services.get(service_name)
```

#### Health Check Aggregation
```python
# Future health check aggregation
async def check_all_services():
    services = ["semantic_search", "motor_control", "system_monitor"]
    health_checks = []
    
    for service in services:
        url = f"{service_registry.get_service_url(service)}/health"
        health_checks.append(check_service_health(url))
    
    return await asyncio.gather(*health_checks)
```

## Performance Characteristics

### Semantic Search API Performance

| Metric | Typical | Peak | Cached |
|--------|---------|------|--------|
| Simple Query (STM32H7) | 2-5s | 15s | 0.0ms |
| Complex Query (all collections) | 5-20s | 30s | 0.0ms |
| Health Check | 50-200ms | 5s | N/A |
| Memory Usage | 50-100MB | 200MB | - |
| Concurrent Requests | 10 max | - | Unlimited |

### Future Performance Targets

| Service | Response Time | Throughput | Memory |
|---------|---------------|------------|--------|
| Motor Control | <10ms | 100 req/s | <50MB |
| System Monitor | <100ms | 50 req/s | <30MB |
| Gateway | <5ms | 500 req/s | <100MB |

## Development Workflow Integration

### API Development Lifecycle

#### 1. Design Phase
- **OpenAPI Specification**: Define API contract first
- **Schema Validation**: Ensure request/response schemas are complete
- **Documentation**: Write comprehensive API documentation
- **Mock Implementation**: Create mock services for early integration

#### 2. Implementation Phase
- **Service Implementation**: Build REST service with proper error handling
- **Client Library**: Develop client libraries for common languages
- **Testing Suite**: Comprehensive unit and integration tests
- **Performance Testing**: Load testing and performance validation

#### 3. Integration Phase
- **Service Discovery**: Implement service registry and discovery
- **API Gateway**: Centralized request routing and authentication
- **Monitoring**: Comprehensive service monitoring and alerting
- **Documentation**: Update documentation with real performance metrics

### Feature Development Integration

#### Semantic Search Enhancement Example
```bash
# Feature tracking integration
python scripts/feature_tracker.py update FTR-XXX --status IN_PROGRESS

# API development workflow
1. Update OpenAPI specification
2. Implement new endpoint in semantic_search_service.py
3. Update client library (stm32_semantic_search.py)
4. Add integration tests
5. Update API documentation

# Status and documentation sync
python scripts/auto_update_status.py --source build --verbose
```

## Security and Authentication

### Current Security Posture

#### Semantic Search API
- **Local Development**: Localhost binding only (127.0.0.1:8080)
- **No Authentication**: Designed for local development environment
- **Input Validation**: Basic parameter validation and sanitization
- **Resource Protection**: Request timeouts and size limits
- **Error Handling**: No sensitive information exposure

### Future Security Enhancements

#### Production Security Roadmap
1. **API Gateway Authentication**: JWT token-based authentication
2. **Rate Limiting**: Per-client request rate limiting with Redis
3. **HTTPS/TLS**: End-to-end encryption for all API communications
4. **API Keys**: Service-to-service authentication with rotating keys
5. **Audit Logging**: Comprehensive API access and operation logging
6. **Input Sanitization**: Advanced XSS and injection attack protection

#### Security Architecture (Planned)
```
┌─────────────────┐    HTTPS/JWT    ┌─────────────────┐
│   API Gateway   │ ◄──────────────► │  Client Apps    │
│  (Auth + Rate   │                  │                 │
│   Limiting)     │                  └─────────────────┘
└─────────────────┘
          │
          ▼ Internal Network
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ Semantic Search │    │ Motor Control   │    │ System Monitor  │
│ Service         │    │ Service         │    │ Service         │
│ (Internal Only) │    │ (Internal Only) │    │ (Internal Only) │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Testing Strategy

### Current Testing Infrastructure

#### Semantic Search API Testing
- **Unit Tests**: Service component testing with mocked dependencies
- **Integration Tests**: End-to-end API testing with real ChromaDB
- **Performance Tests**: Load testing with concurrent requests
- **Health Check Validation**: Service health monitoring verification

#### Testing Commands
```bash
# Test semantic search functionality
python scripts/stm32_semantic_search.py concept "test" --scope all

# Health check validation
curl -f http://localhost:8080/health || echo "Service unhealthy"

# Load testing (future)
python tests/load_test_semantic_api.py --concurrent 10 --duration 60s
```

### Future Testing Framework

#### Comprehensive API Testing Suite
1. **Contract Testing**: OpenAPI specification validation
2. **Load Testing**: Performance testing under realistic loads
3. **Security Testing**: Authentication and authorization validation
4. **Chaos Engineering**: Service resilience and fault tolerance testing
5. **End-to-End Testing**: Full workflow validation across all services

## Monitoring and Observability

### Current Monitoring

#### Semantic Search Service
- **Health Endpoints**: `/health` and `/status` for service monitoring
- **Service Logs**: Comprehensive request and error logging
- **Performance Metrics**: Query response times and cache hit rates
- **Resource Monitoring**: Memory usage and database connection status

#### Monitoring Commands
```bash
# Service health check
curl http://localhost:8080/health | jq '.status'

# Performance monitoring
curl http://localhost:8080/status | jq '.cache_hit_rate'

# Log monitoring
tail -f logs/semantic_search_service.log
```

### Future Monitoring Framework

#### Production Monitoring Stack
1. **Metrics Collection**: Prometheus for metrics aggregation
2. **Log Aggregation**: ELK stack for centralized logging
3. **Tracing**: Distributed tracing with Jaeger
4. **Alerting**: PagerDuty integration for critical alerts
5. **Dashboards**: Grafana for real-time monitoring dashboards

## Migration and Versioning

### API Versioning Strategy

#### Semantic Versioning
- **Major Version**: Breaking changes to API contract
- **Minor Version**: New features with backward compatibility
- **Patch Version**: Bug fixes and non-breaking changes

#### Version Management
```yaml
# OpenAPI versioning example
info:
  version: "2.0.0"  # Current: Semantic search integration
  # 1.0.0: Initial motor control API
  # 2.0.0: Added semantic search capabilities
  # 3.0.0: (Future) Unified API gateway
```

### Migration Support

#### Backward Compatibility
- **API Contract Stability**: Maintain existing endpoint compatibility
- **Deprecation Notices**: Clear deprecation timelines and migration paths
- **Client Library Updates**: Automatic client library version management
- **Documentation Versioning**: Version-specific documentation maintenance

## Future Roadmap

### Phase 1: Motor Control API Implementation (Q4 2025)
- [ ] REST API server for motor control operations
- [ ] Real-time motor status and position feedback
- [ ] Safety system integration with emergency stop capabilities
- [ ] Multi-motor coordination and synchronization

### Phase 2: System Integration (Q1 2026)
- [ ] API Gateway implementation with unified authentication
- [ ] Service discovery and load balancing
- [ ] Comprehensive monitoring and alerting system
- [ ] Production-ready security implementation

### Phase 3: Advanced Features (Q2 2026)
- [ ] WebSocket support for real-time updates
- [ ] GraphQL API for flexible data querying
- [ ] Machine learning integration for predictive maintenance
- [ ] Mobile app API support with optimized endpoints

### Phase 4: Enterprise Features (Q3 2026)
- [ ] Multi-tenant support with tenant isolation
- [ ] Advanced analytics and reporting APIs
- [ ] Third-party integration APIs and webhooks
- [ ] High-availability deployment patterns

## Conclusion

The STM32H753ZI API ecosystem provides a comprehensive foundation for motor control and documentation search capabilities. The current semantic search API demonstrates production-ready quality with high performance, comprehensive documentation, and robust error handling.

The planned motor control and system monitoring APIs will extend this foundation to create a complete embedded systems control and monitoring platform.

### Key Strengths
- **Production Quality**: Semantic search API is fully operational and documented
- **Scalable Architecture**: Client-server design supports multiple concurrent users
- **Comprehensive Documentation**: Complete API reference with examples and integration guides
- **Performance Optimized**: Cached responses and efficient query processing
- **Development Integration**: VS Code tasks and workflow automation

### Next Steps
1. **Complete Motor Control API**: Implement REST interface for hardware control
2. **System Monitoring Integration**: Add comprehensive system health monitoring
3. **API Gateway Development**: Unified authentication and request routing
4. **Production Deployment**: Security, monitoring, and high-availability features

---

*This overview provides a complete picture of the API ecosystem for the STM32H753ZI project. For specific API details, refer to the individual API documentation files.*
