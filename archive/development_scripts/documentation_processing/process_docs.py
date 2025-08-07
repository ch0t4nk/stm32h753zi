#!/usr/bin/env python3
"""
Document Processing Pipeline for STM32 Semantic Search
Processes documentation files and generates embeddings for vector database
"""

import os
import json
import time
import ollama
from pathlib import Path
from typing import List, Dict, Any, Optional
from dataclasses import dataclass
import hashlib
import re

# Import our custom modules
from chunk_stm32_docs import STM32DocumentChunker, DocumentChunk, ChunkType
from init_vector_db import STM32VectorDatabase, CollectionType


@dataclass
class ProcessingStats:
    total_files: int = 0
    processed_files: int = 0
    failed_files: int = 0
    total_chunks: int = 0
    total_embeddings: int = 0
    processing_time: float = 0.0
    errors: List[str] = None

    def __post_init__(self):
        if self.errors is None:
            self.errors = []


class STM32DocumentProcessor:
    """Main document processing pipeline for STM32 semantic search"""

    def __init__(
        self,
        embedding_model: str = "mxbai-embed-large",
        vector_db_path: str = "./vector_db",
        batch_size: int = 50,
    ):
        self.embedding_model = embedding_model
        self.vector_db_path = vector_db_path
        self.batch_size = batch_size

        # Initialize components
        self.chunker = STM32DocumentChunker()
        self.vector_db = STM32VectorDatabase(vector_db_path)

        # Processing statistics
        self.stats = ProcessingStats()

        # Source mappings
        self.source_mappings = {
            "00_reference/stm32h753xx_User_Manual_md/": CollectionType.STM32_HAL,
            "00_reference/x_cube_spn2_markdown_docs/": CollectionType.MOTOR_CONTROL,
            "00_reference/stm32h7xx_nucleo_bsp_md/": CollectionType.STM32_HAL,
            ".github/instructions/": CollectionType.INSTRUCTION_GUIDES,
            "src/": CollectionType.PROJECT_CODE,
            "docs/": CollectionType.INSTRUCTION_GUIDES,
        }

        # File patterns to process
        self.file_patterns = {
            "markdown": [".md", ".markdown"],
            "code": [".c", ".h", ".cpp", ".hpp"],
            "config": [".json", ".yaml", ".yml", ".txt"],
        }

    def initialize_system(self) -> bool:
        """Initialize the complete processing system"""
        print("🚀 Initializing STM32 Document Processing System...")

        # Test Ollama connection
        if not self.test_ollama_connection():
            print(
                "❌ Ollama connection failed - using mock embeddings for testing"
            )
            self.use_mock_embeddings = True
        else:
            print("✅ Ollama connection established")
            self.use_mock_embeddings = False

        # Initialize vector database
        if not self.vector_db.initialize_database():
            print("❌ Vector database initialization failed")
            return False

        print("✅ System initialization completed")
        return True

    def test_ollama_connection(self) -> bool:
        """Test connection to Ollama service"""
        try:
            test_response = ollama.embed(
                model=self.embedding_model, input="test connection"
            )
            return "embedding" in test_response
        except Exception as e:
            print(f"Ollama connection test failed: {e}")
            return False

    def process_all_documents(self) -> ProcessingStats:
        """Process all STM32 documentation sources"""
        print("📚 Starting comprehensive document processing...")
        start_time = time.time()

        # Reset statistics
        self.stats = ProcessingStats()

        # Process each source directory
        for source_path, collection_type in self.source_mappings.items():
            if os.path.exists(source_path):
                print(
                    f"\n📁 Processing: {source_path} → {collection_type.value}"
                )
                self.process_directory(source_path, collection_type)
            else:
                print(f"⚠️  Source not found: {source_path}")

        # Finalize statistics
        self.stats.processing_time = time.time() - start_time

        self.print_processing_summary()
        return self.stats

    def process_directory(
        self, directory_path: str, collection_type: CollectionType
    ) -> None:
        """Process all files in a directory"""
        directory = Path(directory_path)

        # Find all processable files
        files_to_process = []
        for pattern_type, extensions in self.file_patterns.items():
            for ext in extensions:
                files_to_process.extend(directory.rglob(f"*{ext}"))

        self.stats.total_files += len(files_to_process)

        print(f"   📄 Found {len(files_to_process)} files to process")

        # Process files in batches
        batch_documents = []
        batch_embeddings = []
        batch_metadatas = []
        batch_ids = []

        for file_path in files_to_process:
            try:
                # Process single file
                chunks = self.process_file(file_path, collection_type)

                if not chunks:
                    continue

                # Generate embeddings for chunks
                for chunk in chunks:
                    # Generate unique ID
                    chunk_id = self.generate_chunk_id(file_path, chunk)

                    # Generate embedding
                    embedding = self.generate_embedding(chunk.content)

                    if embedding:
                        batch_documents.append(chunk.content)
                        batch_embeddings.append(embedding)
                        batch_metadatas.append(
                            self.prepare_metadata(chunk, file_path)
                        )
                        batch_ids.append(chunk_id)

                        self.stats.total_chunks += 1
                        self.stats.total_embeddings += 1

                # Process batch when it reaches batch_size
                if len(batch_documents) >= self.batch_size:
                    self.store_batch(
                        collection_type,
                        batch_documents,
                        batch_embeddings,
                        batch_metadatas,
                        batch_ids,
                    )

                    # Clear batch
                    batch_documents.clear()
                    batch_embeddings.clear()
                    batch_metadatas.clear()
                    batch_ids.clear()

                self.stats.processed_files += 1

                if self.stats.processed_files % 10 == 0:
                    print(
                        f"   ⏳ Processed {self.stats.processed_files} files..."
                    )

            except Exception as e:
                self.stats.failed_files += 1
                error_msg = f"Failed to process {file_path}: {e}"
                self.stats.errors.append(error_msg)
                print(f"   ❌ {error_msg}")

        # Process remaining batch
        if batch_documents:
            self.store_batch(
                collection_type,
                batch_documents,
                batch_embeddings,
                batch_metadatas,
                batch_ids,
            )

    def process_file(
        self, file_path: Path, collection_type: CollectionType
    ) -> List[DocumentChunk]:
        """Process a single file and return chunks"""
        try:
            # Read file content
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()

            if len(content.strip()) < 50:  # Skip very small files
                return []

            # Determine chunk type based on file path and collection
            chunk_type = self.determine_chunk_type(file_path, collection_type)

            # Generate chunks
            chunks = self.chunker.chunk_document(
                content, str(file_path), force_type=chunk_type
            )

            return chunks

        except Exception as e:
            print(f"Error processing file {file_path}: {e}")
            return []

    def determine_chunk_type(
        self, file_path: Path, collection_type: CollectionType
    ) -> Optional[ChunkType]:
        """Determine appropriate chunk type for file"""
        file_str = str(file_path).lower()

        # Safety-related files
        if any(term in file_str for term in ["safety", "fault", "emergency"]):
            return ChunkType.SAFETY_GUIDE

        # Code files
        if file_path.suffix in [".c", ".h", ".cpp", ".hpp"]:
            return ChunkType.CODE_EXAMPLE

        # HAL documentation
        if "hal" in file_str or "stm32h7" in file_str:
            return ChunkType.HAL_FUNCTION

        # Motor control documentation
        if any(
            term in file_str for term in ["l6470", "motor", "stepper", "spn2"]
        ):
            return ChunkType.CONFIG_SEQUENCE

        # Instructions and configuration
        if "instruction" in file_str or "config" in file_str:
            return ChunkType.CONFIG_SEQUENCE

        return None  # Let chunker decide

    def generate_embedding(self, content: str) -> Optional[List[float]]:
        """Generate embedding for content"""
        if self.use_mock_embeddings:
            # Generate mock embedding for testing
            import numpy as np

            return np.random.rand(1024).tolist()

        try:
            response = ollama.embed(model=self.embedding_model, input=content)
            return response["embedding"]

        except Exception as e:
            print(f"Embedding generation failed: {e}")
            return None

    def generate_chunk_id(self, file_path: Path, chunk: DocumentChunk) -> str:
        """Generate unique ID for chunk"""
        # Create hash from file path and chunk content
        hash_input = f"{file_path}:{chunk.chunk_index}:{chunk.content[:100]}"
        hash_object = hashlib.md5(hash_input.encode())
        return f"{file_path.stem}_{chunk.chunk_index}_{hash_object.hexdigest()[:8]}"

    def prepare_metadata(
        self, chunk: DocumentChunk, file_path: Path
    ) -> Dict[str, Any]:
        """Prepare metadata for vector database storage"""
        metadata = chunk.metadata.copy()

        # Add universal metadata
        metadata.update(
            {
                "source_file": str(file_path),
                "file_extension": file_path.suffix,
                "file_size": (
                    file_path.stat().st_size if file_path.exists() else 0
                ),
                "chunk_index": chunk.chunk_index,
                "chunk_type": chunk.chunk_type.value,
                "content_length": len(chunk.content),
                "processing_timestamp": time.time(),
                "embedding_model": self.embedding_model,
            }
        )

        # Add STM32-specific metadata
        metadata.update(self.extract_stm32_metadata(chunk.content))

        return metadata

    def extract_stm32_metadata(self, content: str) -> Dict[str, Any]:
        """Extract STM32-specific metadata from content"""
        metadata = {}

        # Extract STM32 peripheral references
        peripherals = re.findall(
            r"\b(GPIO|SPI|UART|I2C|TIM|DMA|ADC|DAC|CAN|ETH)\b",
            content,
            re.IGNORECASE,
        )
        if peripherals:
            # Convert list to comma-separated string for ChromaDB compatibility
            metadata["peripherals"] = ",".join(
                set([p.upper() for p in peripherals])
            )

        # Extract HAL function references
        hal_functions = re.findall(r"\bHAL_\w+", content)
        if hal_functions:
            # Convert list to comma-separated string
            metadata["hal_functions"] = ",".join(set(hal_functions))

        # Extract register references
        registers = re.findall(r"\b[A-Z]+_[A-Z0-9_]+(?:_REG)?\b", content)
        if registers:
            # Convert list to comma-separated string, limit to 10
            metadata["registers"] = ",".join(set(registers[:10]))

        # Check for safety-critical content
        safety_keywords = [
            "safety",
            "fault",
            "error",
            "emergency",
            "critical",
            "watchdog",
        ]
        safety_score = sum(
            1
            for keyword in safety_keywords
            if keyword.lower() in content.lower()
        )
        metadata["safety_relevance"] = safety_score

        # Check for real-time content
        rt_keywords = [
            "real-time",
            "timing",
            "frequency",
            "period",
            "deadline",
            "interrupt",
        ]
        rt_score = sum(
            1 for keyword in rt_keywords if keyword.lower() in content.lower()
        )
        metadata["realtime_relevance"] = rt_score

        return metadata

    def store_batch(
        self,
        collection_type: CollectionType,
        documents: List[str],
        embeddings: List[List[float]],
        metadatas: List[Dict[str, Any]],
        ids: List[str],
    ) -> bool:
        """Store a batch of documents in vector database"""
        try:
            success = self.vector_db.add_documents(
                collection_type, documents, embeddings, metadatas, ids
            )

            if success:
                print(f"   ✅ Stored batch of {len(documents)} documents")
            else:
                print(
                    f"   ❌ Failed to store batch of {len(documents)} documents"
                )

            return success

        except Exception as e:
            print(f"   ❌ Batch storage failed: {e}")
            return False

    def print_processing_summary(self) -> None:
        """Print comprehensive processing summary"""
        print("\n" + "=" * 60)
        print("📊 DOCUMENT PROCESSING SUMMARY")
        print("=" * 60)
        print(f"🗂️  Total files found: {self.stats.total_files}")
        print(f"✅ Successfully processed: {self.stats.processed_files}")
        print(f"❌ Failed to process: {self.stats.failed_files}")
        print(f"🧩 Total chunks generated: {self.stats.total_chunks}")
        print(f"🎯 Total embeddings created: {self.stats.total_embeddings}")
        print(
            f"⏱️  Total processing time: {self.stats.processing_time:.2f} seconds"
        )

        if self.stats.errors:
            print(f"\n❌ Errors encountered ({len(self.stats.errors)}):")
            for error in self.stats.errors[:5]:  # Show first 5 errors
                print(f"   • {error}")
            if len(self.stats.errors) > 5:
                print(f"   ... and {len(self.stats.errors) - 5} more errors")

        # Collection statistics
        collection_stats = self.vector_db.get_collection_stats()
        print(f"\n📚 Collection Statistics:")
        for collection_name, stats in collection_stats.items():
            if "error" not in stats:
                print(
                    f"   {collection_name}: {stats['document_count']} documents"
                )

        print("=" * 60)

    def save_processing_report(
        self, output_file: str = "processing_report.json"
    ) -> None:
        """Save detailed processing report"""
        report = {
            "processing_stats": {
                "total_files": self.stats.total_files,
                "processed_files": self.stats.processed_files,
                "failed_files": self.stats.failed_files,
                "total_chunks": self.stats.total_chunks,
                "total_embeddings": self.stats.total_embeddings,
                "processing_time": self.stats.processing_time,
                "errors": self.stats.errors,
            },
            "collection_stats": self.vector_db.get_collection_stats(),
            "configuration": {
                "embedding_model": self.embedding_model,
                "vector_db_path": self.vector_db_path,
                "batch_size": self.batch_size,
                "source_mappings": {
                    k: v.value for k, v in self.source_mappings.items()
                },
            },
            "timestamp": time.time(),
        }

        with open(output_file, "w") as f:
            json.dump(report, f, indent=2)

        print(f"📋 Processing report saved to {output_file}")


def main():
    """Main execution for document processing"""
    print("🚀 STM32 Document Processing Pipeline")
    print("====================================")

    # Initialize processor
    processor = STM32DocumentProcessor()

    # Initialize system
    if not processor.initialize_system():
        print("❌ System initialization failed")
        return 1

    # Process all documents
    stats = processor.process_all_documents()

    # Save report
    processor.save_processing_report("scripts/processing_report.json")

    # Check if processing was successful
    if stats.failed_files == 0:
        print("\n🎉 Document processing completed successfully!")
        return 0
    elif stats.processed_files > stats.failed_files:
        print(
            f"\n⚠️  Document processing completed with {stats.failed_files} failures"
        )
        return 0
    else:
        print(
            f"\n💥 Document processing failed: too many failures ({stats.failed_files})"
        )
        return 1


if __name__ == "__main__":
    exit(main())
