#!/usr/bin/env python3
"""
High-Performance Semantic Search System for STM32H753ZI Project

This module provides ultra-fast semantic search capabilities with:
- Targeted collection loading (only load what's needed)
- Persistent connection reuse
- Collection-level caching
- Sub-second query times
- Compatible with existing 1024-dimensional embeddings

Author: STM32H753ZI Project Team
Date: August 11, 2025
"""

import chromadb
import time
import logging
from typing import List, Dict, Any, Optional, Set, Union
from enum import Enum
from dataclasses import dataclass

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class CollectionScope(Enum):
    """Available search scopes mapped to collections."""
    STM32H7 = "stm32_hal"
    L6470 = "motor_control"
    NUCLEO_BSP = "stm32_hal"  # BSP content is in stm32_hal collection
    PROJECT = "project_code"
    GUIDES = "instruction_guides"
    SAFETY = "safety_systems"
    ALL = "all"

@dataclass
class SearchResult:
    """Structured search result."""
    content: str
    metadata: Dict[str, Any]
    distance: float
    collection: str

class FastCollectionManager:
    """Ultra-fast collection manager with targeted loading."""
    
    def __init__(self, db_path: str = "./docs/semantic_vector_db"):
        """Initialize the high-performance semantic search system."""
        self.db_path = db_path
        self.client = None
        self.collection_cache = {}
        self.stats = {
            'collections_loaded': 0,
            'total_load_time': 0.0,
            'queries_served': 0,
            'cache_hits': 0
        }
        
        # Initialize ChromaDB client - use default embedding function to match existing database
        start_time = time.time()
        self.client = chromadb.PersistentClient(path=db_path)
        init_time = time.time() - start_time
        logger.info(f"📊 ChromaDB client initialized in {init_time*1000:.1f}ms")
        
        # Validate database
        self._validate_database()
    
    def _validate_database(self) -> None:
        """Validate database structure and available collections."""
        try:
            collections = self.client.list_collections()
            collection_names = [col.name for col in collections]
            logger.info(f"✅ Database validation passed - {len(collections)} collections available")
            for name in collection_names:
                logger.info(f"   - {name}")
        except Exception as e:
            logger.error(f"❌ Database validation failed: {e}")
            raise
    
    def get_collection(self, collection_name: str):
        """Get collection with caching for speed."""
        if collection_name in self.collection_cache:
            self.stats['cache_hits'] += 1
            return self.collection_cache[collection_name]
        
        start_time = time.time()
        try:
            collection = self.client.get_collection(collection_name)
            load_time = time.time() - start_time
            
            # Cache the collection
            self.collection_cache[collection_name] = collection
            self.stats['collections_loaded'] += 1
            self.stats['total_load_time'] += load_time
            
            logger.info(f"🚀 Loaded collection '{collection_name}' ({collection.count():,} docs) in {load_time*1000:.1f}ms")
            return collection
            
        except Exception as e:
            logger.error(f"❌ Failed to load collection '{collection_name}': {e}")
            return None
    
    def _scope_to_collections(self, scope: str) -> List[str]:
        """Map search scope to collection names."""
        scope_upper = scope.upper()
        
        if scope_upper == "ALL":
            return ["motor_control", "instruction_guides", "project_code", "stm32_hal", "safety_systems"]
        elif scope_upper in ["STM32H7", "STM32_HAL"]:
            return ["stm32_hal"]
        elif scope_upper == "L6470":
            return ["motor_control"]
        elif scope_upper in ["NUCLEO_BSP", "BSP"]:
            return ["stm32_hal"]  # BSP content is in stm32_hal
        elif scope_upper == "PROJECT":
            return ["project_code"]
        elif scope_upper in ["GUIDES", "INSTRUCTIONS"]:
            return ["instruction_guides"]
        elif scope_upper == "SAFETY":
            return ["safety_systems"]
        else:
            # Default to all collections for unknown scopes
            return ["motor_control", "instruction_guides", "project_code", "stm32_hal", "safety_systems"]
    
    def search(self, query: str, scope: str = "all", n_results: int = 3) -> List[SearchResult]:
        """Perform high-speed semantic search."""
        start_time = time.time()
        results = []
        
        # Get target collections based on scope
        target_collections = self._scope_to_collections(scope)
        collections_processed = 0
        
        logger.info(f"🔍 Testing: '{query}' in scope '{scope}'")
        
        for collection_name in target_collections:
            collection = self.get_collection(collection_name)
            if collection is None:
                continue
                
            try:
                # Query the collection using its native embedding function
                search_results = collection.query(
                    query_texts=[query],
                    n_results=n_results
                )
                
                # Convert to structured results
                if search_results and isinstance(search_results, dict) and 'documents' in search_results:
                    docs_list = search_results.get('documents')
                    metas_list = search_results.get('metadatas')
                    dists_list = search_results.get('distances')
                    
                    if docs_list and len(docs_list) > 0:
                        documents = docs_list[0] if docs_list[0] else []
                        metadatas = metas_list[0] if metas_list and len(metas_list) > 0 else []
                        distances = dists_list[0] if dists_list and len(dists_list) > 0 else []
                        
                        for i, doc in enumerate(documents):
                            metadata = dict(metadatas[i]) if i < len(metadatas) and metadatas[i] else {}
                            distance = distances[i] if i < len(distances) else 0.0
                            
                            results.append(SearchResult(
                                content=str(doc),
                                metadata=metadata,
                                distance=float(distance),
                                collection=collection_name
                            ))
                
                collections_processed += 1
                
            except Exception as e:
                logger.error(f"❌ Search failed in collection '{collection_name}': {e}")
        
        # Sort results by relevance (lower distance = more relevant)
        results.sort(key=lambda x: x.distance)
        
        search_time = time.time() - start_time
        self.stats['queries_served'] += 1
        
        logger.info(f"🔍 Search completed in {search_time*1000:.1f}ms across {collections_processed} collections")
        logger.info(f"📊 Found {len(results)} results (showing top {n_results})")
        
        return results[:n_results]
    
    def get_stats(self) -> Dict[str, Any]:
        """Get performance statistics."""
        return self.stats.copy()

def main():
    """Test the high-performance semantic search system."""
    print("🧪 Testing Fast Collection Manager")
    print("=" * 50)
    
    # Initialize the fast search system
    search_manager = FastCollectionManager()
    
    # Test queries
    test_queries = [
        ("GPIO configuration", "stm32h7"),
        ("L6470 registers", "l6470"),
        ("BSP functions", "nucleo_bsp"),
        ("motor control", "all")
    ]
    
    for query, scope in test_queries:
        results = search_manager.search(query, scope, n_results=3)
        
        # Display results
        for i, result in enumerate(results, 1):
            print(f"  {i}. [{result.collection}] Distance: {result.distance:.3f}")
            print(f"     {result.content[:100]}...")
            if result.metadata:
                print(f"     Metadata: {result.metadata}")
            print()
    
    # Show performance stats
    stats = search_manager.get_stats()
    print("📊 Performance Statistics:")
    for key, value in stats.items():
        print(f"   {key}: {value}")

if __name__ == "__main__":
    main()
