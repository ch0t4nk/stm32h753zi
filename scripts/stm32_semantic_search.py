#!/usr/bin/env python3
"""
STM32 Semantic Search - Production Integration
Replaces legacy search_enhanced_docs.py with semantic         # Check if database is             if total_docs == 0 and self.auto_rebuild:
                print("� Empty database detected, rebuilding...")
                self._process_workspace_docs()
            else:
                print(f"📊 Database ready: {total_docs} documents across {len(stats)} collections")
                # Cache collection references for faster access
                self._cached_collections = {}
                for collection_name in stats.keys():
                    try:
                        self._cached_collections[collection_name] = self.vector_db.client.get_collection(collection_name)
                        print(f"   ✅ {collection_name}: {stats[collection_name]['document_count']} docs")
                    except Exception as e:
                        print(f"   ⚠️  Failed to cache {collection_name}: {e}")
                print("⚡ Collections cached for optimal query performance")able and has data
        if not hasattr(self, '_vector_db') or self._vector_db is None:
            print("🔧 Initializing vector database...")
            self._vector_db = STM32VectorDatabase()
            stats = self.vector_db.get_collection_stats()
            total_docs = sum(stat["document_count"] for stat in stats.values())
            if total_docs == 0 and self.auto_rebuild:
                print("📥 No documents found. Rebuilding database...")
                self._vector_db.initialize_database()
                print(f"✅ Database rebuilt with documents")
            else:
                print(f"📊 Using existing database: {total_docs} documents")
        
        STM32WorkspaceSearcher._initialized = Truees
Integrates with existing workspace documentation
"""

from semantic_search import STM32SemanticSearch, SearchResult
from chunk_stm32_docs import STM32DocumentChunker
from init_vector_db import STM32VectorDatabase, CollectionType
import sys
import argparse
import requests
from pathlib import Path
from typing import List, Dict, Optional

# Add scripts directory to path
sys.path.insert(0, str(Path(__file__).parent))

# Import our semantic search modules

# SSOT Documentation Configuration
WORKSPACE_ROOT = Path("/workspaces/code")
SEMANTIC_DB_PATH = WORKSPACE_ROOT / "docs" / "semantic_vector_db"
REFERENCE_DOCS_PATH = WORKSPACE_ROOT / "00_reference"

# Legacy index paths (for migration reference)
DOC_INDEX_STM32H7_FULL = (
    WORKSPACE_ROOT / "docs" / "indexes" / "STM32H7_FULL_INDEX.json"
)
DOC_INDEX_L6470_SEARCH = (
    WORKSPACE_ROOT / "docs" / "indexes" / "L6470_SEARCH_INDEX.json"
)
DOC_INDEX_NUCLEO_BSP = (
    WORKSPACE_ROOT / "docs" / "indexes" / "STM32H7xx_Nucleo_BSP_INDEX.json"
)


class OllamaEmbedding:
    """Real embedding generator using Ollama mxbai-embed-large"""

    def __init__(
        self, model="mxbai-embed-large", url="http://localhost:11434"
    ):
        self.model = model
        self.url = url
        self.api_url = f"{url}/api/embeddings"
        self._test_connection()

    def _test_connection(self):
        """Test Ollama connection and model availability"""
        try:
            response = requests.get(f"{self.url}/api/tags", timeout=5)
            if response.status_code == 200:
                models = response.json().get("models", [])
                model_names = [m["name"] for m in models]
                if any(self.model in name for name in model_names):
                    print(f"✅ Ollama connected: {self.model} available")
                    self.connected = True
                    return True
                else:
                    print(
                        f"⚠️  Model {self.model} not found. Available: {model_names}"
                    )
            else:
                print(f"⚠️  Ollama API error: {response.status_code}")
        except Exception as e:
            print(f"⚠️  Ollama connection failed: {e}")

        print("   Using mock embeddings for demonstration")
        self.connected = False
        return False

    def generate_embedding(self, text: str) -> List[float]:
        """Generate real embedding using Ollama API with fallback"""
        if not hasattr(self, "connected") or not self.connected:
            return self._mock_embedding(text)

        try:
            payload = {"model": self.model, "prompt": text}
            response = requests.post(self.api_url, json=payload, timeout=30)

            if response.status_code == 200:
                result = response.json()
                return result.get("embedding", self._mock_embedding(text))
            else:
                print(
                    f"🔄 Ollama API error {response.status_code}, using mock"
                )
                return self._mock_embedding(text)

        except Exception as e:
            print(f"🔄 Ollama error: {e}, using mock")
            return self._mock_embedding(text)

    def _mock_embedding(self, text: str) -> List[float]:
        """Fallback mock embedding"""
        return [
            float(hash(text + str(i)) % 1000) / 1000.0 for i in range(1024)
        ]


class STM32WorkspaceSearcher:
    """Enhanced STM32 workspace searcher with semantic capabilities"""
    
    _instance = None
    _initialized = False
    
    def __new__(cls, auto_rebuild=True):
        if cls._instance is None:
            cls._instance = super(STM32WorkspaceSearcher, cls).__new__(cls)
        return cls._instance
    
    def __init__(self, auto_rebuild=True):
        # Only initialize once
        if STM32WorkspaceSearcher._initialized:
            return
            
        self.workspace_root = Path("/workspaces/code")
        self.vector_db_path = self.workspace_root / "docs" / "semantic_vector_db"
        self.auto_rebuild = auto_rebuild
        
        # Initialize embedder and chunker first
        self.searcher = STM32SemanticSearch()
        
        # Add direct access to chunker for performance
        self.chunker = self.searcher.chunker if hasattr(self.searcher, 'chunker') else STM32DocumentChunker()
        
        # Initialize vector database with persistent connection
        self.vector_db = STM32VectorDatabase(str(self.vector_db_path))
        
        # Initialize database connection and load existing collections
        print("🔗 Initializing database connection...")
        if not self.vector_db.initialize_database():
            if self.auto_rebuild:
                print("⚠️  Database initialization failed, rebuilding...")
                self._process_workspace_docs()
                return
            else:
                raise Exception("Failed to initialize vector database")
        
        # Pre-load all collections for faster queries (performance optimization)
        print("⚡ Pre-loading collections for optimal performance...")
        try:
            stats = self.vector_db.get_collection_stats()
            total_docs = sum(stat["document_count"] for stat in stats.values())
            if total_docs == 0 and self.auto_rebuild:
                print("� Empty database detected, rebuilding...")
                self._process_workspace_docs()
            else:
                print(f"📊 Using existing database: {total_docs} documents")
        except Exception as e:
            if self.auto_rebuild:
                print(f"⚠️  Database error: {e}, rebuilding...")
                self._process_workspace_docs()
            else:
                print(f"❌ Database error: {e}")
                raise
        
        STM32WorkspaceSearcher._initialized = True
        print("✅ STM32WorkspaceSearcher initialization complete")

    def _search_cached_collection(self, collection_name: str, query_embedding: List[float], n_results: int = 10):
        """Fast search using cached collection reference"""
        if not hasattr(self, '_cached_collections') or collection_name not in self._cached_collections:
            # Fallback to regular search if cache not available
            print(f"⚠️  Collection {collection_name} not cached, using fallback")
            collection_type = CollectionType(collection_name)
            return self.vector_db.search_documents(collection_type, query_embedding, n_results)
        
        try:
            # Use cached collection for direct query (much faster)
            collection = self._cached_collections[collection_name]
            results = collection.query(
                query_embeddings=[query_embedding],
                n_results=n_results,
                include=["documents", "metadatas", "distances"]
            )
            
            # Convert to the same format as search_documents
            if results and "documents" in results and len(results["documents"]) > 0:
                return {
                    "documents": results["documents"][0],  # ChromaDB returns list of lists
                    "metadatas": results["metadatas"][0] if "metadatas" in results else [],
                    "distances": results["distances"][0] if "distances" in results else []
                }
            return None
            
        except Exception as e:
            print(f"   ⚠️  Error in cached search for {collection_name}: {e}")
            # Fallback to regular search
            collection_type = CollectionType(collection_name)
            return self.vector_db.search_documents(collection_type, query_embedding, n_results)

    def _process_workspace_docs(self):
        """Process actual workspace documentation"""
        print("🔍 Processing STM32H753ZI workspace documentation...")

        # Define key documentation sources for semantic processing
        doc_sources = [
            {
                "path": REFERENCE_DOCS_PATH / "stm32h753xx_User_Manual_md",
                "collection": CollectionType.STM32_HAL,
                "pattern": "*.md",
                "category": "stm32h7_reference",
            },
            {
                "path": REFERENCE_DOCS_PATH / "x_cube_spn2_markdown_docs",
                "collection": CollectionType.MOTOR_CONTROL,
                "pattern": "*.md",
                "category": "l6470_spn2",
            },
            {
                "path": REFERENCE_DOCS_PATH / "stm32h7xx_nucleo_bsp_md",
                "collection": CollectionType.STM32_HAL,
                "pattern": "*.md",
                "category": "nucleo_bsp",
            },
            # Add project source code
            {
                "path": WORKSPACE_ROOT / "src",
                "collection": CollectionType.PROJECT_CODE,
                "pattern": "**/*.{c,h}",
                "category": "project_source",
            },
            {
                "path": WORKSPACE_ROOT / ".github" / "instructions",
                "collection": CollectionType.INSTRUCTION_GUIDES,
                "pattern": "*.md",
                "category": "development_instructions",
            },
            {
                "path": WORKSPACE_ROOT / "docs",
                "collection": CollectionType.INSTRUCTION_GUIDES,
                "pattern": "**/*.md",
                "category": "project_documentation",
            },
            # Add build system files
            {
                "path": WORKSPACE_ROOT / "cmake",
                "collection": CollectionType.PROJECT_CODE,
                "pattern": "*.cmake",
                "category": "build_system",
            },
            # Add testing files
            {
                "path": WORKSPACE_ROOT / "tests",
                "collection": CollectionType.SAFETY_SYSTEMS,
                "pattern": "**/*.{c,h}",
                "category": "testing_validation",
            },
        ]

        # Add individual reference documents
        reference_files = [
            {
                "file": "RM0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.md",  # noqa: E501
                "collection": CollectionType.STM32_HAL,
                "category": "stm32h7_reference",
            },
            {
                "file": "STM32H72x_73x_74x_75x_system_architecture_overview.md",
                "collection": CollectionType.STM32_HAL,
                "category": "stm32h7_architecture",
            },
        ]

        total_processed = 0

        # Process directory sources
        for source in doc_sources:
            if source["path"].exists():
                total_processed += self._process_doc_directory(source)

        # Process individual files
        for ref_file in reference_files:
            file_path = REFERENCE_DOCS_PATH / ref_file["file"]
            if file_path.exists():
                total_processed += self._process_single_file(
                    file_path, ref_file["collection"], ref_file["category"]
                )

        print(
            f"✅ Processed {total_processed} documents into semantic database"
        )

    def _process_doc_directory(self, source: Dict) -> int:
        """Process a directory of documentation files"""
        print(f"   📁 Processing {source['path'].name}...")

        processed = 0
        
        # Handle different pattern types
        pattern = source["pattern"]
        if "**/" in pattern:
            # Recursive pattern
            if pattern.startswith("**/"):
                pattern = pattern[3:]  # Remove **/ prefix
            files = list(source["path"].rglob(pattern))
        elif "{" in pattern:
            # Multiple extensions pattern like *.{c,h}
            base_pattern = pattern.split("{")[0]  # Get base like "*."
            extensions = pattern.split("{")[1].split("}")[0].split(",")  # Get extensions
            files = []
            for ext in extensions:
                files.extend(source["path"].glob(f"{base_pattern}{ext}"))
        else:
            # Simple pattern
            files = list(source["path"].glob(pattern))

        for file_path in files:
            try:
                processed += self._process_single_file(
                    file_path, source["collection"], source["category"]
                )
            except Exception as e:
                print(f"      ⚠️  Failed to process {file_path.name}: {e}")

        print(
            f"      ✅ Processed {processed} files from {source['path'].name}"
        )
        return processed

    def _process_single_file(
        self, file_path: Path, collection: CollectionType, category: str
    ) -> int:
        """Process a single documentation file"""
        try:
            # Read file content
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()

            if len(content.strip()) < 100:  # Skip very small files
                return 0

            # Chunk the document
            # Process and chunk the document
            chunks = self.chunker.chunk_document(content, str(file_path.name))

            if not chunks:
                return 0

            # Process chunks in batches
            batch_size = 50
            total_added = 0

            for i in range(0, len(chunks), batch_size):
                batch_chunks = chunks[i : i + batch_size]

                # Generate embeddings
                embeddings = []
                contents = []
                metadata_list = []
                ids = []

                for j, chunk in enumerate(batch_chunks):
                    embedding = self.searcher.generate_query_embedding(chunk.content)

                    metadata = {
                        "source_file": file_path.name,
                        "chunk_index": i + j,
                        "chunk_type": chunk.chunk_type.value,
                        "content_length": len(chunk.content),
                        "category": category,
                        "line_start": chunk.metadata.get("line_start", 0),
                        "line_end": chunk.metadata.get("line_end", 0),
                    }

                    # Convert lists to strings for ChromaDB
                    if "keywords" in chunk.metadata and isinstance(
                        chunk.metadata["keywords"], list
                    ):
                        metadata["keywords"] = ", ".join(
                            chunk.metadata["keywords"]
                        )

                    embeddings.append(embedding)
                    contents.append(chunk.content)
                    metadata_list.append(metadata)
                    ids.append(f"{file_path.stem}_chunk_{i + j}")

                # Add to database
                if self.vector_db.add_documents(
                    collection, contents, embeddings, metadata_list, ids
                ):
                    total_added += len(batch_chunks)

            if total_added > 0:
                print(f"      📝 {file_path.name}: {total_added} chunks")

            return 1  # Return 1 file processed

        except Exception as e:
            print(f"      ❌ Error processing {file_path.name}: {e}")
            return 0

    def search(
        self, query: str, category: str = "all", max_results: int = 5
    ) -> List[SearchResult]:
        """Perform semantic search"""
        print(f"🔍 Semantic search: '{query}' (category: {category})")

        # Generate query embedding
        query_embedding = self.searcher.generate_query_embedding(query)
        if query_embedding is None:
            print("❌ Failed to generate query embedding")
            return []

        # Map category to collections
        collection_mapping = {
            "hal": [CollectionType.STM32_HAL],
            "motor": [CollectionType.MOTOR_CONTROL],
            "safety": [CollectionType.SAFETY_SYSTEMS],
            "code": [CollectionType.PROJECT_CODE],
            "instructions": [CollectionType.INSTRUCTION_GUIDES],
            "all": [
                CollectionType.STM32_HAL,
                CollectionType.MOTOR_CONTROL,
                CollectionType.SAFETY_SYSTEMS,
                CollectionType.PROJECT_CODE,
                CollectionType.INSTRUCTION_GUIDES,
            ],
        }

        collections_to_search = collection_mapping.get(
            category, collection_mapping["all"]
        )

        # Search each collection directly using cached collections for better performance
        all_results = []
        for collection in collections_to_search:
            try:
                # Use cached collection search for better performance
                print(f"   🔍 Searching {collection.value}...")
                results = self._search_cached_collection(
                    collection.value, query_embedding, max_results
                )

                # Convert ChromaDB results to SearchResult objects
                if results and "documents" in results:
                    for i, doc in enumerate(results["documents"]):
                        if i < len(results.get("distances", [])):
                            # Convert distance to similarity (1 - distance)
                            similarity = 1.0 - results["distances"][i]

                            metadata = (
                                results.get("metadatas", [{}])[i]
                                if i < len(results.get("metadatas", []))
                                else {}
                            )

                            search_result = SearchResult(
                                content=doc,
                                source_file=metadata.get(
                                    "source_file", "unknown"
                                ),
                                chunk_type=metadata.get("chunk_type", "text"),
                                similarity_score=similarity,
                                metadata=metadata,
                                collection=collection.value,
                            )
                            all_results.append(search_result)

            except Exception as e:
                print(f"   ⚠️  Error searching {collection.value}: {e}")

        # Sort by similarity score and limit results
        all_results.sort(key=lambda x: x.similarity_score, reverse=True)
        final_results = all_results[:max_results]

        if final_results:
            print(f"✅ Found {len(final_results)} results")
            for i, result in enumerate(final_results, 1):
                similarity = result.similarity_score
                print(
                    f"   {i}. {result.source_file} "
                    f"(similarity: {similarity:.3f})"
                )
                print(f"      {result.content[:80]}...")
        else:
            print("⚠️  No results found")

        return final_results


def main():
    """Main CLI interface"""
    parser = argparse.ArgumentParser(
        description="STM32 Semantic Search - Production System"
    )
    parser.add_argument(
        "search_type",
        choices=["concept", "function", "peripheral", "register", "test"],
        help="Type of search to perform",
    )
    parser.add_argument("query", help="Search query")
    parser.add_argument(
        "--scope",
        choices=["STM32H7", "L6470", "NUCLEO_BSP", "all"],
        default="all",
        help="Search scope",
    )
    parser.add_argument(
        "--max-results", type=int, default=5, help="Maximum results"
    )
    parser.add_argument(
        "--rebuild", action="store_true", help="Rebuild semantic database"
    )
    parser.add_argument(
        "--debug", action="store_true", help="Enable debug output for database operations"
    )

    args = parser.parse_args()

    # Map legacy scopes to new categories
    scope_mapping = {
        "STM32H7": "hal",
        "L6470": "motor",
        "NUCLEO_BSP": "hal",
        "all": "all",
    }

    try:
        # Initialize searcher - only auto-rebuild if explicitly requested
        if args.rebuild:
            print("🔄 Rebuilding semantic database...")
            # Clear and rebuild database
            if SEMANTIC_DB_PATH.exists():
                import shutil
                shutil.rmtree(SEMANTIC_DB_PATH)
            searcher = STM32WorkspaceSearcher(auto_rebuild=True)
        else:
            # Normal search mode - do not auto-rebuild
            searcher = STM32WorkspaceSearcher(auto_rebuild=False)
            
        # Enable debug output if requested
        if args.debug:
            print(f"🔍 Debug: Searcher instance: {type(searcher)}")
            try:
                collection_stats = searcher.vector_db.get_collection_stats()
                print(f"🔍 Debug: Available collections: {list(collection_stats.keys())}")
                print(f"🔍 Debug: Collection counts: {collection_stats}")
            except Exception as e:
                print(f"🔍 Debug: Error getting collection info: {e}")

        # Perform search
        category = scope_mapping.get(args.scope, "all")

        # Enhance query based on search type
        enhanced_query = f"{args.query}"
        if args.search_type == "function":
            enhanced_query = f"function {args.query} implementation usage"
        elif args.search_type == "peripheral":
            enhanced_query = f"{args.query} peripheral configuration registers"
        elif args.search_type == "register":
            enhanced_query = f"{args.query} register configuration bits"

        results = searcher.search(enhanced_query, category, args.max_results)

        if not results:
            print("\n💡 Suggestions:")
            print("   - Try broader search terms")
            print("   - Use different scope (--scope all)")
            print("   - Check if documentation is processed (--rebuild)")

        return 0 if results else 1

    except Exception as e:
        print(f"❌ Search failed: {e}")
        return 1


if __name__ == "__main__":
    exit(main())
