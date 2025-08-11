#!/usr/bin/env python3
"""
Collection Manager for High-Performance Semantic Search
Fast, targeted collection management with lazy loading
"""

import os
import json
import time
from pathlib import Path
from typing import Dict, List, Optional, Set, Any
from dataclasses import dataclass
from enum import Enum

import chromadb
from chromadb.config import Settings


class CollectionScope(Enum):
    """Collection scope definitions for targeted loading"""
    STM32H7 = "stm32_hal"
    L6470 = "motor_control" 
    NUCLEO_BSP = "nucleo_bsp"
    PROJECT = "project_source"
    INSTRUCTIONS = "instruction_guides"
    ALL = "all"


@dataclass
class CollectionInfo:
    """Collection metadata for fast access decisions"""
    name: str
    doc_count: int
    size_mb: float
    last_updated: str
    load_time_ms: float = 0
    
    
class FastCollectionManager:
    """High-speed collection manager with intelligent loading"""
    
    def __init__(self, db_path: str = "docs/semantic_vector_db"):
        self.db_path = Path(db_path)
        self.client = None
        self.loaded_collections: Dict[str, Any] = {}
        self.collection_info: Dict[str, CollectionInfo] = {}
        self.load_stats = {
            "collections_loaded": 0,
            "total_load_time": 0.0,
            "queries_served": 0,
            "cache_hits": 0
        }
        
        # Collection mapping for fast scope resolution
        self.scope_mapping = {
            CollectionScope.STM32H7: ["stm32_hal"],
            CollectionScope.L6470: ["motor_control"],
            CollectionScope.NUCLEO_BSP: ["nucleo_bsp"],
            CollectionScope.PROJECT: ["project_source"],
            CollectionScope.INSTRUCTIONS: ["instruction_guides"],
            CollectionScope.ALL: []  # Will be populated dynamically
        }
        
    def initialize_client(self) -> bool:
        """Initialize ChromaDB client with optimized settings"""
        try:
            if self.client is None:
                start_time = time.time()
                
                self.client = chromadb.PersistentClient(
                    path=str(self.db_path),
                    settings=Settings(
                        anonymized_telemetry=False,
                        is_persistent=True,
                        persist_directory=str(self.db_path)
                    )
                )
                
                init_time = (time.time() - start_time) * 1000
                print(f"📊 ChromaDB client initialized in {init_time:.1f}ms")
                
                # Populate available collections for ALL scope
                available_collections = [c.name for c in self.client.list_collections()]
                self.scope_mapping[CollectionScope.ALL] = available_collections
                
                return True
                
        except Exception as e:
            print(f"❌ Failed to initialize ChromaDB client: {e}")
            return False
            
        return True
        
    def get_collections_for_scope(self, scope: str) -> List[str]:
        """Get collection names for a given scope"""
        try:
            scope_enum = CollectionScope(scope.lower())
        except ValueError:
            scope_enum = CollectionScope.ALL
            
        return self.scope_mapping.get(scope_enum, [])
        
    def load_collection_lazy(self, collection_name: str) -> Optional[Any]:
        """Load a single collection with caching"""
        if collection_name in self.loaded_collections:
            self.load_stats["cache_hits"] += 1
            return self.loaded_collections[collection_name]
            
        if not self.initialize_client():
            return None
            
        try:
            start_time = time.time()
            
            if self.client is None:
                return None
                
            collection = self.client.get_collection(collection_name)
            load_time = (time.time() - start_time) * 1000
            
            # Cache the collection
            self.loaded_collections[collection_name] = collection
            
            # Update stats
            self.load_stats["collections_loaded"] += 1
            self.load_stats["total_load_time"] += load_time
            
            # Store collection info
            doc_count = collection.count()
            self.collection_info[collection_name] = CollectionInfo(
                name=collection_name,
                doc_count=doc_count,
                size_mb=0,  # Would need file system analysis
                last_updated=time.strftime("%Y-%m-%d %H:%M:%S"),
                load_time_ms=load_time
            )
            
            print(f"🚀 Loaded collection '{collection_name}' ({doc_count:,} docs) in {load_time:.1f}ms")
            return collection
            
        except Exception as e:
            print(f"❌ Failed to load collection '{collection_name}': {e}")
            return None
            
    def search_collections(self, query: str, scope: str, max_results: int = 10) -> List[Dict]:
        """Search specific collections based on scope"""
        start_time = time.time()
        
        collection_names = self.get_collections_for_scope(scope)
        if not collection_names:
            print(f"⚠️  No collections found for scope: {scope}")
            return []
            
        all_results = []
        
        for collection_name in collection_names:
            collection = self.load_collection_lazy(collection_name)
            if not collection:
                continue
                
            try:
                # Perform semantic search
                results = collection.query(
                    query_texts=[query],
                    n_results=max_results,
                    include=["documents", "metadatas", "distances"]
                )
                
                # Format results with collection context
                for i, doc in enumerate(results['documents'][0]):
                    metadata = results['metadatas'][0][i] if results['metadatas'] else {}
                    distance = results['distances'][0][i] if results['distances'] else 1.0
                    
                    all_results.append({
                        'content': doc,
                        'metadata': metadata,
                        'distance': distance,
                        'collection': collection_name,
                        'relevance_score': 1.0 - distance  # Convert distance to relevance
                    })
                    
            except Exception as e:
                print(f"❌ Search failed in collection '{collection_name}': {e}")
                continue
                
        # Sort by relevance and limit results
        all_results.sort(key=lambda x: x['relevance_score'], reverse=True)
        final_results = all_results[:max_results]
        
        search_time = (time.time() - start_time) * 1000
        self.load_stats["queries_served"] += 1
        
        print(f"🔍 Search completed in {search_time:.1f}ms across {len(collection_names)} collections")
        print(f"📊 Found {len(final_results)} results (showing top {max_results})")
        
        return final_results
        
    def get_performance_stats(self) -> Dict:
        """Get performance statistics"""
        return {
            **self.load_stats,
            "loaded_collections": list(self.loaded_collections.keys()),
            "collection_info": {name: {
                "doc_count": info.doc_count,
                "load_time_ms": info.load_time_ms
            } for name, info in self.collection_info.items()}
        }
        
    def clear_cache(self) -> None:
        """Clear loaded collection cache"""
        self.loaded_collections.clear()
        self.collection_info.clear()
        print("🗑️  Collection cache cleared")
        
    def validate_database(self) -> bool:
        """Quick database validation without full load"""
        if not self.initialize_client():
            return False
            
        try:
            if self.client is None:
                return False
                
            collections = self.client.list_collections()
            if not collections:
                print("⚠️  No collections found in database")
                return False
                
            print(f"✅ Database validation passed - {len(collections)} collections available")
            for collection in collections:
                print(f"   - {collection.name}")
                
            return True
            
        except Exception as e:
            print(f"❌ Database validation failed: {e}")
            return False


def main():
    """Test the collection manager"""
    manager = FastCollectionManager()
    
    print("🧪 Testing Fast Collection Manager")
    print("=" * 50)
    
    # Validate database
    if not manager.validate_database():
        print("❌ Database validation failed")
        return
        
    # Test scoped search
    test_queries = [
        ("GPIO configuration", "stm32h7"),
        ("L6470 registers", "l6470"),
        ("BSP functions", "nucleo_bsp"),
        ("motor control", "all")
    ]
    
    for query, scope in test_queries:
        print(f"\n🔍 Testing: '{query}' in scope '{scope}'")
        results = manager.search_collections(query, scope, max_results=3)
        
        for i, result in enumerate(results):
            print(f"  {i+1}. [{result['collection']}] {result['content'][:100]}...")
            print(f"     Relevance: {result['relevance_score']:.3f}")
            
    # Show performance stats
    print(f"\n📊 Performance Statistics:")
    stats = manager.get_performance_stats()
    for key, value in stats.items():
        if key not in ['loaded_collections', 'collection_info']:
            print(f"   {key}: {value}")


if __name__ == "__main__":
    main()
