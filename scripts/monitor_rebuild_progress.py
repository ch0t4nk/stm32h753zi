#!/usr/bin/env python3
"""
Quick Database Progress Monitor
Monitor semantic database rebuild progress without interfering
"""

import chromadb
from pathlib import Path
import time
import subprocess

SEMANTIC_DB_PATH = Path("/workspaces/code/docs/semantic_vector_db")

def get_database_status():
    """Get quick database status"""
    try:
        client = chromadb.PersistentClient(path=str(SEMANTIC_DB_PATH))
        collections = client.list_collections()
        
        total_docs = 0
        collection_info = {}
        
        for collection in collections:
            try:
                coll = client.get_collection(collection.name)
                count = coll.count()
                total_docs += count
                collection_info[collection.name] = count
            except Exception as e:
                collection_info[collection.name] = f"Error: {e}"
        
        # Get database size
        result = subprocess.run(['du', '-sh', str(SEMANTIC_DB_PATH)], 
                              capture_output=True, text=True)
        db_size = result.stdout.split()[0] if result.returncode == 0 else "Unknown"
        
        return {
            "total_documents": total_docs,
            "collections": collection_info,
            "database_size": db_size,
            "collection_count": len(collections)
        }
        
    except Exception as e:
        return {"error": str(e)}

def check_rebuild_process():
    """Check if rebuild process is still running"""
    try:
        result = subprocess.run(['pgrep', '-f', 'rebuild_comprehensive'], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            pid = result.stdout.strip()
            # Get CPU and memory usage
            ps_result = subprocess.run(['ps', 'aux'], capture_output=True, text=True)
            for line in ps_result.stdout.split('\n'):
                if pid in line and 'rebuild_comprehensive' in line:
                    parts = line.split()
                    cpu_percent = parts[2]
                    mem_percent = parts[3]
                    return {
                        "running": True,
                        "pid": pid,
                        "cpu_percent": cpu_percent,
                        "memory_percent": mem_percent
                    }
            return {"running": True, "pid": pid}
        else:
            return {"running": False}
    except Exception as e:
        return {"running": False, "error": str(e)}

if __name__ == "__main__":
    print("🔄 Database Progress Monitor")
    print("=" * 50)
    
    # Check rebuild process
    process_status = check_rebuild_process()
    if process_status["running"]:
        print(f"✅ Rebuild Process Active:")
        print(f"   PID: {process_status.get('pid', 'Unknown')}")
        print(f"   CPU: {process_status.get('cpu_percent', 'Unknown')}%")
        print(f"   Memory: {process_status.get('memory_percent', 'Unknown')}%")
    else:
        print("⏸️  Rebuild Process: Not running")
    
    print()
    
    # Check database status
    db_status = get_database_status()
    if "error" not in db_status:
        print(f"📊 Database Status:")
        print(f"   Total Documents: {db_status['total_documents']:,}")
        print(f"   Collections: {db_status['collection_count']}")
        print(f"   Database Size: {db_status['database_size']}")
        print()
        print(f"📚 Collection Details:")
        collections = db_status.get('collections', {})
        if isinstance(collections, dict):
            for name, count in collections.items():
                print(f"   {name}: {count:,}" if isinstance(count, int) else f"   {name}: {count}")
    else:
        print(f"❌ Database Error: {db_status['error']}")
    
    print()
    print("🎯 Expected Collections (9 total):")
    expected = [
        "stm32_hal", "motor_control", "project_source", 
        "instruction_guides", "project_documentation", "build_system",
        "automation_scripts", "api_specifications", "workspace_config"
    ]
    
    if "error" not in db_status:
        collections = db_status.get('collections', {})
        if isinstance(collections, dict):
            current_collections = set(collections.keys())
            for collection in expected:
                if collection in current_collections:
                    count = collections[collection]
                    status = f"{count:,} docs" if isinstance(count, int) else "Error"
                    print(f"   ✅ {collection}: {status}")
                else:
                    print(f"   ⏳ {collection}: Pending")
        else:
            for collection in expected:
                print(f"   ⏳ {collection}: Unknown")
    else:
        for collection in expected:
            print(f"   ⏳ {collection}: Unknown")
