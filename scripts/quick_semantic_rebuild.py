#!/usr/bin/env python3
"""
Quick Comprehensive Semantic Search Rebuild
Extends current semantic database to include entire workspace
"""

import chromadb
import requests
from pathlib import Path
import hashlib
import re

WORKSPACE_ROOT = Path("/workspaces/code")
SEMANTIC_DB_PATH = WORKSPACE_ROOT / "docs" / "semantic_vector_db"

def get_ollama_embedding(text):
    """Generate embedding using Ollama or fallback"""
    try:
        response = requests.post(
            "http://localhost:11434/api/embeddings",
            json={"model": "mxbai-embed-large", "prompt": text},
            timeout=10
        )
        if response.status_code == 200:
            return response.json().get("embedding", [])
    except:
        pass
    
    # Fallback mock embedding
    return [float(hash(text + str(i)) % 1000) / 1000.0 for i in range(1024)]

def quick_rebuild():
    """Quick rebuild adding our critical missing content"""
    print("🚀 Quick comprehensive semantic search rebuild...")
    
    client = chromadb.PersistentClient(path=str(SEMANTIC_DB_PATH))
    
    # New collections for our workspace
    new_collections = [
        {
            "name": "project_source",
            "description": "Production source code: HAL abstraction, drivers, controllers",
            "paths": ["src/"],
            "extensions": [".c", ".h"]
        },
        {
            "name": "instruction_guides", 
            "description": "Modernized development instructions",
            "paths": [".github/instructions/"],
            "extensions": [".md"]
        },
        {
            "name": "project_documentation",
            "description": "Analysis reports, design docs",
            "paths": ["docs/"],
            "extensions": [".md"],
            "exclude": ["semantic_vector_db"]
        },
        {
            "name": "automation_scripts",
            "description": "Python automation and build scripts",
            "paths": ["scripts/"],
            "extensions": [".py", ".sh"]
        }
    ]
    
    total_added = 0
    
    for config in new_collections:
        print(f"\n🔨 Processing: {config['name']}")
        
        # Try to get existing collection or create new
        try:
            collection = client.get_collection(config["name"])
            print(f"  📋 Found existing collection with {collection.count()} documents")
            client.delete_collection(config["name"])
            print("  🗑️  Cleared for rebuild")
        except:
            pass
        
        collection = client.create_collection(
            name=config["name"],
            metadata={"description": config["description"]}
        )
        
        # Find files
        files = []
        for path_pattern in config["paths"]:
            base_path = WORKSPACE_ROOT / path_pattern
            if base_path.exists():
                for ext in config["extensions"]:
                    found_files = list(base_path.rglob(f"*{ext}"))
                    # Filter out excluded directories
                    if "exclude" in config:
                        found_files = [f for f in found_files 
                                     if not any(exc in str(f) for exc in config["exclude"])]
                    files.extend(found_files)
        
        print(f"  📂 Found {len(files)} files")
        
        # Process files in batches
        documents = []
        metadatas = []
        ids = []
        
        for file_path in files:
            try:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                
                # Simple chunking by size for quick rebuild
                chunks = [content[i:i+2000] for i in range(0, len(content), 1500)]
                
                for i, chunk in enumerate(chunks):
                    if len(chunk.strip()) < 50:  # Skip tiny chunks
                        continue
                        
                    chunk_id = hashlib.md5(f"{file_path}_{i}".encode()).hexdigest()
                    
                    documents.append(chunk)
                    metadatas.append({
                        "source_file": str(file_path.relative_to(WORKSPACE_ROOT)),
                        "file_type": file_path.suffix,
                        "collection": config["name"],
                        "chunk_index": i
                    })
                    ids.append(chunk_id)
                    
            except Exception as e:
                print(f"    ⚠️  Error with {file_path.name}: {e}")
        
        # Add to collection in batches
        if documents:
            batch_size = 50
            for i in range(0, len(documents), batch_size):
                batch_docs = documents[i:i+batch_size]
                batch_metas = metadatas[i:i+batch_size]
                batch_ids = ids[i:i+batch_size]
                
                collection.add(
                    documents=batch_docs,
                    metadatas=batch_metas,
                    ids=batch_ids
                )
            
            print(f"  ✅ Added {len(documents)} chunks")
            total_added += len(documents)
        else:
            print("  ⚠️  No content added")
    
    # Show final stats
    print(f"\n🎉 Rebuild complete! Added {total_added} new documents")
    
    print("\n📊 UPDATED DATABASE STATISTICS:")
    collections = client.list_collections()
    total_docs = 0
    
    for collection in collections:
        count = collection.count()
        total_docs += count
        print(f"  {collection.name}: {count:,} documents")
    
    print(f"\n🎯 Total: {total_docs:,} documents")
    improvement = round(total_docs / 981, 1) if total_docs > 981 else 1.0
    print(f"📈 Improvement: {improvement}x over original 981 documents")

if __name__ == "__main__":
    quick_rebuild()
