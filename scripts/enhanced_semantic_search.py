#!/usr/bin/env python3
"""
Enhanced Semantic Search with Comprehensive Workspace Coverage
Tests the new GPU-accelerated, instruction-tuned semantic search system
"""

import chromadb
import requests
from pathlib import Path
import time
import json

WORKSPACE_ROOT = Path("/workspaces/code")
SEMANTIC_DB_PATH = WORKSPACE_ROOT / "docs" / "semantic_vector_db"

class EnhancedSemanticSearch:
    """Enhanced semantic search with comprehensive workspace coverage"""
    
    def __init__(self):
        self.client = chromadb.PersistentClient(path=str(SEMANTIC_DB_PATH))
        
        # Load Instructor-XL for query embeddings (if available)
        self.instructor_model = None
        try:
            from InstructorEmbedding import INSTRUCTOR
            import torch
            device = "cuda" if torch.cuda.is_available() else "cpu"
            self.instructor_model = INSTRUCTOR('hkunlp/instructor-xl', device=device)
            print(f"✅ Query embedding model loaded on {device.upper()}")
        except:
            print("⚠️  Using ChromaDB default embeddings for queries")
    
    def get_collection_stats(self):
        """Show comprehensive database statistics"""
        print("\n📊 COMPREHENSIVE DATABASE STATISTICS:")
        collections = self.client.list_collections()
        total_docs = 0
        stats = {}
        
        for collection in collections:
            count = collection.count()
            total_docs += count
            stats[collection.name] = count
            print(f"  {collection.name}: {count:,} documents")
        
        print(f"\n🎯 Total indexed: {total_docs:,} documents")
        improvement = round(total_docs / 981, 1) if total_docs > 981 else 1.0
        print(f"📈 Improvement: {improvement}x over original 981 documents")
        
        return stats, total_docs
    
    def search_collection(self, query, collection_name, n_results=5):
        """Search a specific collection with enhanced context"""
        try:
            collection = self.client.get_collection(collection_name)
            
            # Generate query embedding if Instructor-XL available
            query_embedding = None
            if self.instructor_model:
                try:
                    # Use appropriate instruction for query
                    instructions = {
                        "stm32_hal": "Represent the STM32 HAL search query for finding peripheral configuration:",
                        "motor_control": "Represent the stepper motor search query for finding L6470 functions:",
                        "project_source": "Represent the embedded C code search query for finding HAL functions:",
                        "instruction_guides": "Represent the development instruction search query:",
                        "project_documentation": "Represent the technical documentation search query:",
                        "build_system": "Represent the build configuration search query:",
                        "automation_scripts": "Represent the automation script search query:",
                        "api_specifications": "Represent the API specification search query:",
                    }
                    
                    instruction = instructions.get(collection_name, "Represent the search query:")
                    embedding = self.instructor_model.encode([[instruction, query]])
                    query_embedding = embedding[0].tolist()
                except Exception as e:
                    print(f"    ⚠️ Query embedding failed: {e}")
            
            # Perform search
            if query_embedding:
                results = collection.query(
                    query_embeddings=[query_embedding],
                    n_results=n_results,
                    include=['documents', 'metadatas', 'distances']
                )
            else:
                results = collection.query(
                    query_texts=[query],
                    n_results=n_results,
                    include=['documents', 'metadatas', 'distances']
                )
            
            return results
            
        except Exception as e:
            print(f"❌ Error searching {collection_name}: {e}")
            return None
    
    def comprehensive_search(self, query, max_results_per_collection=3):
        """Search across all collections with intelligent ranking"""
        print(f"\n🔍 COMPREHENSIVE SEARCH: '{query}'")
        print("=" * 60)
        
        all_results = []
        collections = self.client.list_collections()
        
        for collection in collections:
            print(f"\n📂 Searching {collection.name}...")
            results = self.search_collection(query, collection.name, max_results_per_collection)
            
            if results and results['documents'][0]:
                for i, (doc, meta, distance) in enumerate(zip(
                    results['documents'][0],
                    results['metadatas'][0], 
                    results['distances'][0]
                )):
                    all_results.append({
                        'collection': collection.name,
                        'document': doc[:200] + "..." if len(doc) > 200 else doc,
                        'metadata': meta,
                        'distance': distance,
                        'rank': i + 1
                    })
                    
                print(f"  Found {len(results['documents'][0])} results")
            else:
                print(f"  No results found")
        
        # Sort by distance (lower is better)
        all_results.sort(key=lambda x: x['distance'])
        
        print(f"\n🏆 TOP RESULTS ACROSS ALL COLLECTIONS:")
        print("-" * 60)
        
        for i, result in enumerate(all_results[:10]):  # Top 10 overall
            print(f"\n{i+1}. [{result['collection']}] (similarity: {1-result['distance']:.3f})")
            print(f"   File: {result['metadata'].get('source_file', 'Unknown')}")
            print(f"   Context: {result['metadata'].get('context', 'N/A')}")
            print(f"   Content: {result['document']}")
        
        return all_results
    
    def test_search_scenarios(self):
        """Test various search scenarios to validate the comprehensive database"""
        
        print("\n🧪 TESTING COMPREHENSIVE SEMANTIC SEARCH")
        print("=" * 70)
        
        test_scenarios = [
            # STM32 HAL searches
            ("GPIO configuration", "Should find STM32 HAL GPIO setup examples"),
            ("SPI master setup", "Should find SPI peripheral configuration"),
            ("UART communication", "Should find UART driver examples"),
            
            # Motor control searches  
            ("L6470 stepper driver", "Should find L6470 motor driver functions"),
            ("stepper motor initialization", "Should find motor setup procedures"),
            
            # Project source code searches
            ("HAL abstraction", "Should find our HAL abstraction layer"),
            ("safety system", "Should find safety-related source code"),
            ("error handling", "Should find error management patterns"),
            
            # Development instructions
            ("coding patterns", "Should find instruction guides"),
            ("build configuration", "Should find CMake and build setup"),
            
            # Documentation searches
            ("project completion", "Should find analysis and completion reports"),
            ("API specification", "Should find REST and CAN API docs"),
        ]
        
        for query, expected in test_scenarios:
            print(f"\n🎯 Test: '{query}'")
            print(f"   Expected: {expected}")
            
            start_time = time.time()
            results = self.comprehensive_search(query, max_results_per_collection=2)
            search_time = time.time() - start_time
            
            print(f"   ⏱️  Search time: {search_time:.3f}s")
            print(f"   📊 Total results: {len(results)}")
            
            if results:
                top_result = results[0]
                print(f"   🏆 Best match: [{top_result['collection']}] {top_result['metadata'].get('source_file', 'Unknown')}")
                print(f"   📈 Similarity: {1-top_result['distance']:.3f}")
            
            print("-" * 50)
    
    def quick_search(self, query, collection=None):
        """Quick search interface for interactive use"""
        if collection:
            results = self.search_collection(query, collection, n_results=5)
            if results and results['documents'][0]:
                print(f"\n🔍 Results from {collection}:")
                for i, (doc, meta, distance) in enumerate(zip(
                    results['documents'][0][:3],  # Top 3
                    results['metadatas'][0][:3],
                    results['distances'][0][:3]
                )):
                    print(f"\n{i+1}. Similarity: {1-distance:.3f}")
                    print(f"   File: {meta.get('source_file', 'Unknown')}")
                    print(f"   Content: {doc[:150]}...")
        else:
            return self.comprehensive_search(query, max_results_per_collection=2)

def main():
    """Main testing and demonstration"""
    searcher = EnhancedSemanticSearch()
    
    # Show database statistics
    stats, total_docs = searcher.get_collection_stats()
    
    if total_docs < 2000:
        print(f"\n⚠️  Database appears incomplete ({total_docs} docs)")
        print("   Waiting for rebuild to complete...")
        return
    
    print(f"\n🎉 Database ready with {total_docs:,} documents!")
    
    # Run comprehensive tests
    searcher.test_search_scenarios()
    
    print(f"\n💡 INTERACTIVE USAGE:")
    print(f"   from scripts.enhanced_semantic_search import EnhancedSemanticSearch")
    print(f"   searcher = EnhancedSemanticSearch()")
    print(f"   searcher.quick_search('your query here')")
    print(f"   searcher.quick_search('GPIO init', 'stm32_hal')")

if __name__ == "__main__":
    main()
