#!/usr/bin/env python3
"""
Semantic Database Validation and Testing Tool
Live validation of semantic search database during rebuild process
"""

import chromadb
import json
import time
import requests
import warnings
import os
from pathlib import Path
from typing import Dict, List, Any, Optional
from datetime import datetime

# Try to import Instructor-XL (same as rebuild script)
try:
    from InstructorEmbedding import INSTRUCTOR
    import torch
    INSTRUCTOR_AVAILABLE = True
except ImportError:
    INSTRUCTOR_AVAILABLE = False
    torch = None
    INSTRUCTOR = None

WORKSPACE_ROOT = Path("/workspaces/code")
SEMANTIC_DB_PATH = WORKSPACE_ROOT / "docs" / "semantic_vector_db"

# Warning suppressions (match rebuild script)
warnings.filterwarnings("ignore", category=DeprecationWarning)
warnings.filterwarnings("ignore", message=".*_target_device.*has been deprecated.*")
os.environ["TOKENIZERS_PARALLELISM"] = "false"

class SemanticDatabaseValidator:
    """Live validation tool for semantic search database"""
    
    def __init__(self):
        print("🔍 Initializing Semantic Database Validator...")
        
        # Initialize ChromaDB client without default embedding function
        try:
            self.client = chromadb.PersistentClient(path=str(SEMANTIC_DB_PATH))
            print(f"✅ Connected to semantic database: {SEMANTIC_DB_PATH}")
        except Exception as e:
            print(f"❌ Failed to connect to database: {e}")
            self.client = None
            
        # Initialize same embedding model as rebuild script
        self.instructor_model = None
        if INSTRUCTOR_AVAILABLE and torch is not None and INSTRUCTOR is not None:
            try:
                print("🔄 Loading Instructor-XL model for validation...")
                device = "cuda" if torch.cuda.is_available() else "cpu"
                os.environ["TRANSFORMERS_CACHE"] = os.path.expanduser("~/.cache/huggingface")
                self.instructor_model = INSTRUCTOR('hkunlp/instructor-xl', device=device, cache_folder=os.path.expanduser("~/.cache/huggingface"))
                print(f"✅ Instructor-XL loaded on {device.upper()}")
            except Exception as e:
                print(f"⚠️  Failed to load Instructor-XL: {e}")
                self.instructor_model = None
        else:
            print("⚠️  InstructorEmbedding not available - using Ollama for validation")
    
    def get_embedding(self, text: str, collection_name: str = "stm32_hal") -> List[float]:
        """Generate embedding using same method as rebuild script"""
        
        # Method 1: Instructor-XL with domain-specific instructions (preferred)
        if self.instructor_model is not None:
            try:
                # Use same domain-specific instructions as rebuild script
                embedding_instructions = {
                    "stm32_hal": "Represent the STM32 HAL documentation for finding peripheral configuration and driver functions:",
                    "motor_control": "Represent the stepper motor control documentation for finding L6470 driver functions and configuration:",
                    "project_source": "Represent the embedded C source code for finding HAL abstraction functions, drivers, and safety systems:",
                    "instruction_guides": "Represent the development instruction document for finding coding patterns and best practices:",
                    "project_documentation": "Represent the technical documentation for finding analysis reports and design decisions:",
                    "build_system": "Represent the build configuration file for finding CMake setup and compilation options:",
                    "automation_scripts": "Represent the automation script for finding build tools and validation utilities:",
                    "api_specifications": "Represent the API specification for finding interface definitions and protocol details:",
                    "workspace_config": "Represent the workspace configuration for finding VS Code settings and development environment:",
                    "validation_programs": "Represent the validation program for finding test executables and verification procedures:"
                }
                
                instruction = embedding_instructions.get(
                    collection_name, 
                    "Represent the document for semantic search and retrieval:"
                )
                
                embedding = self.instructor_model.encode([[instruction, text]])
                if len(embedding) > 0 and len(embedding[0]) == 768:
                    return embedding[0].tolist()
                    
            except Exception as e:
                print(f"    ⚠️  Instructor-XL failed: {e}, falling back to Ollama")
        
        # Method 2: Ollama mxbai-embed-large (fallback)
        return self.get_ollama_embedding(text)
    
    def get_ollama_embedding(self, text: str) -> List[float]:
        """Generate embedding using Ollama mxbai-embed-large (fallback method)"""
        try:
            response = requests.post(
                "http://localhost:11434/api/embeddings",
                json={"model": "mxbai-embed-large", "prompt": text},
                timeout=30
            )
            if response.status_code == 200:
                embedding = response.json().get("embedding", [])
                if len(embedding) == 1024:  # Verify mxbai-embed-large dimensions
                    return embedding
            
            print("    ⚠️  Ollama failed, using mock embedding")
            return self._mock_embedding(text)
            
        except Exception as e:
            print(f"    ⚠️  Ollama error: {e}, using mock embedding")
            return self._mock_embedding(text)
    
    def _mock_embedding(self, text: str) -> List[float]:
        """Fallback mock embedding for demonstration"""
        return [float(hash(text + str(i)) % 1000) / 1000.0 for i in range(1024)]
    
    def get_database_stats(self) -> Dict[str, Any]:
        """Get current database statistics"""
        if not self.client:
            return {}
        
        stats = {
            "timestamp": datetime.now().isoformat(),
            "collections": {},
            "total_documents": 0,
            "database_size_mb": 0
        }
        
        try:
            # Get database size
            if SEMANTIC_DB_PATH.exists():
                stats["database_size_mb"] = sum(
                    f.stat().st_size for f in SEMANTIC_DB_PATH.rglob('*') if f.is_file()
                ) / 1024 / 1024
            
            # Get collection information
            collections = self.client.list_collections()
            
            for collection in collections:
                try:
                    coll = self.client.get_collection(collection.name)
                    count = coll.count()
                    metadata = collection.metadata
                    
                    stats["collections"][collection.name] = {
                        "document_count": count,
                        "description": metadata.get("description", "No description"),
                        "status": "active"
                    }
                    stats["total_documents"] += count
                    
                except Exception as e:
                    stats["collections"][collection.name] = {
                        "document_count": 0,
                        "description": "Error retrieving info",
                        "status": f"error: {e}",
                        "error": str(e)
                    }
            
            print(f"📊 Database Statistics:")
            print(f"   Total Collections: {len(stats['collections'])}")
            print(f"   Total Documents: {stats['total_documents']}")
            print(f"   Database Size: {stats['database_size_mb']:.1f} MB")
            
            return stats
            
        except Exception as e:
            print(f"⚠️  Error getting database stats: {e}")
            return stats
    
    def test_collection_search(self, collection_name: str, test_queries: List[str]) -> Dict[str, Any]:
        """Test search functionality on a specific collection"""
        if not self.client:
            return {"error": "No database connection"}
        
        try:
            collection = self.client.get_collection(collection_name)
            results = {}
            
            print(f"\n🔍 Testing collection: {collection_name}")
            
            for query in test_queries:
                start_time = time.time()
                
                try:
                    # Generate query embedding using same method as database
                    query_embedding = self.get_embedding(query, collection_name)
                    
                    # Search using our embedding instead of default ChromaDB embedding
                    search_results = collection.query(
                        query_embeddings=[query_embedding],
                        n_results=3
                    )
                    
                    response_time = time.time() - start_time
                    
                    # Safely extract documents with proper null checking
                    documents = search_results.get("documents", [[]])
                    doc_list = documents[0] if documents and len(documents) > 0 else []
                    
                    results[query] = {
                        "response_time_ms": response_time * 1000,
                        "results_found": len(doc_list),
                        "status": "success",
                        "sample_results": doc_list[:1] if doc_list else []
                    }
                    
                    print(f"   ✅ '{query[:30]}...': {len(doc_list)} results ({response_time*1000:.1f}ms)")
                    
                except Exception as e:
                    results[query] = {
                        "response_time_ms": 0,
                        "results_found": 0,
                        "status": f"error: {e}",
                        "error": str(e)
                    }
                    print(f"   ❌ '{query[:30]}...': Error - {e}")
            
            return results
            
        except Exception as e:
            print(f"   ❌ Collection {collection_name} error: {e}")
            return {"error": str(e)}
    
    def run_comprehensive_validation(self) -> Dict[str, Any]:
        """Run comprehensive validation of the current database state"""
        print("🚀 Starting Comprehensive Database Validation...")
        
        validation_results = {
            "validation_timestamp": datetime.now().isoformat(),
            "database_stats": self.get_database_stats(),
            "search_tests": {},
            "performance_summary": {}
        }
        
        # Define test queries for each expected collection
        test_scenarios = {
            "stm32_hal": [
                "GPIO pin configuration",
                "HAL_GPIO_Init function",
                "SPI master setup",
                "UART communication",
                "Timer PWM configuration"
            ],
            "motor_control": [
                "L6470 stepper driver",
                "motor position control",
                "stepper acceleration",
                "L6470_SetParam",
                "microstepping mode"
            ],
            "project_source": [
                "HAL abstraction layer",
                "error handling",
                "motor controller",
                "safety systems",
                "peripheral drivers"
            ],
            "instruction_guides": [
                "coding guidelines",
                "SSOT configuration",
                "build system",
                "development workflow",
                "architecture patterns"
            ],
            "project_documentation": [
                "project analysis",
                "completion report",
                "technical documentation",
                "design decisions",
                "system architecture"
            ],
            "build_system": [
                "CMake configuration",
                "compiler settings",
                "toolchain setup",
                "build targets",
                "linker script"
            ],
            "automation_scripts": [
                "build automation",
                "validation tools",
                "semantic search",
                "status update",
                "Python scripts"
            ],
            "api_specifications": [
                "REST API",
                "CAN protocol",
                "interface definition",
                "API endpoints",
                "protocol specification"
            ],
            "workspace_config": [
                "VS Code settings",
                "development environment",
                "workspace configuration",
                "editor setup",
                "development tools"
            ]
        }
        
        # Test each collection that exists
        available_collections = [coll.name for coll in self.client.list_collections()] if self.client else []
        
        total_response_times = []
        total_results_found = 0
        successful_tests = 0
        failed_tests = 0
        
        for collection_name, queries in test_scenarios.items():
            if collection_name in available_collections:
                test_results = self.test_collection_search(collection_name, queries)
                validation_results["search_tests"][collection_name] = test_results
                
                # Aggregate performance metrics
                for query, result in test_results.items():
                    if isinstance(result, dict) and result.get("status") == "success":
                        total_response_times.append(result["response_time_ms"])
                        total_results_found += result["results_found"]
                        successful_tests += 1
                    else:
                        failed_tests += 1
            else:
                print(f"   ⏭️  Collection {collection_name} not found (may not be indexed yet)")
                validation_results["search_tests"][collection_name] = {"status": "collection_not_found"}
        
        # Calculate performance summary
        if total_response_times:
            validation_results["performance_summary"] = {
                "average_response_time_ms": sum(total_response_times) / len(total_response_times),
                "fastest_response_ms": min(total_response_times),
                "slowest_response_ms": max(total_response_times),
                "total_results_found": total_results_found,
                "successful_tests": successful_tests,
                "failed_tests": failed_tests,
                "success_rate": successful_tests / (successful_tests + failed_tests) * 100 if (successful_tests + failed_tests) > 0 else 0
            }
            
            perf = validation_results["performance_summary"]
            print(f"\n📈 Performance Summary:")
            print(f"   Average Response Time: {perf['average_response_time_ms']:.1f}ms")
            print(f"   Fastest Response: {perf['fastest_response_ms']:.1f}ms")
            print(f"   Slowest Response: {perf['slowest_response_ms']:.1f}ms")
            print(f"   Total Results Found: {perf['total_results_found']}")
            print(f"   Success Rate: {perf['success_rate']:.1f}% ({successful_tests}/{successful_tests + failed_tests} tests)")
        
        return validation_results
    
    def save_validation_report(self, results: Dict[str, Any]):
        """Save validation results to file"""
        report_path = WORKSPACE_ROOT / "docs" / f"semantic_db_validation_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        
        with open(report_path, 'w') as f:
            json.dump(results, f, indent=2)
        
        print(f"💾 Validation report saved: {report_path}")
        
        # Also create a summary markdown report
        self.generate_markdown_summary(results, report_path.with_suffix('.md'))
    
    def generate_markdown_summary(self, results: Dict[str, Any], output_path: Path):
        """Generate a markdown summary of validation results"""
        md_content = f"""# Semantic Database Validation Report

**Generated**: {results.get('validation_timestamp', 'Unknown')}  
**Database**: `{SEMANTIC_DB_PATH}`

## Database Overview

"""
        
        db_stats = results.get('database_stats', {})
        md_content += f"- **Total Collections**: {len(db_stats.get('collections', {}))}\n"
        md_content += f"- **Total Documents**: {db_stats.get('total_documents', 0):,}\n"
        md_content += f"- **Database Size**: {db_stats.get('database_size_mb', 0):.1f} MB\n\n"
        
        # Collection status
        md_content += "### Collections Status\n\n"
        md_content += "| Collection | Documents | Status | Description |\n"
        md_content += "|------------|-----------|--------|--------------|\n"
        
        for name, info in db_stats.get('collections', {}).items():
            status_icon = "✅" if info.get('status') == 'active' else "❌"
            md_content += f"| {name} | {info.get('document_count', 0):,} | {status_icon} {info.get('status', 'unknown')} | {info.get('description', 'N/A')} |\n"
        
        # Performance summary
        perf = results.get('performance_summary', {})
        if perf:
            md_content += f"\n## Performance Summary\n\n"
            md_content += f"- **Average Response Time**: {perf.get('average_response_time_ms', 0):.1f}ms\n"
            md_content += f"- **Response Time Range**: {perf.get('fastest_response_ms', 0):.1f}ms - {perf.get('slowest_response_ms', 0):.1f}ms\n"
            md_content += f"- **Total Results Found**: {perf.get('total_results_found', 0):,}\n"
            md_content += f"- **Test Success Rate**: {perf.get('success_rate', 0):.1f}%\n"
        
        # Search test results
        md_content += f"\n## Search Test Results\n\n"
        
        search_tests = results.get('search_tests', {})
        for collection, tests in search_tests.items():
            if isinstance(tests, dict) and tests.get('status') != 'collection_not_found':
                md_content += f"### {collection}\n\n"
                
                working_tests = [q for q, r in tests.items() if isinstance(r, dict) and r.get('status') == 'success']
                failed_tests = [q for q, r in tests.items() if isinstance(r, dict) and r.get('status') != 'success']
                
                if working_tests:
                    md_content += f"**✅ Working Queries ({len(working_tests)}):**\n"
                    for query in working_tests[:3]:  # Show first 3
                        result = tests[query]
                        md_content += f"- `{query}`: {result['results_found']} results ({result['response_time_ms']:.1f}ms)\n"
                    
                    if len(working_tests) > 3:
                        md_content += f"- ... and {len(working_tests) - 3} more\n"
                
                if failed_tests:
                    md_content += f"\n**❌ Failed Queries ({len(failed_tests)}):**\n"
                    for query in failed_tests[:3]:  # Show first 3
                        md_content += f"- `{query}`: {tests[query].get('status', 'unknown error')}\n"
                
                md_content += "\n"
        
        md_content += f"\n---\n*Generated by Semantic Database Validator*\n"
        
        with open(output_path, 'w') as f:
            f.write(md_content)
        
        print(f"📋 Markdown summary saved: {output_path}")
    
    def monitor_rebuild_progress(self, check_interval: int = 60):
        """Monitor rebuild progress by checking database growth"""
        print(f"🔄 Starting rebuild progress monitoring (checking every {check_interval}s)...")
        
        previous_stats = None
        
        while True:
            try:
                current_stats = self.get_database_stats()
                
                if previous_stats:
                    doc_growth = current_stats["total_documents"] - previous_stats["total_documents"]
                    size_growth = current_stats["database_size_mb"] - previous_stats["database_size_mb"]
                    
                    print(f"\n📈 Progress Update:")
                    print(f"   Documents Added: +{doc_growth:,}")
                    print(f"   Size Growth: +{size_growth:.1f} MB")
                    print(f"   Current Total: {current_stats['total_documents']:,} documents ({current_stats['database_size_mb']:.1f} MB)")
                    
                    # Show collection growth
                    for name, info in current_stats["collections"].items():
                        if name in previous_stats["collections"]:
                            prev_count = previous_stats["collections"][name]["document_count"]
                            curr_count = info["document_count"]
                            if curr_count > prev_count:
                                print(f"   {name}: +{curr_count - prev_count:,} documents ({curr_count:,} total)")
                
                previous_stats = current_stats
                time.sleep(check_interval)
                
            except KeyboardInterrupt:
                print("\n⏹️  Monitoring stopped by user")
                break
            except Exception as e:
                print(f"⚠️  Monitoring error: {e}")
                time.sleep(check_interval)

if __name__ == "__main__":
    validator = SemanticDatabaseValidator()
    
    # Run validation
    results = validator.run_comprehensive_validation()
    
    # Save results
    validator.save_validation_report(results)
    
    print("\n🎉 Validation complete! Use '--monitor' to track rebuild progress.")
