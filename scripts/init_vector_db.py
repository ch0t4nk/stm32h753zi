#!/usr/bin/env python3
"""
Vector Database Initialization for STM32 Semantic Search
Creates ChromaDB collections with metadata schema for STM32 documentation
"""

import chromadb
import json
import time
from typing import List, Dict, Any, Optional
from pathlib import Path
from dataclasses import dataclass, asdict
from enum import Enum


class CollectionType(Enum):
    STM32_HAL = "stm32_hal"
    MOTOR_CONTROL = "motor_control"
    SAFETY_SYSTEMS = "safety_systems"
    PROJECT_CODE = "project_code"
    INSTRUCTION_GUIDES = "instruction_guides"


@dataclass
class CollectionConfig:
    name: str
    description: str
    chunk_types: List[str]
    metadata_fields: List[str]
    embedding_function: Optional[str] = "mxbai-embed-large"


class STM32VectorDatabase:
    """Vector database manager for STM32 semantic search"""

    def __init__(self, db_path: str = "./vector_db"):
        self.db_path = Path(db_path)
        self.client = None
        self.collections = {}

        # Collection configurations
        self.collection_configs = {
            CollectionType.STM32_HAL: CollectionConfig(
                name="stm32_hal",
                description="STM32H7 HAL function documentation",
                chunk_types=["HAL_FUNCTION", "REGISTER_MAP"],
                metadata_fields=[
                    "function_name",
                    "peripheral",
                    "category",
                    "complexity",
                    "hal_version",
                    "register_access",
                    "code_example",
                ],
            ),
            CollectionType.MOTOR_CONTROL: CollectionConfig(
                name="motor_control",
                description="L6470 and stepper motor control documentation",
                chunk_types=["DRIVER_FUNCTION", "ALGORITHM", "CONFIG_GUIDE"],
                metadata_fields=[
                    "driver_type",
                    "control_mode",
                    "safety_level",
                    "motor_type",
                    "coordination_type",
                    "motion_profile",
                ],
            ),
            CollectionType.SAFETY_SYSTEMS: CollectionConfig(
                name="safety_systems",
                description="Safety requirements and fault handling",
                chunk_types=["SAFETY_GUIDE", "FAULT_PROCEDURE", "VALIDATION"],
                metadata_fields=[
                    "safety_level",
                    "real_time",
                    "fault_type",
                    "safety_critical",
                    "real_time_constraint",
                ],
            ),
            CollectionType.PROJECT_CODE: CollectionConfig(
                name="project_code",
                description="Implementation examples and patterns",
                chunk_types=[
                    "CODE_EXAMPLE",
                    "CONFIG_PATTERN",
                    "BEST_PRACTICE",
                ],
                metadata_fields=[
                    "file_path",
                    "ssot_compliance",
                    "test_coverage",
                    "instruction_scope",
                    "ssot_config_file",
                ],
            ),
            CollectionType.INSTRUCTION_GUIDES: CollectionConfig(
                name="instruction_guides",
                description="Project-specific guidelines and SSOT definitions",
                chunk_types=["INSTRUCTION", "SSOT_CONFIG", "WORKFLOW"],
                metadata_fields=[
                    "instruction_type",
                    "applies_to",
                    "priority",
                    "test_requirement",
                    "documentation_category",
                ],
            ),
        }

    def initialize_database(self) -> bool:
        """Initialize ChromaDB client and create collections"""
        try:
            # Create persistent client
            self.client = chromadb.PersistentClient(path=str(self.db_path))

            print(f"🗄️  Initializing vector database at {self.db_path}")

            # Create collections
            for collection_type, config in self.collection_configs.items():
                success = self.create_collection(collection_type, config)
                if not success:
                    print(f"❌ Failed to create collection: {config.name}")
                    return False

            print(
                f"✅ Database initialized with {len(self.collections)} collections"
            )
            return True

        except Exception as e:
            print(f"❌ Database initialization failed: {e}")
            return False

    def create_collection(
        self, collection_type: CollectionType, config: CollectionConfig
    ) -> bool:
        """Create a collection with metadata schema"""
        try:
            # Check if collection exists
            existing_collections = [
                col.name for col in self.client.list_collections()
            ]

            if config.name in existing_collections:
                print(f"📁 Loading existing collection: {config.name}")
                collection = self.client.get_collection(config.name)
            else:
                print(f"🆕 Creating new collection: {config.name}")
                collection = self.client.create_collection(
                    name=config.name,
                    metadata={
                        "description": config.description,
                        "chunk_types": json.dumps(config.chunk_types),
                        "metadata_fields": json.dumps(config.metadata_fields),
                        "created_at": time.time(),
                        "embedding_function": config.embedding_function,
                    },
                )

            self.collections[collection_type] = collection

            # Print collection info
            collection_info = collection.count()
            print(
                f"   📊 Collection '{config.name}': {collection_info} documents"
            )

            return True

        except Exception as e:
            print(f"❌ Failed to create collection {config.name}: {e}")
            return False

    def add_documents(
        self,
        collection_type: CollectionType,
        documents: List[str],
        embeddings: List[List[float]],
        metadatas: List[Dict[str, Any]],
        ids: List[str],
    ) -> bool:
        """Add documents to specified collection"""
        try:
            if collection_type not in self.collections:
                print(f"❌ Collection {collection_type} not found")
                return False

            collection = self.collections[collection_type]

            # Validate inputs
            if not (
                len(documents) == len(embeddings) == len(metadatas) == len(ids)
            ):
                print("❌ Input arrays must have same length")
                return False

            # Add documents in batches
            batch_size = 100
            total_docs = len(documents)

            for i in range(0, total_docs, batch_size):
                end_idx = min(i + batch_size, total_docs)

                batch_docs = documents[i:end_idx]
                batch_embeddings = embeddings[i:end_idx]
                batch_metadatas = metadatas[i:end_idx]
                batch_ids = ids[i:end_idx]

                collection.add(
                    documents=batch_docs,
                    embeddings=batch_embeddings,
                    metadatas=batch_metadatas,
                    ids=batch_ids,
                )

                print(
                    f"   📝 Added batch {i//batch_size + 1}: {len(batch_docs)} documents"
                )

            print(
                f"✅ Added {total_docs} documents to {collection_type.value}"
            )
            return True

        except Exception as e:
            print(
                f"❌ Failed to add documents to {collection_type.value}: {e}"
            )
            return False

    def search_collection(
        self,
        collection_type: CollectionType,
        query_embeddings: List[List[float]],
        n_results: int = 10,
        where: Optional[Dict[str, Any]] = None,
    ) -> Dict[str, Any]:
        """Search in specific collection"""
        try:
            if collection_type not in self.collections:
                return {
                    "success": False,
                    "error": f"Collection {collection_type} not found",
                }

            collection = self.collections[collection_type]

            results = collection.query(
                query_embeddings=query_embeddings,
                n_results=n_results,
                where=where,
                include=["documents", "metadatas", "distances"],
            )

            return {
                "success": True,
                "collection": collection_type.value,
                "results": results,
            }

        except Exception as e:
            return {
                "success": False,
                "error": f"Search failed in {collection_type.value}: {e}",
            }

    def search_all_collections(
        self, query_embeddings: List[List[float]], n_results: int = 5
    ) -> Dict[str, Any]:
        """Search across all collections"""
        all_results = {}

        for collection_type in self.collections:
            result = self.search_collection(
                collection_type, query_embeddings, n_results
            )
            all_results[collection_type.value] = result

        return all_results

    def search_documents(
        self,
        collection_type: CollectionType,
        query_embedding: List[float],
        n_results: int = 10,
        where: Optional[Dict[str, Any]] = None,
    ) -> Optional[Dict[str, Any]]:
        """Search documents in a specific collection (compatible interface)"""
        try:
            if collection_type not in self.collections:
                print(f"⚠️  Collection {collection_type.value} not found")
                return None

            collection = self.collections[collection_type]

            # ChromaDB expects query_embeddings as list of lists
            results = collection.query(
                query_embeddings=[query_embedding],
                n_results=n_results,
                where=where,
                include=["documents", "metadatas", "distances"],
            )

            # Flatten the results since we only have one query
            if results and "documents" in results and results["documents"]:
                return {
                    "documents": results["documents"][0],
                    "metadatas": (
                        results["metadatas"][0]
                        if "metadatas" in results
                        else []
                    ),
                    "distances": (
                        results["distances"][0]
                        if "distances" in results
                        else []
                    ),
                    "ids": results["ids"][0] if "ids" in results else [],
                }
            else:
                return {
                    "documents": [],
                    "metadatas": [],
                    "distances": [],
                    "ids": [],
                }

        except Exception as e:
            print(f"⚠️  Search failed in {collection_type.value}: {e}")
            return None

    def get_collection_stats(self) -> Dict[str, Any]:
        """Get statistics for all collections"""
        stats = {}

        for collection_type, collection in self.collections.items():
            try:
                count = collection.count()
                metadata = collection.metadata

                stats[collection_type.value] = {
                    "document_count": count,
                    "description": metadata.get("description", ""),
                    "chunk_types": json.loads(
                        metadata.get("chunk_types", "[]")
                    ),
                    "created_at": metadata.get("created_at", 0),
                }

            except Exception as e:
                stats[collection_type.value] = {"error": str(e)}

        return stats

    def backup_database(self, backup_path: str) -> bool:
        """Create backup of vector database"""
        try:
            backup_dir = Path(backup_path)
            backup_dir.mkdir(parents=True, exist_ok=True)

            timestamp = int(time.time())

            for collection_type, collection in self.collections.items():
                # Get all documents
                results = collection.get(
                    include=["documents", "metadatas", "embeddings"]
                )

                backup_file = (
                    backup_dir / f"{collection_type.value}_{timestamp}.json"
                )

                backup_data = {
                    "collection_name": collection_type.value,
                    "timestamp": timestamp,
                    "documents": results["documents"],
                    "metadatas": results["metadatas"],
                    "embeddings": results["embeddings"],
                    "ids": results["ids"],
                }

                with open(backup_file, "w") as f:
                    json.dump(backup_data, f, indent=2)

                print(f"💾 Backed up {collection_type.value} to {backup_file}")

            return True

        except Exception as e:
            print(f"❌ Backup failed: {e}")
            return False

    def restore_database(self, backup_path: str) -> bool:
        """Restore vector database from backup"""
        try:
            backup_dir = Path(backup_path)

            if not backup_dir.exists():
                print(f"❌ Backup directory not found: {backup_path}")
                return False

            # Find backup files
            backup_files = list(backup_dir.glob("*.json"))

            if not backup_files:
                print(f"❌ No backup files found in {backup_path}")
                return False

            for backup_file in backup_files:
                with open(backup_file, "r") as f:
                    backup_data = json.load(f)

                collection_name = backup_data["collection_name"]

                # Find corresponding collection type
                collection_type = None
                for ct, config in self.collection_configs.items():
                    if config.name == collection_name:
                        collection_type = ct
                        break

                if collection_type and collection_type in self.collections:
                    collection = self.collections[collection_type]

                    # Clear existing data
                    existing_ids = collection.get()["ids"]
                    if existing_ids:
                        collection.delete(ids=existing_ids)

                    # Restore data
                    collection.add(
                        documents=backup_data["documents"],
                        embeddings=backup_data["embeddings"],
                        metadatas=backup_data["metadatas"],
                        ids=backup_data["ids"],
                    )

                    print(f"🔄 Restored {collection_name} from {backup_file}")

            return True

        except Exception as e:
            print(f"❌ Restore failed: {e}")
            return False

    def close(self):
        """Close database connections"""
        self.collections.clear()
        if self.client:
            self.client = None
        print("🔒 Database connections closed")


def test_vector_database():
    """Test vector database functionality"""
    print("🧪 Testing STM32 Vector Database...")

    # Initialize database
    db = STM32VectorDatabase("./test_vector_db")

    if not db.initialize_database():
        print("❌ Database initialization failed")
        return False

    # Test with sample data
    sample_documents = [
        "HAL_GPIO_Init configures GPIO peripheral pins",
        "L6470 stepper driver controls motor position",
        "Safety monitor checks for fault conditions",
        "Position control maintains motor accuracy",
        "SSOT configuration ensures parameter consistency",
    ]

    # Generate mock embeddings (would use real embeddings in production)
    import numpy as np

    sample_embeddings = [
        np.random.rand(1024).tolist() for _ in sample_documents
    ]

    sample_metadatas = [
        {
            "function_name": "HAL_GPIO_Init",
            "peripheral": "GPIO",
            "category": "initialization",
        },
        {
            "driver_type": "L6470",
            "control_mode": "position",
            "motor_type": "stepper",
        },
        {
            "safety_level": "critical",
            "fault_type": "monitoring",
            "real_time": True,
        },
        {
            "file_path": "src/controllers/position_control.c",
            "ssot_compliance": True,
        },
        {
            "instruction_type": "SSOT",
            "applies_to": "**/*.{c,h}",
            "priority": "high",
        },
    ]

    sample_ids = [f"doc_{i}" for i in range(len(sample_documents))]

    # Test adding documents to different collections
    collections_to_test = [
        (CollectionType.STM32_HAL, 0, 1),
        (CollectionType.MOTOR_CONTROL, 1, 2),
        (CollectionType.SAFETY_SYSTEMS, 2, 3),
        (CollectionType.PROJECT_CODE, 3, 4),
        (CollectionType.INSTRUCTION_GUIDES, 4, 5),
    ]

    for collection_type, start_idx, end_idx in collections_to_test:
        success = db.add_documents(
            collection_type,
            sample_documents[start_idx:end_idx],
            sample_embeddings[start_idx:end_idx],
            sample_metadatas[start_idx:end_idx],
            sample_ids[start_idx:end_idx],
        )

        if not success:
            print(f"❌ Failed to add documents to {collection_type}")
            return False

    # Test search functionality
    query_embedding = [np.random.rand(1024).tolist()]

    search_result = db.search_collection(
        CollectionType.STM32_HAL, query_embedding, n_results=2
    )

    if search_result["success"]:
        print(
            f"✅ Search test passed: found {len(search_result['results']['documents'][0])} results"
        )
    else:
        print(f"❌ Search test failed: {search_result['error']}")
        return False

    # Test statistics
    stats = db.get_collection_stats()
    print(f"📊 Database statistics: {json.dumps(stats, indent=2)}")

    # Test backup
    if db.backup_database("./test_backup"):
        print("✅ Backup test passed")
    else:
        print("❌ Backup test failed")
        return False

    db.close()
    print("✅ All vector database tests passed!")
    return True


def main():
    """Main execution for database initialization"""
    if test_vector_database():
        print("\n🎉 Vector database system validated successfully!")
        return 0
    else:
        print("\n💥 Vector database system validation failed!")
        return 1


if __name__ == "__main__":
    exit(main())
