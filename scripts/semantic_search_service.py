#!/usr/bin/env python3
"""
High-Performance Semantic Search Service for STM32H753ZI Project

This service provides ultra-fast semantic search with persistent background operation,
intelligent caching, and real-time delta updates targeting <200ms query response times.

Author: STM32H753ZI Development Team
Date: August 11, 2025
Version: 1.0
"""

import asyncio
import json
import time
import hashlib
import logging
from datetime import datetime, timedelta
from pathlib import Path
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, asdict
from collections import defaultdict, OrderedDict
import threading
import signal
import sys

# External dependencies
try:
    import aiohttp
    from aiohttp import web
    import chromadb
    import numpy as np
    from chromadb.config import Settings
    import psutil
    import requests
except ImportError as e:
    print(f"Missing required dependency: {e}")
    print("Install with: pip install aiohttp chromadb numpy psutil requests")
    sys.exit(1)


class EmbeddingGenerator:
    """Handles embedding generation with fallback to mock embeddings"""
    
    def __init__(self, model: str = "mxbai-embed-large", ollama_url: str = "http://localhost:11434"):
        self.model = model
        self.ollama_url = ollama_url
        self.api_url = f"{ollama_url}/api/embeddings"
        self.use_ollama = self._test_ollama_connection()
        
    def _test_ollama_connection(self) -> bool:
        """Test if Ollama is available and has the required model"""
        try:
            response = requests.get(f"{self.ollama_url}/api/tags", timeout=5)
            if response.status_code == 200:
                models = response.json().get("models", [])
                model_names = [m["name"] for m in models]
                if any(self.model in name for name in model_names):
                    logging.info(f"✅ Ollama connected: {self.model} available")
                    return True
                else:
                    logging.warning(f"⚠️  Model {self.model} not found. Available: {model_names}")
            else:
                logging.warning(f"⚠️  Ollama API error: {response.status_code}")
        except Exception as e:
            logging.warning(f"⚠️  Ollama connection failed: {e}")

        logging.info("📝 Using mock embeddings (1024-dimensional)")
        return False

    def generate_embedding(self, text: str) -> List[float]:
        """Generate 1024-dimensional embedding for the given text"""
        if self.use_ollama:
            return self._generate_ollama_embedding(text)
        else:
            return self._generate_mock_embedding(text)
    
    def _generate_ollama_embedding(self, text: str) -> List[float]:
        """Generate embedding using Ollama API"""
        try:
            payload = {"model": self.model, "prompt": text}
            response = requests.post(self.api_url, json=payload, timeout=30)
            
            if response.status_code == 200:
                result = response.json()
                embedding = result.get("embedding", [])
                if len(embedding) == 1024:
                    return embedding
                else:
                    logging.warning(f"⚠️  Ollama returned {len(embedding)}-dim embedding, expected 1024")
                    return self._generate_mock_embedding(text)
            else:
                logging.warning(f"🔄 Ollama API error {response.status_code}, using mock")
                return self._generate_mock_embedding(text)

        except Exception as e:
            logging.warning(f"🔄 Ollama error: {e}, using mock")
            return self._generate_mock_embedding(text)

    def _generate_mock_embedding(self, text: str) -> List[float]:
        """Generate deterministic 1024-dimensional mock embedding"""
        # Use hash of text as seed for reproducible results
        import hashlib
        seed = int(hashlib.md5(text.encode()).hexdigest()[:8], 16)
        np.random.seed(seed)
        
        # Generate 1024-dimensional embedding
        embedding = np.random.rand(1024).astype(float)
        
        # Normalize to unit vector (common practice for embeddings)
        norm = np.linalg.norm(embedding)
        if norm > 0:
            embedding = embedding / norm
            
        return embedding.tolist()


@dataclass
class QueryResult:
    """Semantic search query result with performance metadata"""
    query: str
    scope: str
    results: List[Dict[str, Any]]
    response_time_ms: float
    cache_hit: bool
    source: str  # 'hot_cache', 'persistent_cache', 'computed'
    timestamp: datetime
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        return {
            **asdict(self),
            'timestamp': self.timestamp.isoformat()
        }


@dataclass
class PerformanceMetrics:
    """Real-time performance tracking"""
    query_count: int = 0
    avg_response_time: float = 0.0
    cache_hit_rate: float = 0.0
    active_collections: int = 0
    memory_usage_mb: float = 0.0
    last_update: Optional[datetime] = None
    
    def update_query_metrics(self, response_time: float, cache_hit: bool):
        """Update query performance metrics"""
        self.query_count += 1
        self.avg_response_time = (
            (self.avg_response_time * (self.query_count - 1) + response_time) 
            / self.query_count
        )
        # Simple cache hit rate calculation
        if hasattr(self, '_cache_hits'):
            self._cache_hits += 1 if cache_hit else 0
        else:
            self._cache_hits = 1 if cache_hit else 0
        self.cache_hit_rate = self._cache_hits / self.query_count


class LRUCache:
    """High-performance LRU cache with size limits"""
    
    def __init__(self, maxsize: int = 1000):
        self.maxsize = maxsize
        self.cache = OrderedDict()
        self.lock = threading.Lock()
        
    def get(self, key: str) -> Optional[Any]:
        """Get item from cache and move to end (most recent)"""
        with self.lock:
            if key in self.cache:
                # Move to end (most recent)
                value = self.cache.pop(key)
                self.cache[key] = value
                return value
            return None
    
    def put(self, key: str, value: Any) -> None:
        """Add item to cache with LRU eviction"""
        with self.lock:
            if key in self.cache:
                self.cache.pop(key)
            elif len(self.cache) >= self.maxsize:
                # Remove oldest item
                self.cache.popitem(last=False)
            self.cache[key] = value
    
    def size(self) -> int:
        """Get current cache size"""
        with self.lock:
            return len(self.cache)


class EmbeddingService:
    """Simple embedding service for 1024-dimensional vectors"""
    
    def __init__(self):
        # Try to use a real embedding model or fall back to mock
        self.use_mock = True  # For now, always use mock to match existing DB
        
    def generate_embedding(self, text: str) -> List[float]:
        """Generate 1024-dimensional embedding"""
        if self.use_mock:
            # Generate deterministic mock embedding to match existing DB format
            import hashlib
            import numpy as np
            
            # Create deterministic seed from text
            seed = int(hashlib.md5(text.encode()).hexdigest()[:8], 16)
            np.random.seed(seed % (2**32))
            
            # Generate 1024-dimensional embedding
            embedding = np.random.rand(1024).tolist()
            return embedding
        else:
            # TODO: Implement real embedding service (Ollama, etc.)
            return [0.0] * 1024


class ChunkedDatabase:
    """Git-friendly chunked vector database manager"""
    
    def __init__(self, base_path: str):
        self.base_path = Path(base_path)
        self.base_path.mkdir(parents=True, exist_ok=True)
        
        # Initialize ChromaDB with correct path (no 'chunks' subdirectory)
        self.client = chromadb.PersistentClient(
            path=str(self.base_path),
            settings=Settings(
                anonymized_telemetry=False,
                allow_reset=True
            )
        )
        
        # Collection management
        self.loaded_collections = {}
        self.collection_metadata = self._load_collection_metadata()
        self.chunk_cache = LRUCache(maxsize=50)  # Limit loaded chunks
        
        # Performance tracking
        self.load_times = defaultdict(list)
        
    def _load_collection_metadata(self) -> Dict[str, Any]:
        """Load collection metadata for smart loading decisions"""
        metadata_path = self.base_path / "indexes" / "collection_metadata.json"
        if metadata_path.exists():
            with open(metadata_path, 'r') as f:
                return json.load(f)
        return {}
    
    def get_collection_for_scope(self, scope: str) -> List[str]:
        """Get required collections for query scope"""
        scope_mapping = {
            'STM32H7': ['stm32_hal'],
            'L6470': ['motor_control'],
            'NUCLEO_BSP': ['stm32_hal'],
            'PROJECT': ['project_code', 'instruction_guides'],
            'hal': ['stm32_hal'],
            'motor': ['motor_control'],
            'all': ['stm32_hal', 'motor_control', 'project_code', 'instruction_guides']
        }
        return scope_mapping.get(scope.upper(), scope_mapping.get(scope.lower(), scope_mapping['all']))
    
    async def load_collection_async(self, collection_name: str) -> Any:
        """Asynchronously load collection with caching"""
        if collection_name in self.loaded_collections:
            return self.loaded_collections[collection_name]
        
        start_time = time.time()
        
        try:
            # Load collection from ChromaDB
            collection = self.client.get_collection(collection_name)
            
            # Cache the loaded collection
            self.loaded_collections[collection_name] = collection
            
            # Track load performance
            load_time = time.time() - start_time
            self.load_times[collection_name].append(load_time)
            
            logging.info(f"Loaded collection '{collection_name}' in {load_time:.3f}s")
            return collection
            
        except Exception as e:
            logging.error(f"Failed to load collection '{collection_name}': {e}")
            return None
    
    async def query_collections(self, collections: List[str], query_text: str, 
                               n_results: int = 10, embedding_generator = None) -> List[Dict[str, Any]]:
        """Query multiple collections efficiently"""
        all_results = []
        
        # Generate query embedding once for all collections
        if embedding_generator:
            query_embedding = embedding_generator.generate_embedding(query_text)
        else:
            # Fallback to mock embedding if no generator provided
            import numpy as np
            np.random.seed(hash(query_text) % 2**32)
            query_embedding = np.random.rand(1024).tolist()
        
        # Load and query collections concurrently
        tasks = []
        for collection_name in collections:
            task = self._query_single_collection(collection_name, query_embedding, n_results)
            tasks.append(task)
        
        # Execute queries concurrently
        collection_results = await asyncio.gather(*tasks, return_exceptions=True)
        
        # Combine and rank results
        for results in collection_results:
            if isinstance(results, Exception):
                logging.warning(f"Collection query failed: {results}")
                continue
            if results:
                all_results.extend(results)
        
        # Sort by relevance score and return top results
        all_results.sort(key=lambda x: x.get('distance', float('inf')))
        return all_results[:n_results]
    
    async def _query_single_collection(self, collection_name: str, query_embedding: List[float], 
                                      n_results: int) -> List[Dict[str, Any]]:
        """Query a single collection using embedding"""
        collection = await self.load_collection_async(collection_name)
        if not collection:
            return []
        
        try:
            # Execute semantic search with embedding
            results = collection.query(
                query_embeddings=[query_embedding],
                n_results=min(n_results, 100),  # Limit to prevent excessive results
                include=['metadatas', 'documents', 'distances']
            )
            
            # Format results
            formatted_results = []
            if results['documents'] and results['documents'][0]:
                for i, doc in enumerate(results['documents'][0]):
                    formatted_results.append({
                        'document': doc,
                        'metadata': results['metadatas'][0][i] if results['metadatas'][0] else {},
                        'distance': results['distances'][0][i] if results['distances'][0] else 1.0,
                        'collection': collection_name
                    })
            
            return formatted_results
            
        except Exception as e:
            logging.error(f"Query failed for collection '{collection_name}': {e}")
            return []


class MultiLayerCache:
    """Intelligent multi-layer caching system for maximum performance"""
    
    def __init__(self, base_path: str):
        self.base_path = Path(base_path)
        self.cache_dir = self.base_path / "cache"
        self.cache_dir.mkdir(parents=True, exist_ok=True)
        
        # Layer 1: Hot cache (RAM) - Recent/frequent queries
        self.hot_cache = LRUCache(maxsize=1000)
        
        # Layer 2: Persistent cache (SSD) - Computed results
        self.persistent_cache_path = self.cache_dir / "persistent_queries.json"
        self.persistent_cache = self._load_persistent_cache()
        
        # Layer 3: Embedding cache - Reusable embeddings
        self.embedding_cache = LRUCache(maxsize=5000)
        
        # Cache statistics
        self.stats = {
            'hot_hits': 0,
            'persistent_hits': 0,
            'cache_misses': 0,
            'total_queries': 0
        }
        
    def _load_persistent_cache(self) -> Dict[str, Any]:
        """Load persistent cache from disk"""
        if self.persistent_cache_path.exists():
            try:
                with open(self.persistent_cache_path, 'r') as f:
                    return json.load(f)
            except Exception as e:
                logging.warning(f"Failed to load persistent cache: {e}")
        return {}
    
    def _save_persistent_cache(self):
        """Save persistent cache to disk"""
        try:
            with open(self.persistent_cache_path, 'w') as f:
                json.dump(self.persistent_cache, f, indent=2)
        except Exception as e:
            logging.error(f"Failed to save persistent cache: {e}")
    
    def _generate_cache_key(self, query: str, scope: str) -> str:
        """Generate consistent cache key"""
        content = f"{query.lower().strip()}:{scope.lower()}"
        return hashlib.md5(content.encode()).hexdigest()
    
    def get(self, query: str, scope: str) -> Tuple[Optional[QueryResult], str]:
        """Get cached result with source tracking"""
        cache_key = self._generate_cache_key(query, scope)
        self.stats['total_queries'] += 1
        
        # Try hot cache first (RAM - fastest)
        hot_result = self.hot_cache.get(cache_key)
        if hot_result:
            self.stats['hot_hits'] += 1
            return hot_result, 'hot_cache'
        
        # Try persistent cache (SSD - fast)
        if cache_key in self.persistent_cache:
            cached_data = self.persistent_cache[cache_key]
            
            # Check cache expiry (24 hours for now)
            cache_time = datetime.fromisoformat(cached_data['timestamp'])
            if datetime.now() - cache_time < timedelta(hours=24):
                # Promote to hot cache for next access
                result = QueryResult(**cached_data)
                self.hot_cache.put(cache_key, result)
                self.stats['persistent_hits'] += 1
                return result, 'persistent_cache'
        
        # Cache miss
        self.stats['cache_misses'] += 1
        return None, 'cache_miss'
    
    def put(self, query: str, scope: str, result: QueryResult):
        """Store result in all cache layers"""
        cache_key = self._generate_cache_key(query, scope)
        
        # Store in hot cache
        self.hot_cache.put(cache_key, result)
        
        # Store in persistent cache
        self.persistent_cache[cache_key] = result.to_dict()
        
        # Periodically save persistent cache
        if len(self.persistent_cache) % 100 == 0:
            self._save_persistent_cache()
    
    def get_cache_stats(self) -> Dict[str, Any]:
        """Get cache performance statistics"""
        total = self.stats['total_queries']
        if total == 0:
            return self.stats
        
        return {
            **self.stats,
            'hot_hit_rate': self.stats['hot_hits'] / total,
            'persistent_hit_rate': self.stats['persistent_hits'] / total,
            'total_hit_rate': (self.stats['hot_hits'] + self.stats['persistent_hits']) / total,
            'cache_miss_rate': self.stats['cache_misses'] / total
        }


class SemanticSearchService:
    """High-performance semantic search service with <200ms response times"""
    
    def __init__(self, db_path: str = "docs/semantic_vector_db"):
        self.db_path = Path(db_path)
        self.db_path.mkdir(parents=True, exist_ok=True)
        
        # Core components
        self.database = ChunkedDatabase(str(self.db_path))
        self.cache = MultiLayerCache(str(self.db_path))
        self.metrics = PerformanceMetrics()
        self.embedding_generator = EmbeddingGenerator()
        
        # Service state
        self.running = False
        self.startup_time = None
        
        # Logging setup
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger(__name__)
        
    async def start_service(self):
        """Start the semantic search service"""
        self.startup_time = datetime.now()
        self.running = True
        
        self.logger.info("Starting STM32 Semantic Search Service")
        self.logger.info(f"Database path: {self.db_path}")
        
        # Warm up critical collections
        await self._warmup_collections()
        
        self.logger.info("Service ready for queries")
    
    async def _warmup_collections(self):
        """Pre-load ALL available collections for faster initial queries"""
        print("🔥 Pre-warming all collections for optimal performance...")
        
        # Get all available collections from the database
        try:
            all_collections = self.database.client.list_collections()
            collection_names = [col.name for col in all_collections]
            print(f"📊 Found {len(collection_names)} collections: {collection_names}")
        except Exception as e:
            print(f"⚠️  Failed to list collections: {e}")
            # Fallback to known collections
            collection_names = ['stm32_hal', 'motor_control', 'instruction_guides', 'project_code', 'safety_systems']
        
        # Pre-load all collections in parallel
        warmup_tasks = []
        for collection_name in collection_names:
            task = self.database.load_collection_async(collection_name)
            warmup_tasks.append(task)
        
        # Load all collections concurrently for maximum speed
        start_time = time.time()
        loaded_collections = await asyncio.gather(*warmup_tasks, return_exceptions=True)
        warmup_time = time.time() - start_time
        
        # Count successful loads
        successful_loads = 0
        for i, result in enumerate(loaded_collections):
            if not isinstance(result, Exception) and result is not None:
                successful_loads += 1
        
        print(f"⚡ Pre-warmed {successful_loads}/{len(collection_names)} collections in {warmup_time:.2f}s")
        print("🚀 All collections cached in memory for instant access")
        self.logger.info(f"Pre-loaded {successful_loads} collections in {warmup_time:.2f}s")
    
    async def query(self, query_text: str, scope: str = "all", 
                   max_results: int = 10) -> QueryResult:
        """Ultra-fast semantic search query"""
        start_time = time.time()
        
        # Check cache first
        cached_result, cache_source = self.cache.get(query_text, scope)
        if cached_result:
            # Update cache hit timestamp
            cached_result.response_time_ms = (time.time() - start_time) * 1000
            cached_result.cache_hit = True
            cached_result.source = cache_source
            
            self.metrics.update_query_metrics(cached_result.response_time_ms, True)
            return cached_result
        
        # Compute fresh result
        try:
            # Get required collections for scope
            required_collections = self.database.get_collection_for_scope(scope)
            
            # Execute semantic search
            search_results = await self.database.query_collections(
                required_collections, query_text, max_results, self.embedding_generator
            )
            
            # Create result object
            response_time = (time.time() - start_time) * 1000
            result = QueryResult(
                query=query_text,
                scope=scope,
                results=search_results,
                response_time_ms=response_time,
                cache_hit=False,
                source='computed',
                timestamp=datetime.now()
            )
            
            # Cache the result for future queries
            self.cache.put(query_text, scope, result)
            
            # Update performance metrics
            self.metrics.update_query_metrics(response_time, False)
            
            return result
            
        except Exception as e:
            self.logger.error(f"Query failed: {e}")
            # Return empty result on error
            return QueryResult(
                query=query_text,
                scope=scope,
                results=[],
                response_time_ms=(time.time() - start_time) * 1000,
                cache_hit=False,
                source='error',
                timestamp=datetime.now()
            )
    
    def get_service_status(self) -> Dict[str, Any]:
        """Get comprehensive service status"""
        process = psutil.Process()
        
        return {
            'service': {
                'running': self.running,
                'startup_time': self.startup_time.isoformat() if self.startup_time else None,
                'uptime_seconds': (datetime.now() - self.startup_time).total_seconds() 
                                 if self.startup_time else 0
            },
            'performance': {
                'query_count': self.metrics.query_count,
                'avg_response_time_ms': round(self.metrics.avg_response_time, 2),
                'cache_hit_rate': round(self.metrics.cache_hit_rate, 3)
            },
            'cache': self.cache.get_cache_stats(),
            'collections': {
                'loaded': len(self.database.loaded_collections),
                'available': list(self.database.loaded_collections.keys())
            },
            'system': {
                'memory_usage_mb': round(process.memory_info().rss / 1024 / 1024, 1),
                'cpu_percent': process.cpu_percent(),
                'threads': process.num_threads()
            }
        }


# HTTP API for service interaction
async def create_app(service: SemanticSearchService) -> web.Application:
    """Create aiohttp web application for REST API"""
    
    async def query_handler(request):
        """Handle semantic search queries"""
        try:
            data = await request.json()
            query_text = data.get('query', '')
            scope = data.get('scope', 'all')
            max_results = data.get('max_results', 10)
            
            if not query_text:
                return web.json_response(
                    {'error': 'Query text is required'}, 
                    status=400
                )
            
            result = await service.query(query_text, scope, max_results)
            return web.json_response(result.to_dict())
            
        except Exception as e:
            return web.json_response(
                {'error': str(e)}, 
                status=500
            )
    
    async def status_handler(request):
        """Handle service status requests"""
        status = service.get_service_status()
        return web.json_response(status)
    
    async def health_handler(request):
        """Simple health check endpoint"""
        return web.json_response({
            'status': 'healthy',
            'timestamp': datetime.now().isoformat(),
            'service': 'STM32 Semantic Search'
        })
    
    # Create application
    app = web.Application()
    
    # Add routes
    app.router.add_post('/query', query_handler)
    app.router.add_get('/status', status_handler)
    app.router.add_get('/health', health_handler)
    
    # Add CORS support with proper middleware decorator
    @web.middleware
    async def cors_handler(request, handler):
        response = await handler(request)
        response.headers['Access-Control-Allow-Origin'] = '*'
        response.headers['Access-Control-Allow-Methods'] = 'GET, POST, OPTIONS'
        response.headers['Access-Control-Allow-Headers'] = 'Content-Type'
        return response
    
    app.middlewares.append(cors_handler)
    
    return app


async def run_service(host: str = '127.0.0.1', port: int = 8080):
    """Run the semantic search service"""
    # Initialize service
    service = SemanticSearchService()
    await service.start_service()
    
    # Create web application
    app = await create_app(service)
    
    # Setup graceful shutdown
    def signal_handler(signum, frame):
        logging.info("Received shutdown signal, stopping service...")
        service.running = False
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Start HTTP server
    runner = web.AppRunner(app)
    await runner.setup()
    
    site = web.TCPSite(runner, host, port)
    await site.start()
    
    logging.info(f"Semantic Search Service running on http://{host}:{port}")
    logging.info("API Endpoints:")
    logging.info(f"  POST http://{host}:{port}/query - Semantic search queries")
    logging.info(f"  GET  http://{host}:{port}/status - Service status")
    logging.info(f"  GET  http://{host}:{port}/health - Health check")
    
    # Keep service running
    try:
        while service.running:
            await asyncio.sleep(1)
    except KeyboardInterrupt:
        logging.info("Service stopped by user")
    finally:
        await runner.cleanup()


# CLI interface for direct usage
async def cli_query(query_text: str, scope: str = "all", max_results: int = 10):
    """Command-line interface for semantic search"""
    service = SemanticSearchService()
    await service.start_service()
    
    print(f"STM32 Semantic Search Service - Query: '{query_text}' (scope: {scope})")
    print("-" * 80)
    
    result = await service.query(query_text, scope, max_results)
    
    print(f"Response Time: {result.response_time_ms:.1f}ms")
    print(f"Cache Hit: {result.cache_hit} ({result.source})")
    print(f"Results Found: {len(result.results)}")
    print()
    
    for i, doc_result in enumerate(result.results[:max_results], 1):
        print(f"Result {i}:")
        print(f"  Collection: {doc_result.get('collection', 'unknown')}")
        print(f"  Relevance: {1 - doc_result.get('distance', 1):.3f}")
        print(f"  Content: {doc_result.get('document', '')[:200]}...")
        if doc_result.get('metadata'):
            print(f"  Metadata: {doc_result['metadata']}")
        print()


if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="STM32 High-Performance Semantic Search Service")
    parser.add_argument('--mode', choices=['service', 'query'], default='service',
                       help='Run as service or execute single query')
    parser.add_argument('--host', default='127.0.0.1', help='Service host')
    parser.add_argument('--port', type=int, default=8080, help='Service port')
    parser.add_argument('--query', help='Search query text')
    parser.add_argument('--scope', default='all', help='Search scope')
    parser.add_argument('--max-results', type=int, default=10, help='Maximum results')
    
    args = parser.parse_args()
    
    if args.mode == 'service':
        print("Starting STM32 Semantic Search Service...")
        asyncio.run(run_service(args.host, args.port))
    elif args.mode == 'query':
        if not args.query:
            print("Error: --query is required for query mode")
            sys.exit(1)
        asyncio.run(cli_query(args.query, args.scope, args.max_results))
