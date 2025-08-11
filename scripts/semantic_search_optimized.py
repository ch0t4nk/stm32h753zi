#!/usr/bin/env python3
"""
Optimized Semantic Search for STM32H753ZI Project

High-performance semantic search with lazy loading and intelligent caching.
Designed for sub-2-second query response times.

Usage:
    ./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
    
Author: STM32H753ZI Project  
Date: 2025-08-11
Feature: FTR-014 High-Performance Semantic Search System
"""

import os
import sys
import time
import json
import argparse
import logging
from typing import Dict, List, Optional, Any
from pathlib import Path

try:
    import chromadb
    from chromadb.config import Settings
    import requests
except ImportError as e:
    print(f"Missing required packages: {e}")
    print("Run: .venv/bin/pip install chromadb requests")
    sys.exit(1)

# Configure logging
logging.basicConfig(level=logging.WARNING)  # Reduce noise
logger = logging.getLogger(__name__)

class OptimizedSemanticSearch:
    """Optimized semantic search with lazy loading and caching"""
    
    def __init__(self, db_path: Optional[str] = None):
        self.db_path = db_path or "/workspaces/code/docs/semantic_vector_db"
        self.client: Optional[Any] = None
        self.collections: Dict[str, Any] = {}  # Loaded collections cache
        
        # Scope to collection mapping for targeted loading
        self.scope_collections = {
            "STM32H7": "stm32_hal",
            "stm32h7": "stm32_hal", 
            "L6470": "motor_control",
            "l6470": "motor_control",
            "motor": "motor_control",
            "stepper": "motor_control",
            "project": "project_code",
            "code": "project_code",
            "instructions": "instruction_guides",
            "guides": "instruction_guides"
        }
        
    def _init_client(self):
        """Initialize ChromaDB client lazily"""
        if self.client is not None:
            return True
            
        try:
            if not os.path.exists(self.db_path):
                print(f"ERROR: Database not found at {self.db_path}")
                print("Run: ./scripts/rebuild_semantic_db.sh")
                return False
                
            settings = Settings(
                chroma_db_impl="duckdb+parquet",
                persist_directory=self.db_path,
                anonymized_telemetry=False
            )
            
            self.client = chromadb.Client(settings)
            return True
            
        except Exception as e:
            print(f"ERROR: Failed to initialize database: {e}")
            return False
            
    def _load_collection(self, collection_name: str):
        """Load specific collection on demand"""
        if collection_name in self.collections:
            return self.collections[collection_name]
            
        if not self._init_client():
            return None
            
        try:
            collection = self.client.get_collection(collection_name)
            self.collections[collection_name] = collection
            return collection
        except Exception as e:
            logger.warning(f"Collection {collection_name} not available: {e}")
            return None
            
    def _get_target_collections(self, scope: str) -> List[str]:
        """Get collections to search based on scope"""
        if scope.lower() == "all":
            return ["stm32_hal", "motor_control", "project_code", "instruction_guides"]
        
        # Map scope to specific collection
        collection_name = self.scope_collections.get(scope.lower())
        if collection_name:
            return [collection_name]
            
        # Default to STM32H7 if unknown scope
        return ["stm32_hal"]
        
    def search(self, query: str, scope: str = "all", max_results: int = 10) -> Dict[str, Any]:
        """Execute optimized semantic search"""
        start_time = time.time()
        
        # Get target collections for scope
        target_collections = self._get_target_collections(scope)
        
        all_results = []
        
        for collection_name in target_collections:
            collection = self._load_collection(collection_name)
            if not collection:
                continue
                
            try:
                # Query collection
                results = collection.query(
                    query_texts=[query],
                    n_results=max_results,
                    include=["documents", "metadatas", "distances"]
                )
                
                # Check if results exist
                if (results and 'documents' in results and results['documents'] and 
                    len(results['documents']) > 0 and results['documents'][0]):
                    
                    # Format results
                    for doc, metadata, distance in zip(
                        results['documents'][0],
                        results['metadatas'][0], 
                        results['distances'][0]
                    ):
                        all_results.append({
                            'content': doc,
                            'source': metadata.get('source', 'unknown') if metadata else 'unknown',
                            'collection': collection_name,
                            'score': 1 - distance,  # Convert distance to similarity score
                            'metadata': metadata or {}
                        })
                    
            except Exception as e:
                logger.error(f"Query failed on {collection_name}: {e}")
                continue
                
        # Sort by score and limit results
        all_results.sort(key=lambda x: x['score'], reverse=True)
        final_results = all_results[:max_results]
        
        query_time = time.time() - start_time
        
        return {
            'query': query,
            'scope': scope,
            'results': final_results,
            'query_time': f"{query_time:.2f}s",
            'total_results': len(final_results),
            'collections_searched': target_collections
        }

def format_results(search_results: Dict[str, Any], show_content: bool = True):
    """Format search results for display"""
    results = search_results['results']
    
    print(f"\n🔍 Search: '{search_results['query']}' (scope: {search_results['scope']})")
    print(f"⏱️  Query time: {search_results['query_time']}")
    print(f"📚 Collections: {', '.join(search_results['collections_searched'])}")
    print(f"📄 Results: {search_results['total_results']}")
    print("=" * 80)
    
    if not results:
        print("No results found.")
        return
        
    for i, result in enumerate(results, 1):
        collection_emoji = {
            'stm32_hal': '🔧',
            'motor_control': '⚙️', 
            'project_code': '💻',
            'instruction_guides': '📖'
        }.get(result['collection'], '📄')
        
        print(f"\n{i}. {collection_emoji} [{result['collection']}] {result['source']}")
        print(f"   Score: {result['score']:.3f}")
        
        if show_content:
            content = result['content']
            if len(content) > 200:
                content = content[:200] + "..."
            print(f"   Content: {content}")
            
def main():
    """Command line interface"""
    parser = argparse.ArgumentParser(description="STM32H753ZI Semantic Search")
    
    parser.add_argument('query_type', choices=['concept', 'function', 'peripheral', 'register'],
                       help='Type of search query')
    parser.add_argument('query', help='Search query text')
    parser.add_argument('--scope', default='all', 
                       choices=['STM32H7', 'L6470', 'project', 'instructions', 'all'],
                       help='Search scope')
    parser.add_argument('--max-results', type=int, default=10,
                       help='Maximum number of results')
    parser.add_argument('--no-content', action='store_true',
                       help='Hide result content')
    parser.add_argument('--json', action='store_true',
                       help='Output results as JSON')
    
    args = parser.parse_args()
    
    # Build search query based on type
    if args.query_type == 'function':
        query = f"function {args.query} implementation usage example"
    elif args.query_type == 'peripheral':
        query = f"{args.query} peripheral configuration setup initialization"
    elif args.query_type == 'register':
        query = f"{args.query} register bit field configuration"
    else:  # concept
        query = args.query
        
    # Execute search
    searcher = OptimizedSemanticSearch()
    results = searcher.search(query, args.scope, args.max_results)
    
    if args.json:
        print(json.dumps(results, indent=2))
    else:
        format_results(results, show_content=not args.no_content)

if __name__ == "__main__":
    main()
