#!/usr/bin/env python3
"""
Comprehensive Workspace Semantic Search Rebuild
Indexes the ENTIRE STM32H753ZI workspace for AI-powered assistance
"""

import chromadb
import requests
import json
import re
from pathlib import Path
from typing import List, Dict, Any, Optional
import hashlib
import time
import warnings
import os

# SSOT-based warning suppression configuration
SUPPRESS_WARNINGS_FOR_MODELS = [
    "hkunlp/instructor-large", 
    "hkunlp/instructor-xl",
    "sentence-transformers/all-MiniLM-L6-v2", 
    "BAAI/bge-large-en-v1.5"
]

SUPPRESS_DEPRECATION_WARNINGS = True
SUPPRESS_TARGET_DEVICE_WARNINGS = True
SUPPRESS_OLLAMA_CONNECTION_WARNINGS = False  # Keep visible for troubleshooting
SUPPRESS_GPU_AVAILABILITY_WARNINGS = False   # Keep visible for GPU setup

# Apply warning suppressions based on SSOT configuration
if SUPPRESS_DEPRECATION_WARNINGS:
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    
if SUPPRESS_TARGET_DEVICE_WARNINGS:
    warnings.filterwarnings("ignore", message=".*_target_device.*has been deprecated.*")
    
os.environ["TOKENIZERS_PARALLELISM"] = "false"  # Avoid tokenizer warnings

# Try to import Instructor-XL with fallback
try:
    from InstructorEmbedding import INSTRUCTOR
    import torch
    INSTRUCTOR_AVAILABLE = True
    
    # Check GPU availability
    if torch.cuda.is_available():
        gpu_count = torch.cuda.device_count()
        gpu_name = torch.cuda.get_device_name(0)
        if not SUPPRESS_GPU_AVAILABILITY_WARNINGS:
            print(f"🚀 InstructorEmbedding with GPU acceleration available")
            print(f"   GPU: {gpu_name} ({gpu_count} device{'s' if gpu_count > 1 else ''})")
    else:
        if not SUPPRESS_GPU_AVAILABILITY_WARNINGS:
            print("✅ InstructorEmbedding available (CPU mode)")
        
except ImportError:
    INSTRUCTOR_AVAILABLE = False
    if not SUPPRESS_OLLAMA_CONNECTION_WARNINGS:
        print("⚠️  InstructorEmbedding not available - falling back to Ollama")

WORKSPACE_ROOT = Path("/workspaces/code")
SEMANTIC_DB_PATH = WORKSPACE_ROOT / "docs" / "semantic_vector_db"

class WorkspaceSemanticIndexer:
    """Comprehensive workspace indexer for semantic search"""
    
    def __init__(self):
        self.client = chromadb.PersistentClient(path=str(SEMANTIC_DB_PATH))
        self.collections = {}
        
        # Initialize Instructor-XL if available
        self.instructor_model = None
        if INSTRUCTOR_AVAILABLE and 'torch' in globals() and 'INSTRUCTOR' in globals():
            try:
                if not SUPPRESS_GPU_AVAILABILITY_WARNINGS:
                    print("🔄 Loading Instructor-XL model (hkunlp/instructor-xl)...")
                
                # Force GPU usage if available  
                device = "cuda" if torch.cuda.is_available() else "cpu"
                self.instructor_model = INSTRUCTOR('hkunlp/instructor-xl', device=device)
                
                if device == "cuda" and not SUPPRESS_GPU_AVAILABILITY_WARNINGS:
                    print(f"✅ Instructor-XL loaded on GPU: {torch.cuda.get_device_name(0)}")
                elif not SUPPRESS_GPU_AVAILABILITY_WARNINGS:
                    print("✅ Instructor-XL loaded on CPU")
                    
            except Exception as e:
                if not SUPPRESS_OLLAMA_CONNECTION_WARNINGS:
                    print(f"⚠️  Failed to load Instructor-XL: {e}")
                self.instructor_model = None
        
        # Domain-specific embedding instructions for Instructor-XL
        self.embedding_instructions = {
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
        
        # Enhanced collection strategy covering ENTIRE workspace
        self.collection_config = {
            # Existing collections (enhanced)
            "stm32_hal": {
                "description": "STM32H7 HAL, Nucleo BSP, reference documentation",
                "paths": ["00_reference/stm32h7xx_nucleo_bsp_md/", 
                         "00_reference/stm32h753xx_User_Manual_md/",
                         "00_reference/STM32H7xx_Nucleo_BSP_User_Manual/"]
            },
            "motor_control": {
                "description": "L6470 stepper driver, X-CUBE-SPN2 documentation",
                "paths": ["00_reference/x_cube_spn2_markdown_docs/"]
            },
            
            # NEW collections for our mature workspace
            "project_source": {
                "description": "Production source code: HAL abstraction, drivers, controllers, safety",
                "paths": ["src/"],
                "extensions": [".c", ".h"]
            },
            "instruction_guides": {
                "description": "Modernized development instructions and coding guidelines",
                "paths": [".github/instructions/"],
                "extensions": [".md"]
            },
            "project_documentation": {
                "description": "Analysis reports, design docs, completion reports",
                "paths": ["docs/"],
                "extensions": [".md"],
                "exclude": ["semantic_vector_db/"]
            },
            "build_system": {
                "description": "CMake configuration, build scripts, toolchain setup",
                "paths": ["cmake/", "./"],
                "extensions": [".cmake", ".txt", ".json"],
                "include_files": ["CMakeLists.txt", "CMakePresets.json"]
            },
            "automation_scripts": {
                "description": "Python automation, build scripts, validation tools",
                "paths": ["scripts/"],
                "extensions": [".py", ".sh"]
            },
            "api_specifications": {
                "description": "REST API, CAN protocol, interface specifications",
                "paths": ["api/"],
                "extensions": [".yaml", ".yml", ".json"]
            },
            "workspace_config": {
                "description": "VS Code configuration, dev container, workspace settings",
                "paths": [".vscode/", ".devcontainer/"],
                "extensions": [".json", ".md"]
            },
            "validation_programs": {
                "description": "Phase 4A/4B/4C validation executables and test reports",
                "paths": ["archive/validation_programs/"],
                "extensions": [".c", ".md", ".txt"]
            }
        }
    
    def get_embedding(self, text: str, collection_name: str) -> List[float]:
        """Generate embedding using best available method with domain-specific instructions"""
        
        # Method 1: Instructor-XL with domain-specific instructions (preferred)
        if self.instructor_model is not None:
            try:
                instruction = self.embedding_instructions.get(
                    collection_name, 
                    "Represent the document for semantic search and retrieval:"
                )
                
                # Generate instruction-tuned embedding
                start_time = time.time()
                embedding = self.instructor_model.encode([[instruction, text]])
                embedding_time = time.time() - start_time
                
                # Instructor-XL returns 768-dimensional embeddings
                if len(embedding) > 0 and len(embedding[0]) == 768:
                    if embedding_time > 2.0:  # Log slow embeddings
                        print(f"    🐌 Slow embedding: {embedding_time:.2f}s for {len(text)} chars")
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
            
            if not SUPPRESS_OLLAMA_CONNECTION_WARNINGS:
                print("    ⚠️  Ollama failed, using mock embedding")
            return self._mock_embedding(text)
            
        except Exception as e:
            if not SUPPRESS_OLLAMA_CONNECTION_WARNINGS:
                print(f"    ⚠️  Ollama error: {e}, using mock embedding")
            return self._mock_embedding(text)
    
    def _mock_embedding(self, text: str) -> List[float]:
        """Fallback mock embedding for demonstration"""
        return [float(hash(text + str(i)) % 1000) / 1000.0 for i in range(1024)]
    
    def chunk_source_code(self, content: str, file_path: str) -> List[Dict]:
        """Smart chunking for source code files"""
        chunks = []
        lines = content.split('\n')
        
        # Detect functions, structs, and major code blocks
        current_chunk = []
        current_context = ""
        
        for i, line in enumerate(lines):
            # Detect function definitions
            if re.match(r'^\w+.*\(.*\)\s*\{?\s*$', line.strip()):
                if current_chunk:
                    chunks.append({
                        "content": '\n'.join(current_chunk),
                        "type": "code_block",
                        "context": current_context,
                        "line_start": i - len(current_chunk) + 1,
                        "line_end": i
                    })
                current_chunk = [line]
                current_context = f"Function: {line.strip()}"
            
            # Detect struct/typedef definitions
            elif re.match(r'^(typedef\s+)?(struct|enum|union)', line.strip()):
                if current_chunk:
                    chunks.append({
                        "content": '\n'.join(current_chunk),
                        "type": "code_block", 
                        "context": current_context,
                        "line_start": i - len(current_chunk) + 1,
                        "line_end": i
                    })
                current_chunk = [line]
                current_context = f"Definition: {line.strip()}"
            
            else:
                current_chunk.append(line)
                
            # Chunk size limit
            if len(current_chunk) > 50:
                chunks.append({
                    "content": '\n'.join(current_chunk),
                    "type": "code_block",
                    "context": current_context,
                    "line_start": i - len(current_chunk) + 1,
                    "line_end": i
                })
                current_chunk = []
                current_context = ""
        
        # Add final chunk
        if current_chunk:
            chunks.append({
                "content": '\n'.join(current_chunk),
                "type": "code_block",
                "context": current_context,
                "line_start": len(lines) - len(current_chunk) + 1,
                "line_end": len(lines)
            })
        
        return chunks
    
    def chunk_documentation(self, content: str, file_path: str) -> List[Dict]:
        """Smart chunking for markdown and documentation"""
        chunks = []
        
        # Split by headers for logical sections
        sections = re.split(r'\n(#{1,6}\s+.*)\n', content)
        
        current_section = ""
        current_header = ""
        
        for i, section in enumerate(sections):
            if re.match(r'#{1,6}\s+', section):
                # This is a header
                if current_section:
                    chunks.append({
                        "content": current_section.strip(),
                        "type": "documentation",
                        "context": current_header,
                        "header_level": len(section.split()[0])
                    })
                current_header = section.strip()
                current_section = ""
            else:
                current_section += section
        
        # Add final section
        if current_section:
            chunks.append({
                "content": current_section.strip(),
                "type": "documentation", 
                "context": current_header,
                "header_level": 1
            })
        
        return chunks
    
    def scan_workspace_files(self) -> Dict[str, List[Path]]:
        """Scan workspace and categorize files by collection"""
        file_mapping = {}
        
        for collection_name, config in self.collection_config.items():
            file_mapping[collection_name] = []
            
            for path_pattern in config["paths"]:
                base_path = WORKSPACE_ROOT / path_pattern
                
                if base_path.is_dir():
                    # Scan directory
                    extensions = config.get("extensions", [".md", ".txt", ".rst"])
                    exclude_dirs = config.get("exclude", [])
                    
                    for ext in extensions:
                        for file_path in base_path.rglob(f"*{ext}"):
                            # Check if file should be excluded
                            if not any(exc in str(file_path) for exc in exclude_dirs):
                                file_mapping[collection_name].append(file_path)
                
                elif base_path.is_file():
                    # Single file
                    file_mapping[collection_name].append(base_path)
            
            # Add specific include files
            include_files = config.get("include_files", [])
            for include_file in include_files:
                file_path = WORKSPACE_ROOT / include_file
                if file_path.exists():
                    file_mapping[collection_name].append(file_path)
        
        return file_mapping
    
    def rebuild_comprehensive_database(self):
        """Rebuild semantic database with comprehensive workspace coverage"""
        print("🚀 Starting comprehensive workspace semantic search rebuild...")
        
        # Clear existing database
        print("🗑️  Clearing existing collections...")
        existing_collections = self.client.list_collections()
        for collection in existing_collections:
            self.client.delete_collection(collection.name)
        
        # Scan workspace files
        print("📂 Scanning workspace files...")
        file_mapping = self.scan_workspace_files()
        
        total_files = sum(len(files) for files in file_mapping.values())
        print(f"📊 Found {total_files} files across {len(file_mapping)} collections")
        
        # Process each collection
        for collection_name, files in file_mapping.items():
            if not files:
                print(f"⚠️  No files found for collection: {collection_name}")
                continue
                
            print(f"\n🔨 Processing collection: {collection_name} ({len(files)} files)")
            
            # Create collection
            collection = self.client.create_collection(
                name=collection_name,
                metadata={"description": self.collection_config[collection_name]["description"]}
            )
            
            documents = []
            metadatas = []
            ids = []
            
            for file_path in files:
                try:
                    # Read file content
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    
                    # Choose chunking strategy based on file type
                    if file_path.suffix in ['.c', '.h']:
                        chunks = self.chunk_source_code(content, str(file_path))
                    else:
                        chunks = self.chunk_documentation(content, str(file_path))
                    
                    # Add chunks to collection
                    for i, chunk in enumerate(chunks):
                        chunk_id = hashlib.md5(f"{file_path}_{i}".encode()).hexdigest()
                        
                        documents.append(chunk["content"])
                        metadatas.append({
                            "source_file": str(file_path.relative_to(WORKSPACE_ROOT)),
                            "chunk_type": chunk["type"],
                            "context": chunk.get("context", ""),
                            "file_type": file_path.suffix,
                            "collection": collection_name
                        })
                        ids.append(chunk_id)
                
                except Exception as e:
                    print(f"⚠️  Error processing {file_path}: {e}")
                    continue
            
            # Add to ChromaDB in batches
            batch_size = 50  # Reduced batch size for Instructor-XL memory efficiency
            for i in range(0, len(documents), batch_size):
                batch_docs = documents[i:i+batch_size]
                batch_metas = metadatas[i:i+batch_size]
                batch_ids = ids[i:i+batch_size]
                
                print(f"    🔄 Processing batch {i//batch_size + 1}/{(len(documents)-1)//batch_size + 1}")
                
                # Generate embeddings with domain-specific instructions
                embeddings = [self.get_embedding(doc, collection_name) for doc in batch_docs]
                
                # Add with custom embeddings
                collection.add(
                    documents=batch_docs,
                    metadatas=batch_metas,
                    embeddings=embeddings,
                    ids=batch_ids
                )
            
            print(f"✅ Added {len(documents)} chunks to {collection_name}")
        
        print("\n🎉 Comprehensive semantic search database rebuild complete!")
        
        # Show final statistics
        self.show_database_statistics()
    
    def show_database_statistics(self):
        """Display comprehensive database statistics"""
        print("\n📊 COMPREHENSIVE DATABASE STATISTICS:")
        collections = self.client.list_collections()
        total_docs = 0
        
        for collection in collections:
            count = collection.count()
            total_docs += count
            print(f"  {collection.name}: {count:,} documents")
        
        print(f"\n🎯 Total indexed: {total_docs:,} documents")
        improvement = round(total_docs / 981, 1) if total_docs > 981 else 1.0
        print(f"📈 Coverage increase: {total_docs:,} vs 981 original ({improvement}x improvement)")

if __name__ == "__main__":
    indexer = WorkspaceSemanticIndexer()
    indexer.rebuild_comprehensive_database()
