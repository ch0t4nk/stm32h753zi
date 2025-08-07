#!/usr/bin/env python3
"""
Comprehensive Workspace Semantic Search Rebuild (Clean Version)
Indexes the ENTIRE STM32H753ZI workspace for AI-powered assistance with suppressed warnings
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
    
    # Check GPU availability with controlled messaging
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
    torch = None
    INSTRUCTOR = None
    if not SUPPRESS_OLLAMA_CONNECTION_WARNINGS:
        print("⚠️  InstructorEmbedding not available - falling back to Ollama")

WORKSPACE_ROOT = Path("/workspaces/code")
SEMANTIC_DB_PATH = WORKSPACE_ROOT / "docs" / "semantic_vector_db"

class WorkspaceSemanticIndexer:
    """Comprehensive workspace indexer for semantic search"""
    
    def __init__(self):
        print("🔄 Initializing Comprehensive Workspace Semantic Indexer...")
        self.client = chromadb.PersistentClient(path=str(SEMANTIC_DB_PATH))
        self.collections = {}
        
        # Initialize Instructor-XL if available
        self.instructor_model = None
        if INSTRUCTOR_AVAILABLE and torch is not None and INSTRUCTOR is not None:
            try:
                if not SUPPRESS_GPU_AVAILABILITY_WARNINGS:
                    print("🔄 Loading Instructor-XL model (hkunlp/instructor-xl)...")
                    print("   Note: Initial setup may show 'Creating new model' - this uses cached data")
                
                # Force GPU usage if available  
                device = "cuda" if torch.cuda.is_available() else "cpu"
                
                # Set cache directory explicitly to avoid confusion
                import os
                os.environ["TRANSFORMERS_CACHE"] = os.path.expanduser("~/.cache/huggingface")
                
                self.instructor_model = INSTRUCTOR('hkunlp/instructor-xl', device=device, cache_folder=os.path.expanduser("~/.cache/huggingface"))
                
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
        
        print(f"✅ Indexer initialized with {len(self.collection_config)} collections")
    
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
                    if embedding_time > 2.0 and not SUPPRESS_GPU_AVAILABILITY_WARNINGS:  # Log slow embeddings
                        print(f"    🐌 Slow embedding: {embedding_time:.2f}s for {len(text)} chars")
                    return embedding[0].tolist()
                    
            except Exception as e:
                if not SUPPRESS_OLLAMA_CONNECTION_WARNINGS:
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
    
    def scan_workspace_files(self) -> Dict[str, List[Path]]:
        """Scan entire workspace and categorize files for indexing"""
        print("🔍 Scanning entire workspace for files...")
        
        all_files = {}
        total_files = 0
        
        for collection_name, config in self.collection_config.items():
            all_files[collection_name] = []
            
            for path_str in config["paths"]:
                path = WORKSPACE_ROOT / path_str
                if not path.exists():
                    continue
                    
                if path.is_file():
                    all_files[collection_name].append(path)
                    total_files += 1
                    continue
                    
                # Scan directory
                extensions = config.get("extensions", [".md"])
                exclude_dirs = config.get("exclude", [])
                include_files = config.get("include_files", [])
                
                for file_path in path.rglob("*"):
                    if file_path.is_file():
                        # Check if in excluded directory
                        if any(exclude in str(file_path) for exclude in exclude_dirs):
                            continue
                            
                        # Check if specific file to include
                        if include_files and file_path.name in include_files:
                            all_files[collection_name].append(file_path)
                            total_files += 1
                            continue
                            
                        # Check extension
                        if file_path.suffix in extensions:
                            all_files[collection_name].append(file_path)
                            total_files += 1
        
        print(f"📊 Workspace scan complete:")
        for collection_name, files in all_files.items():
            print(f"   {collection_name}: {len(files)} files")
        print(f"   Total: {total_files} files")
        
        return all_files
    
    def chunk_source_code(self, content: str, file_path: str) -> List[Dict]:
        """Smart chunking for source code files"""
        chunks = []
        lines = content.split('\n')
        
        # Extract functions, structs, and key definitions
        current_chunk = []
        current_function = None
        brace_count = 0
        
        for i, line in enumerate(lines):
            current_chunk.append(line)
            
            # Track function definitions
            if re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(', line.strip()):
                current_function = line.strip()
            
            # Track braces
            brace_count += line.count('{') - line.count('}')
            
            # End of function or significant section
            if (brace_count == 0 and current_function and len(current_chunk) > 5) or len(current_chunk) >= 50:
                chunk_text = '\n'.join(current_chunk)
                chunks.append({
                    'content': chunk_text,
                    'metadata': {
                        'file_path': str(file_path),
                        'function': current_function or 'global',
                        'start_line': i - len(current_chunk) + 1,
                        'end_line': i,
                        'type': 'source_code'
                    }
                })
                current_chunk = []
                current_function = None
        
        # Add remaining content
        if current_chunk:
            chunk_text = '\n'.join(current_chunk)
            chunks.append({
                'content': chunk_text,
                'metadata': {
                    'file_path': str(file_path),
                    'function': current_function or 'global',
                    'start_line': len(lines) - len(current_chunk),
                    'end_line': len(lines),
                    'type': 'source_code'
                }
            })
        
        return chunks
    
    def chunk_documentation(self, content: str, file_path: str) -> List[Dict]:
        """Smart chunking for documentation files"""
        chunks = []
        
        # Split by headers for documentation
        sections = re.split(r'\n(#{1,6}\s+.*?)\n', content)
        
        current_section = ""
        current_header = "Introduction"
        
        for i, section in enumerate(sections):
            if re.match(r'^#{1,6}\s+', section):
                # This is a header
                if current_section.strip():
                    chunks.append({
                        'content': current_section,
                        'metadata': {
                            'file_path': str(file_path),
                            'section': current_header,
                            'type': 'documentation'
                        }
                    })
                current_header = section.strip()
                current_section = section + "\n"
            else:
                current_section += section
        
        # Add final section
        if current_section.strip():
            chunks.append({
                'content': current_section,
                'metadata': {
                    'file_path': str(file_path),
                    'section': current_header,
                    'type': 'documentation'
                }
            })
        
        return chunks
    
    def process_file(self, file_path: Path, collection_name: str) -> List[Dict]:
        """Process a single file and return chunks"""
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            if not content.strip():
                return []
            
            # Choose chunking strategy based on file type
            if file_path.suffix in ['.c', '.h']:
                return self.chunk_source_code(content, str(file_path))
            else:
                return self.chunk_documentation(content, str(file_path))
                
        except Exception as e:
            if not SUPPRESS_OLLAMA_CONNECTION_WARNINGS:
                print(f"    ⚠️  Error processing {file_path}: {e}")
            return []
    
    def rebuild_comprehensive_database(self):
        """Rebuild the entire semantic search database with all workspace files"""
        print("🚀 Starting Comprehensive Workspace Semantic Search Rebuild...")
        print(f"   Target: All files across {len(self.collection_config)} collections")
        print(f"   Method: {'Instructor-XL GPU' if self.instructor_model and torch and torch.cuda.is_available() else 'Instructor-XL CPU' if self.instructor_model else 'Ollama/Mock'}")
        
        # Scan all workspace files
        all_files = self.scan_workspace_files()
        
        # Process each collection
        for collection_name, files in all_files.items():
            if not files:
                print(f"⏭️  Skipping {collection_name} (no files found)")
                continue
                
            print(f"\n📚 Processing collection: {collection_name}")
            print(f"   Files: {len(files)}")
            
            # Create or recreate collection
            try:
                self.client.delete_collection(collection_name)
            except:
                pass
            
            collection = self.client.create_collection(
                name=collection_name,
                metadata={"description": self.collection_config[collection_name]["description"]}
            )
            
            # Process files in batches
            batch_size = 20 if (self.instructor_model and torch and torch.cuda.is_available()) else 10
            all_documents = []
            
            for file_path in files:
                chunks = self.process_file(file_path, collection_name)
                all_documents.extend(chunks)
            
            print(f"   Documents: {len(all_documents)}")
            
            # Add documents in batches
            total_batches = (len(all_documents) + batch_size - 1) // batch_size
            
            for batch_idx in range(0, len(all_documents), batch_size):
                batch_docs = all_documents[batch_idx:batch_idx + batch_size]
                batch_num = (batch_idx // batch_size) + 1
                
                if batch_num % 10 == 0:  # Only show every 10th batch progress
                    print(f"    🔄 Processing batch {batch_num}/{total_batches}")
                
                # Generate embeddings for batch
                embeddings = []
                ids = []
                documents = []
                metadatas = []
                
                for doc in batch_docs:
                    embedding = self.get_embedding(doc['content'], collection_name)
                    embeddings.append(embedding)
                    
                    # Generate unique ID with file path, content hash, and batch info
                    content_hash = hashlib.md5(doc['content'].encode()).hexdigest()
                    metadata_str = str(doc.get('metadata', {}))
                    file_path = doc.get('metadata', {}).get('file_path', 'unknown')
                    unique_string = f"{file_path}:{content_hash}:{metadata_str}:{batch_idx}:{len(embeddings)}"
                    doc_id = hashlib.md5(unique_string.encode()).hexdigest()
                    
                    ids.append(doc_id)
                    documents.append(doc['content'])
                    metadatas.append(doc.get('metadata', {}))
                
                # Add to collection
                collection.add(
                    ids=ids,
                    documents=documents,
                    metadatas=metadatas,
                    embeddings=embeddings
                )
            
            print(f"✅ Collection {collection_name} completed: {len(all_documents)} documents indexed")
        
        print(f"\n🎉 Comprehensive workspace semantic search rebuild complete!")
        print(f"   Collections: {len(self.collection_config)}")
        
        # Final statistics
        total_docs = 0
        for collection_name in self.collection_config.keys():
            try:
                collection = self.client.get_collection(collection_name)
                count = collection.count()
                total_docs += count
                print(f"   {collection_name}: {count} documents")
            except:
                print(f"   {collection_name}: Failed to get count")
        
        print(f"   Total: {total_docs} documents indexed")
        print(f"   Database: {SEMANTIC_DB_PATH}")

if __name__ == "__main__":
    indexer = WorkspaceSemanticIndexer()
    indexer.rebuild_comprehensive_database()
