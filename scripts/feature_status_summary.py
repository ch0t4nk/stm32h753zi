#!/usr/bin/env python3
"""
Feature integration script for STATUS.md automation
Provides feature status updates for inclusion in automated STATUS.md updates
"""

import json
from pathlib import Path

def get_feature_summary():
    """Get concise feature summary for STATUS.md integration"""
    registry_path = Path("features/feature_registry.json")
    
    if not registry_path.exists():
        return "Feature registry not found"
    
    with open(registry_path, 'r') as f:
        registry = json.load(f)
    
    metadata = registry['metadata']
    
    # Count by status
    completed = metadata['completed_features']
    in_progress = metadata['in_progress_features'] 
    planned = metadata['planned_features']
    total = metadata['total_features']
    
    # Current phase progress
    features = registry['features']
    phase_1_4_features = [f for f in features.values() if f['phase'] == '1.4']
    phase_1_4_complete = len([f for f in phase_1_4_features if f['status'] == 'COMPLETE'])
    phase_1_4_total = len(phase_1_4_features)
    
    # Blocked features count
    blocked_count = len([f for f in features.values() if f.get('blockers')])
    
    summary = {
        'total_features': total,
        'completed': completed,
        'in_progress': in_progress,
        'planned': planned,
        'completion_percentage': round((completed / total) * 100, 1),
        'current_phase': '1.4',
        'current_phase_progress': f"{phase_1_4_complete}/{phase_1_4_total}",
        'blocked_features': blocked_count
    }
    
    return summary

if __name__ == '__main__':
    summary = get_feature_summary()
    if isinstance(summary, dict):
        print(f"Features: {summary['completed']}/{summary['total_features']} complete ({summary['completion_percentage']}%)")
        print(f"Phase {summary['current_phase']}: {summary['current_phase_progress']} complete")
        if summary['blocked_features'] > 0:
            print(f"⚠️ {summary['blocked_features']} features blocked")
    else:
        print(summary)
