#!/usr/bin/env python3
"""
STM32 Semantic Search - Lightweight Client
High-performance client for the background semantic search service.
Replaces heavy database operations with fast HTTP requests.

Author: STM32H753ZI Development Team  
Date: August 11, 2025
Version: 2.0 (Lightweight Client)
"""

import sys
import argparse
import requests
import json
from pathlib import Path
from typing import List, Dict, Optional, Any
from dataclasses import dataclass
from datetime import datetime

# SSOT Documentation Configuration
WORKSPACE_ROOT = Path("/workspaces/code")

# Service Configuration
DEFAULT_SERVICE_URL = "http://localhost:8080"
SERVICE_TIMEOUT = 30  # seconds


@dataclass
class SearchResult:
    """Lightweight search result structure"""
    content: str
    source_file: str
    similarity_score: float
    collection: str
    metadata: Dict[str, Any]

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'SearchResult':
        """Create SearchResult from service response"""
        return cls(
            content=data.get('content', ''),
            source_file=data.get('source_file', 'unknown'),
            similarity_score=data.get('similarity_score', 0.0),
            collection=data.get('collection', 'unknown'),
            metadata=data.get('metadata', {})
        )


class SemanticSearchClient:
    """Lightweight client for the semantic search service"""
    
    def __init__(self, service_url: str = DEFAULT_SERVICE_URL):
        self.service_url = service_url.rstrip('/')
        self.query_endpoint = f"{self.service_url}/query" 
        self.status_endpoint = f"{self.service_url}/status"
        self.health_endpoint = f"{self.service_url}/health"
        
    def check_service_status(self) -> bool:
        """Check if the semantic search service is running"""
        try:
            response = requests.get(self.health_endpoint, timeout=5)
            return response.status_code == 200
        except requests.exceptions.RequestException:
            return False
    
    def get_service_info(self) -> Optional[Dict[str, Any]]:
        """Get detailed service status information"""
        try:
            response = requests.get(self.status_endpoint, timeout=5)
            if response.status_code == 200:
                return response.json()
            return None
        except requests.exceptions.RequestException:
            return None
    
    def search(self, query: str, scope: str = "all", max_results: int = 10) -> List[SearchResult]:
        """Perform semantic search via the background service"""
        print(f"🔍 Searching: '{query}' (scope: {scope})")
        
        # Map legacy scopes to service categories
        scope_mapping = {
            "STM32H7": "hal", 
            "L6470": "motor",
            "NUCLEO_BSP": "hal",
            "all": "all",
        }
        category = scope_mapping.get(scope.upper(), "all")
        
        # Prepare request payload
        payload = {
            "query": query,
            "scope": category,
            "max_results": max_results
        }
        
        try:
            # Send request to service
            response = requests.post(
                self.query_endpoint, 
                json=payload, 
                timeout=SERVICE_TIMEOUT
            )
            response.raise_for_status()
            
            # Parse response
            result_data = response.json()
            
            # Handle different response formats
            if isinstance(result_data, dict):
                # Service returns QueryResult format
                results_list = result_data.get('results', [])
                response_time = result_data.get('response_time_ms', 0)
                cache_hit = result_data.get('cache_hit', False)
                
                print(f"⚡ Response time: {response_time:.1f}ms (cache: {cache_hit})")
            else:
                # Service returns direct results list
                results_list = result_data
                
            # Convert to SearchResult objects
            search_results = []
            for item in results_list:
                if isinstance(item, dict):
                    # Handle service response format
                    if 'document' in item:
                        # SemanticSearchService format
                        result = SearchResult(
                            content=item.get('document', ''),
                            source_file=item.get('metadata', {}).get('source_file', 'unknown'),
                            similarity_score=1.0 - item.get('distance', 1.0),
                            collection=item.get('collection', 'unknown'),
                            metadata=item.get('metadata', {})
                        )
                    else:
                        # Direct format
                        result = SearchResult.from_dict(item)
                    search_results.append(result)
            
            return search_results
            
        except requests.exceptions.ConnectionError:
            print("❌ Connection to search service failed")
            print("   Please start the service: `scripts/start_ai_services.sh`")
            return []
        except requests.exceptions.Timeout:
            print(f"❌ Search request timed out after {SERVICE_TIMEOUT}s")
            return []
        except requests.exceptions.HTTPError as e:
            print(f"❌ Service error: {e.response.status_code}")
            try:
                error_detail = e.response.json()
                print(f"   Details: {error_detail}")
            except json.JSONDecodeError:
                print("   (No error details available)")
            return []
        except Exception as e:
            print(f"❌ Unexpected error: {e}")
            return []


def print_search_results(results: List[SearchResult], show_content: bool = True) -> None:
    """Print search results in a formatted way"""
    if not results:
        print("⚠️  No results found")
        return
        
    print(f"✅ Found {len(results)} results:")
    print("-" * 80)
    
    for i, result in enumerate(results, 1):
        print(f"{i}. 📄 {result.source_file}")
        print(f"   📁 Collection: {result.collection}")
        print(f"   📊 Similarity: {result.similarity_score:.3f}")
        
        if show_content:
            content = result.content
            if len(content) > 150:
                content = content[:150] + "..."
            print(f"   📝 Content: {content}")
        
        # Show interesting metadata
        if result.metadata:
            interesting_keys = ['category', 'chunk_type', 'line_start', 'line_end']
            metadata_display = {k: v for k, v in result.metadata.items() if k in interesting_keys and v}
            if metadata_display:
                print(f"   🏷️  Metadata: {metadata_display}")
        print()


def enhance_query_by_type(query: str, search_type: str) -> str:
    """Enhance query based on search type"""
    if search_type == "function":
        return f"function {query} implementation usage"
    elif search_type == "peripheral":
        return f"{query} peripheral configuration registers"  
    elif search_type == "register":
        return f"{query} register configuration bits"
    elif search_type == "concept":
        return f"{query}"
    else:
        return query


def main():
    """Main CLI interface for the lightweight semantic search client"""
    parser = argparse.ArgumentParser(
        description="STM32 Semantic Search - Lightweight Client",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s concept "GPIO configuration" --scope STM32H7
  %(prog)s function "HAL_GPIO_Init" --scope STM32H7  
  %(prog)s peripheral "SPI" --scope all
  %(prog)s register "ABS_POS" --scope L6470
  %(prog)s status  # Check service status
        """
    )
    
    parser.add_argument(
        "search_type",
        choices=["concept", "function", "peripheral", "register", "test", "status"],
        help="Type of search to perform, or 'status' to check service"
    )
    parser.add_argument(
        "query", 
        nargs='?', 
        default='', 
        help="Search query (not required for 'status')"
    )
    parser.add_argument(
        "--scope",
        choices=["STM32H7", "L6470", "NUCLEO_BSP", "motor_control", "all"],
        default="all",
        help="Search scope (default: all)"
    )
    parser.add_argument(
        "--max-results", 
        type=int, 
        default=5, 
        help="Maximum results to return (default: 5)"
    )
    parser.add_argument(
        "--service-url",
        default=DEFAULT_SERVICE_URL,
        help=f"Service URL (default: {DEFAULT_SERVICE_URL})"
    )
    parser.add_argument(
        "--no-content",
        action="store_true",
        help="Don't show content snippets in results"
    )
    
    args = parser.parse_args()
    
    # Initialize client
    client = SemanticSearchClient(args.service_url)
    
    # Handle status command
    if args.search_type == "status":
        print("🔍 Checking semantic search service status...")
        
        if client.check_service_status():
            print("✅ Service is running and responsive")
            
            # Get detailed status
            info = client.get_service_info()
            if info:
                print("\n📊 Service Information:")
                
                # Service status
                service_info = info.get('service', {})
                print(f"   Uptime: {service_info.get('uptime_seconds', 0):.1f}s")
                
                # Performance metrics
                perf_info = info.get('performance', {})
                print(f"   Queries: {perf_info.get('query_count', 0)}")
                print(f"   Avg Response: {perf_info.get('avg_response_time_ms', 0):.1f}ms")
                print(f"   Cache Hit Rate: {perf_info.get('cache_hit_rate', 0):.1%}")
                
                # Collections
                collections_info = info.get('collections', {})
                print(f"   Collections Loaded: {collections_info.get('loaded', 0)}")
                
                # System resources
                system_info = info.get('system', {})
                print(f"   Memory Usage: {system_info.get('memory_usage_mb', 0):.1f}MB")
                
        else:
            print("❌ Service is not running")
            print("   Start with: scripts/start_ai_services.sh")
            return 1
            
        return 0
    
    # Validate query for non-status commands
    if not args.query:
        print("Error: Query argument is required for this search type")
        parser.print_help()
        return 1
    
    # Enhance query based on search type
    enhanced_query = enhance_query_by_type(args.query, args.search_type)
    
    # Perform search
    results = client.search(enhanced_query, args.scope, args.max_results)
    
    # Display results
    print_search_results(results, show_content=not args.no_content)
    
    return 0 if results else 1


if __name__ == "__main__":
    sys.exit(main())
