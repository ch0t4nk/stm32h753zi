#!/usr/bin/env python3
"""
Performance validation tests for high-speed semantic search
Validates FTR-014 acceptance criteria for sub-2-second queries
"""

import time
import statistics
from pathlib import Path
import sys
import os

# Add project root to path for imports
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root / "scripts"))

try:
    from collection_manager import FastCollectionManager
except ImportError:
    print("❌ Could not import collection_manager - ensure scripts are available")
    sys.exit(1)


class SemanticPerformanceValidator:
    """Validate semantic search performance against FTR-014 requirements"""
    
    def __init__(self):
        self.collection_manager = FastCollectionManager()
        self.performance_results = {}
        
    def run_performance_tests(self):
        """Run comprehensive performance validation"""
        print("🚀 High-Performance Semantic Search Validation")
        print("=" * 60)
        print("FTR-014 Target: <2 second query response time")
        print("=" * 60)
        
        # Test cases targeting different scopes and complexities
        test_cases = [
            ("GPIO configuration", "stm32h7", "Simple HAL query"),
            ("L6470 register configuration", "l6470", "Motor driver specific"),
            ("BSP_LED_Init function", "nucleo_bsp", "BSP function lookup"),
            ("stepper motor control algorithms", "all", "Cross-collection search"),
            ("safety system implementation", "project", "Project code search"),
            ("FreeRTOS task configuration", "instructions", "Instruction search")
        ]
        
        all_times = []
        
        for query, scope, description in test_cases:
            print(f"\n🔍 Testing: {description}")
            print(f"   Query: '{query}' (scope: {scope})")
            
            # Warm-up query (not counted)
            self.collection_manager.search_collections(query, scope, max_results=5)
            
            # Timed query runs
            query_times = []
            results = []  # Initialize results
            for run in range(3):
                start_time = time.time()
                results = self.collection_manager.search_collections(query, scope, max_results=5)
                query_time = time.time() - start_time
                query_times.append(query_time)
                
            avg_time = statistics.mean(query_times)
            min_time = min(query_times)
            max_time = max(query_times)
            
            all_times.extend(query_times)
            
            # Performance assessment
            status = "✅ PASS" if avg_time < 2.0 else "❌ FAIL"
            print(f"   Results: {len(results)} items found")
            print(f"   Time: {avg_time:.3f}s avg, {min_time:.3f}s min, {max_time:.3f}s max {status}")
            
            # Store results
            self.performance_results[description] = {
                "query": query,
                "scope": scope,
                "avg_time": avg_time,
                "min_time": min_time,
                "max_time": max_time,
                "results_count": len(results),
                "pass": avg_time < 2.0
            }
            
        # Overall performance summary
        self.print_performance_summary(all_times)
        
        # Collection loading performance
        self.test_collection_loading_performance()
        
        return self.performance_results
        
    def print_performance_summary(self, all_times):
        """Print overall performance statistics"""
        print(f"\n📊 Overall Performance Summary")
        print("=" * 40)
        
        avg_time = statistics.mean(all_times)
        median_time = statistics.median(all_times)
        min_time = min(all_times)
        max_time = max(all_times)
        
        print(f"Average query time: {avg_time:.3f}s")
        print(f"Median query time:  {median_time:.3f}s")
        print(f"Fastest query:      {min_time:.3f}s")
        print(f"Slowest query:      {max_time:.3f}s")
        
        # FTR-014 compliance check
        passing_queries = sum(1 for t in all_times if t < 2.0)
        total_queries = len(all_times)
        pass_rate = (passing_queries / total_queries) * 100
        
        overall_status = "✅ PASS" if pass_rate >= 90 else "❌ FAIL"
        print(f"\nFTR-014 Compliance: {pass_rate:.1f}% queries <2s {overall_status}")
        
        if avg_time < 2.0:
            improvement = ((25.0 - avg_time) / 25.0) * 100
            print(f"Performance improvement: {improvement:.1f}% (from 25s baseline)")
            
    def test_collection_loading_performance(self):
        """Test collection loading and caching performance"""
        print(f"\n🏗️  Collection Loading Performance")
        print("=" * 40)
        
        # Test cold load
        manager = FastCollectionManager()  # Fresh instance
        start_time = time.time()
        collection = manager.load_collection_lazy("stm32_hal")
        cold_load_time = time.time() - start_time
        
        # Test cached load
        start_time = time.time()
        cached_collection = manager.load_collection_lazy("stm32_hal")
        cache_load_time = time.time() - start_time
        
        print(f"Cold load time:   {cold_load_time:.3f}s")
        print(f"Cached load time: {cache_load_time:.3f}s")
        
        cache_speedup = cold_load_time / cache_load_time if cache_load_time > 0 else float('inf')
        print(f"Cache speedup:    {cache_speedup:.1f}x")
        
        # Get performance stats
        stats = manager.get_performance_stats()
        print(f"Cache hit ratio:  {stats['cache_hits']}/{stats['queries_served']}")
        
    def validate_database_integrity(self):
        """Quick validation that database is accessible"""
        print(f"\n🔍 Database Integrity Check")
        print("=" * 30)
        
        if not self.collection_manager.validate_database():
            print("❌ Database validation failed!")
            return False
            
        print("✅ Database validation passed")
        return True
        
    def run_comprehensive_validation(self):
        """Run all validation tests"""
        print("🧪 FTR-014 Comprehensive Validation Suite")
        print("=" * 70)
        
        # Database integrity
        if not self.validate_database_integrity():
            return False
            
        # Performance tests
        results = self.run_performance_tests()
        
        # Final assessment
        all_pass = all(result["pass"] for result in results.values())
        
        print(f"\n🎯 Final Assessment")
        print("=" * 30)
        if all_pass:
            print("✅ FTR-014 VALIDATION PASSED")
            print("   All queries complete in <2 seconds")
            print("   High-performance semantic search ready for production")
        else:
            print("❌ FTR-014 VALIDATION FAILED")
            print("   Some queries exceed 2-second target")
            print("   Additional optimization required")
            
        return all_pass


def main():
    """Run performance validation"""
    validator = SemanticPerformanceValidator()
    success = validator.run_comprehensive_validation()
    
    return 0 if success else 1


if __name__ == "__main__":
    exit(main())
