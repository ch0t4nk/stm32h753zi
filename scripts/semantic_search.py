#!/usr/bin/env python3
"""
Semantic Search Query Interface for STM32 Documentation
Provides intelligent search capabilities across STM32 documentation corpus
"""

import ollama
from typing import List, Dict, Any, Optional, Union
from dataclasses import dataclass
from pathlib import Path
import argparse

# Import our custom modules
from init_vector_db import STM32VectorDatabase, CollectionType


@dataclass
class SearchResult:
    """Search result with relevance scoring"""

    content: str
    source_file: str
    chunk_type: str
    similarity_score: float
    metadata: Dict[str, Any]
    collection: str


@dataclass
class SearchQuery:
    """Structured search query"""

    text: str
    collections: List[CollectionType]
    max_results: int = 10
    min_similarity: float = 0.3
    filters: Optional[Dict[str, Any]] = None


class STM32SemanticSearch:
    """Main semantic search interface for STM32 documentation"""

    def __init__(
        self,
        embedding_model: str = "mxbai-embed-large",
        vector_db_path: str = "./vector_db",
    ):
        self.embedding_model = embedding_model
        self.vector_db_path = vector_db_path

        # Initialize vector database
        self.vector_db = STM32VectorDatabase(vector_db_path)

        # Test connection to Ollama
        self.use_mock_embeddings = not self.test_ollama_connection()

        # Common search categories
        self.search_categories = {
            "hal": [CollectionType.STM32_HAL],
            "motor": [CollectionType.MOTOR_CONTROL],
            "safety": [CollectionType.SAFETY_SYSTEMS],
            "code": [CollectionType.PROJECT_CODE],
            "instructions": [CollectionType.INSTRUCTION_GUIDES],
            "all": list(CollectionType),
        }

    def test_ollama_connection(self) -> bool:
        """Test connection to Ollama service"""
        try:
            test_response = ollama.embed(
                model=self.embedding_model, input="test connection"
            )
            return "embedding" in test_response
        except Exception as e:
            print(f"⚠️  Ollama connection failed: {e}")
            print("   Using mock embeddings for demonstration")
            return False

    def search(self, query: Union[str, SearchQuery]) -> List[SearchResult]:
        """Main search interface"""
        # Convert string query to SearchQuery object
        if isinstance(query, str):
            query = SearchQuery(
                text=query, collections=list(CollectionType), max_results=10
            )

        print(f"🔍 Searching for: '{query.text}'")

        # Generate query embedding
        query_embedding = self.generate_query_embedding(query.text)
        if not query_embedding:
            print("❌ Failed to generate query embedding")
            return []

        # Search across specified collections
        all_results = []

        for collection_type in query.collections:
            try:
                results = self.vector_db.search_documents(
                    collection_type=collection_type,
                    query_embedding=query_embedding,
                    n_results=query.max_results,
                    where=query.filters,
                )

                # Convert to SearchResult objects
                if results and "documents" in results:
                    for i, doc in enumerate(results["documents"]):
                        if i < len(results.get("distances", [])):
                            # Convert distance to similarity (1 - distance)
                            similarity = 1.0 - results["distances"][i]

                            if similarity >= query.min_similarity:
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
                                    chunk_type=metadata.get(
                                        "chunk_type", "unknown"
                                    ),
                                    similarity_score=similarity,
                                    metadata=metadata,
                                    collection=collection_type.value,
                                )
                                all_results.append(search_result)

            except Exception as e:
                print(f"⚠️  Search failed in {collection_type.value}: {e}")

        # Sort by similarity score
        all_results.sort(key=lambda x: x.similarity_score, reverse=True)

        # Return top results
        return all_results[: query.max_results]

    def search_by_category(
        self, query_text: str, category: str = "all", max_results: int = 10
    ) -> List[SearchResult]:
        """Search by predefined category"""
        if category not in self.search_categories:
            available = ", ".join(self.search_categories.keys())
            raise ValueError(
                f"Invalid category '{category}'. " f"Available: {available}"
            )

        search_query = SearchQuery(
            text=query_text,
            collections=self.search_categories[category],
            max_results=max_results,
        )

        return self.search(search_query)

    def search_hal_functions(
        self, function_name: str, max_results: int = 5
    ) -> List[SearchResult]:
        """Search specifically for HAL functions"""
        search_query = SearchQuery(
            text=f"HAL_{function_name}",
            collections=[CollectionType.STM32_HAL],
            max_results=max_results,
            filters={"hal_functions": {"$contains": f"HAL_{function_name}"}},
        )

        return self.search(search_query)

    def search_peripherals(
        self, peripheral: str, max_results: int = 10
    ) -> List[SearchResult]:
        """Search for peripheral-specific documentation"""
        search_query = SearchQuery(
            text=f"{peripheral} peripheral configuration",
            collections=[
                CollectionType.STM32_HAL,
                CollectionType.PROJECT_CODE,
            ],
            max_results=max_results,
            filters={"peripherals": {"$contains": peripheral.upper()}},
        )

        return self.search(search_query)

    def search_safety_critical(
        self, query_text: str, max_results: int = 10
    ) -> List[SearchResult]:
        """Search for safety-critical information"""
        search_query = SearchQuery(
            text=query_text,
            collections=[
                CollectionType.SAFETY_SYSTEMS,
                CollectionType.INSTRUCTION_GUIDES,
            ],
            max_results=max_results,
            filters={"safety_relevance": {"$gte": 1}},
        )

        return self.search(search_query)

    def search_motor_control(
        self, query_text: str, max_results: int = 10
    ) -> List[SearchResult]:
        """Search motor control specific documentation"""
        search_query = SearchQuery(
            text=query_text,
            collections=[CollectionType.MOTOR_CONTROL],
            max_results=max_results,
        )

        return self.search(search_query)

    def generate_query_embedding(
        self, query_text: str
    ) -> Optional[List[float]]:
        """Generate embedding for search query"""
        if self.use_mock_embeddings:
            # Generate mock embedding for testing
            import numpy as np

            return np.random.rand(1024).tolist()

        try:
            response = ollama.embed(
                model=self.embedding_model, input=query_text
            )
            return response["embedding"]

        except Exception as e:
            print(f"Query embedding generation failed: {e}")
            return None

    def print_results(
        self,
        results: List[SearchResult],
        show_content: bool = True,
        max_content_length: int = 200,
    ) -> None:
        """Print search results in formatted way"""
        if not results:
            print("❌ No results found")
            return

        print(f"\n🎯 Found {len(results)} results:")
        print("=" * 60)

        for i, result in enumerate(results, 1):
            print(f"\n{i}. 📄 {Path(result.source_file).name}")
            print(f"   📁 Collection: {result.collection}")
            print(f"   🏷️  Type: {result.chunk_type}")
            print(f"   📊 Similarity: {result.similarity_score:.3f}")

            if show_content:
                content = result.content
                if len(content) > max_content_length:
                    content = content[:max_content_length] + "..."
                print(f"   📝 Content: {content}")

            # Show relevant metadata
            interesting_metadata = {
                k: v
                for k, v in result.metadata.items()
                if k
                in [
                    "hal_functions",
                    "peripherals",
                    "registers",
                    "safety_relevance",
                    "realtime_relevance",
                ]
                and v
            }

            if interesting_metadata:
                print(f"   🔍 Metadata: {interesting_metadata}")

        print("=" * 60)

    def get_collection_status(self) -> Dict[str, Any]:
        """Get status of all collections"""
        return self.vector_db.get_collection_stats()

    def interactive_search(self) -> None:
        """Interactive search session"""
        print("🔍 STM32 Semantic Search - Interactive Mode")
        print("=" * 50)
        print(
            "Available categories:", ", ".join(self.search_categories.keys())
        )
        print("Type 'quit' to exit, 'help' for commands")
        print()

        while True:
            try:
                query = input("Search query: ").strip()

                if query.lower() in ["quit", "exit", "q"]:
                    print("👋 Goodbye!")
                    break

                if query.lower() == "help":
                    self.print_help()
                    continue

                if query.lower() == "status":
                    stats = self.get_collection_status()
                    print("📊 Collection Status:")
                    for collection, info in stats.items():
                        if "error" not in info:
                            count = info.get("document_count", 0)
                            print(f"   {collection}: {count} documents")
                    continue

                if not query:
                    continue

                # Parse category if specified
                category = "all"
                if ":" in query:
                    parts = query.split(":", 1)
                    if parts[0].strip() in self.search_categories:
                        category = parts[0].strip()
                        query = parts[1].strip()

                # Perform search
                results = self.search_by_category(query, category)
                self.print_results(results)

            except KeyboardInterrupt:
                print("\n👋 Goodbye!")
                break
            except Exception as e:
                print(f"❌ Search error: {e}")

    def print_help(self) -> None:
        """Print help information"""
        help_text = """
🔍 STM32 Semantic Search Help
============================

Search Syntax:
  <query>                    - Search all collections
  <category>:<query>         - Search specific category

Available Categories:
  hal        - STM32 HAL documentation
  motor      - Motor control (L6470, stepper)
  safety     - Safety systems and fault handling
  code       - Project source code
  instructions - Setup and configuration guides
  all        - Search all collections

Special Commands:
  help       - Show this help
  status     - Show collection statistics
  quit       - Exit interactive mode

Examples:
  GPIO configuration
  hal:HAL_GPIO_Init
  motor:L6470 stepper control
  safety:fault detection
  code:motor initialization
        """
        print(help_text)


def main():
    """Command line interface for semantic search"""
    parser = argparse.ArgumentParser(
        description="STM32 Semantic Search Interface"
    )
    parser.add_argument(
        "query",
        nargs="?",
        help="Search query (interactive mode if not provided)",
    )
    parser.add_argument(
        "--category",
        "-c",
        default="all",
        choices=["hal", "motor", "safety", "code", "instructions", "all"],
        help="Search category",
    )
    parser.add_argument(
        "--max-results",
        "-n",
        type=int,
        default=10,
        help="Maximum number of results",
    )
    parser.add_argument(
        "--min-similarity",
        "-s",
        type=float,
        default=0.3,
        help="Minimum similarity threshold",
    )
    parser.add_argument(
        "--show-content",
        action="store_true",
        help="Show content snippets in results",
    )
    parser.add_argument(
        "--vector-db", default="./vector_db", help="Vector database path"
    )

    args = parser.parse_args()

    # Initialize search engine
    search_engine = STM32SemanticSearch(vector_db_path=args.vector_db)

    if args.query:
        # Single search mode
        search_query = SearchQuery(
            text=args.query,
            collections=search_engine.search_categories[args.category],
            max_results=args.max_results,
            min_similarity=args.min_similarity,
        )

        results = search_engine.search(search_query)
        search_engine.print_results(results, show_content=args.show_content)
    else:
        # Interactive mode
        search_engine.interactive_search()


if __name__ == "__main__":
    main()
