# High-Performance Semantic Search System Design

## Overview
Complete redesign of semantic search system to achieve **sub-2-second query response times** (down from 25+ seconds) through intelligent architecture, lazy loading, and optimized caching.

## Performance Requirements (FTR-014)
- **Query Response**: <2 seconds (current: 25+ seconds)
- **Database Rebuild**: <5 minutes (current: >30 minutes)
- **Memory Efficiency**: Load only required collections
- **Local Resource**: Database not tracked in git, fast regeneration
- **Developer Experience**: Seamless integration with workflow

## Current Performance Issues
1. **Full Database Loading**: Loads all 252k+ documents on every query
2. **No Collection Targeting**: Loads L6470 docs for STM32H7 queries
3. **No Persistence**: Recreates connections every execution
4. **No Caching**: Rebuilds indexes unnecessarily
5. **Inefficient Architecture**: Script-based rather than service-based

## New Architecture: Service-Based Design

### Core Components

#### 1. Semantic Search Service (`semantic_search_service.py`)
```python
class SemanticSearchService:
    """High-performance semantic search with lazy loading and caching"""
    
    def __init__(self):
        self.collections = {}  # Lazy-loaded collections
        self.client = None     # Persistent ChromaDB client
        self.cache = {}        # Query result cache
    
    def query(self, text: str, scope: str = "all", max_results: int = 10):
        """Execute semantic query with lazy collection loading"""
        # Load only required collections based on scope
        # Use cached results when available
        # Return results in <2 seconds
```

#### 2. Collection Manager (`collection_manager.py`)
```python
class CollectionManager:
    """Manages lazy loading and caching of specific collections"""
    
    def load_collection(self, collection_name: str):
        """Load specific collection only when needed"""
        
    def get_collection_stats(self, collection_name: str):
        """Fast collection metadata without loading documents"""
```

#### 3. Optimized Search Interface (`semantic_search_optimized.py`)
```python
def stm32_search(query_type: str, text: str, scope: str = "all"):
    """User-friendly interface to high-performance search service"""
    # Direct interface to service
    # Intelligent scope mapping
    # Result formatting and display
```

### Performance Optimizations

#### 1. Lazy Collection Loading
```python
# OLD: Load all collections (252k+ documents)
def initialize_database():
    load_all_collections()  # 25+ seconds
    
# NEW: Load only what's needed
def load_collection_on_demand(scope):
    if scope == "STM32H7":
        return load_stm32_hal_collection()  # 241k docs
    elif scope == "L6470":
        return load_motor_control_collection()  # 9k docs
    # Load time: 2-5 seconds per collection
```

#### 2. Persistent Service Architecture
```python
# OLD: Script execution model
python stm32_semantic_search.py  # Reload everything

# NEW: Service model with persistence
service = SemanticSearchService()  # One-time initialization
service.query("GPIO config", "STM32H7")  # Fast subsequent queries
```

#### 3. Intelligent Caching
```python
class QueryCache:
    """Cache query results and collection metadata"""
    
    def __init__(self):
        self.query_cache = {}     # Recent query results
        self.metadata_cache = {}  # Collection statistics
        self.embedding_cache = {} # Computed embeddings
```

#### 4. Scope-Optimized Loading
| Scope | Collections Loaded | Document Count | Load Time |
|-------|-------------------|----------------|-----------|
| `STM32H7` | stm32_hal only | 241k | ~3 seconds |
| `L6470` | motor_control only | 9k | ~1 second |
| `project` | project_code only | 187 | <1 second |
| `all` | All collections | 252k | ~5 seconds |

## Implementation Strategy

### Phase 1: Core Service Architecture
1. **Create SemanticSearchService class**
   - Persistent ChromaDB client
   - Lazy collection loading
   - Basic caching framework

2. **Implement Collection Manager**
   - Collection-specific loading
   - Metadata caching
   - Memory management

### Phase 2: Performance Optimization
1. **Query Result Caching**
   - LRU cache for recent queries
   - Embedding result caching
   - Collection statistics caching

2. **Intelligent Scope Handling**
   - Automatic scope detection
   - Collection mapping optimization
   - Query routing logic

### Phase 3: Developer Experience
1. **Wrapper Scripts**
   - `stm32_search.sh` with service backend
   - Backwards-compatible interfaces
   - Enhanced error handling

2. **Performance Monitoring**
   - Query timing metrics
   - Cache hit rates
   - Collection usage statistics

### Phase 4: Advanced Features
1. **Incremental Updates**
   - Fast collection refresh
   - Document change detection
   - Selective reindexing

2. **Advanced Caching**
   - Persistent disk cache
   - Cross-session optimization
   - Intelligent prefetching

## Expected Performance Improvements

### Query Performance
- **Current**: 25+ seconds (full database load)
- **Target**: <2 seconds (lazy loading + caching)
- **Improvement**: >12x faster

### Memory Usage
- **Current**: 1.5GB+ (all collections loaded)
- **Target**: 200-600MB (collection-specific loading)
- **Improvement**: 60-85% reduction

### Database Rebuild
- **Current**: >30 minutes (full rebuild)
- **Target**: <5 minutes (optimized pipeline)
- **Improvement**: >6x faster

## Database Management Strategy

### Local Resource Approach
```bash
# Database not tracked in git (.gitignore)
docs/semantic_vector_db/

# Fast regeneration when needed
./scripts/rebuild_semantic_db.sh  # <5 minutes

# Service-based access
./scripts/stm32_search.sh concept "GPIO" --scope STM32H7  # <2 seconds
```

### Collection Structure
```
docs/semantic_vector_db/
├── stm32_hal/          # 241k docs - STM32H7 documentation
├── motor_control/      # 9k docs - L6470/stepper control
├── project_code/       # 187 docs - project source
├── instruction_guides/ # 1.8k docs - development guides
└── safety_systems/     # future expansion
```

## Integration with Development Workflow

### VS Code Tasks Integration
```json
{
  "label": "Semantic Search: STM32H7",
  "type": "shell", 
  "command": "./scripts/stm32_search.sh concept '${input:searchQuery}' --scope STM32H7"
}
```

### Command Line Interface
```bash
# High-performance semantic search
./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7
./scripts/stm32_search.sh register "ABS_POS" --scope L6470

# Service management
./scripts/semantic_service.sh start    # Start persistent service
./scripts/semantic_service.sh status   # Check service health
./scripts/semantic_service.sh restart  # Restart with fresh collections
```

### Feature Tracking Integration
- **FTR-014**: High-Performance Semantic Search System
- **Performance Target**: <2 second queries, <5 minute rebuilds
- **Status Tracking**: Query timing metrics in STATUS.md
- **Development Integration**: Seamless workflow with existing tools

## Success Metrics

### Performance Benchmarks
- [ ] Query response time consistently <2 seconds
- [ ] Database rebuild completes in <5 minutes  
- [ ] Memory usage optimized for collection scope
- [ ] Cache hit rate >70% for common queries
- [ ] Zero impact on git repository size

### Developer Experience
- [ ] Backwards compatible with existing search commands
- [ ] Enhanced error messages and debugging
- [ ] Integration with VS Code workflow
- [ ] Automated performance monitoring
- [ ] Clear documentation and usage examples

## Risk Mitigation

### Performance Risks
- **ChromaDB Connection Overhead**: Persistent client connections
- **Memory Leaks**: Proper collection cleanup and management
- **Cache Invalidation**: Smart cache expiry and refresh logic

### Compatibility Risks  
- **Existing Scripts**: Maintain wrapper compatibility
- **Database Format**: Version compatibility checks
- **Dependencies**: Locked package versions

### Operational Risks
- **Database Corruption**: Fast rebuild capability as backup
- **Service Failures**: Graceful fallback to direct access
- **Resource Constraints**: Memory usage monitoring and limits

This design achieves the performance goals of FTR-014 while maintaining developer productivity and system reliability.
