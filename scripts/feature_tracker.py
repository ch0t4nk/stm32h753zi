#!/usr/bin/env python3
"""
Feature Tracker - Centralized feature management system for STM32H753ZI project

Usage:
    python3 scripts/feature_tracker.py list [--status STATUS] [--phase PHASE] [--category CATEGORY]
    python3 scripts/feature_tracker.py show FTR-001
    python3 scripts/feature_tracker.py update FTR-001 --status COMPLETE
    python3 scripts/feature_tracker.py report
    python3 scripts/feature_tracker.py dependencies FTR-001
    python3 scripts/feature_tracker.py validate
"""

import json
import argparse
import sys
import os
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional

class FeatureTracker:
    def __init__(self, registry_path: str = "features/feature_registry.json"):
        self.registry_path = Path(registry_path)
        self.registry = self.load_registry()
    
    def load_registry(self) -> Dict:
        """Load feature registry from JSON file"""
        if not self.registry_path.exists():
            raise FileNotFoundError(f"Feature registry not found: {self.registry_path}")
        
        with open(self.registry_path, 'r') as f:
            return json.load(f)
    
    def save_registry(self):
        """Save feature registry to JSON file"""
        # Update metadata
        self.registry['metadata']['last_updated'] = datetime.now().isoformat() + 'Z'
        self._update_metadata()
        
        with open(self.registry_path, 'w') as f:
            json.dump(self.registry, f, indent=2)
    
    def _update_metadata(self):
        """Update metadata counts"""
        features = self.registry['features']
        status_counts = {}
        
        for feature in features.values():
            status = feature['status']
            status_counts[status] = status_counts.get(status, 0) + 1
        
        self.registry['metadata'].update({
            'total_features': len(features),
            'completed_features': status_counts.get('COMPLETE', 0),
            'in_progress_features': status_counts.get('IN_PROGRESS', 0),
            'planned_features': status_counts.get('PLANNED', 0)
        })
    
    def list_features(self, status: Optional[str] = None, phase: Optional[str] = None, 
                     category: Optional[str] = None) -> List[Dict]:
        """List features with optional filtering"""
        features = []
        
        for feature_id, feature in self.registry['features'].items():
            # Apply filters
            if status and feature['status'] != status:
                continue
            if phase and feature['phase'] != phase:
                continue
            if category and feature['category'] != category:
                continue
            
            features.append(feature)
        
        return sorted(features, key=lambda x: x['id'])
    
    def get_feature(self, feature_id: str) -> Optional[Dict]:
        """Get specific feature by ID"""
        return self.registry['features'].get(feature_id)
    
    def update_feature(self, feature_id: str, **updates):
        """Update feature properties"""
        if feature_id not in self.registry['features']:
            raise ValueError(f"Feature {feature_id} not found")
        
        feature = self.registry['features'][feature_id]
        
        for key, value in updates.items():
            if key in feature:
                feature[key] = value
            else:
                print(f"Warning: Unknown field '{key}' for feature {feature_id}")
        
        self.save_registry()
    
    def get_dependencies(self, feature_id: str) -> Dict[str, List[str]]:
        """Get dependency tree for a feature"""
        feature = self.get_feature(feature_id)
        if not feature:
            return {}
        
        return {
            'dependencies': feature.get('dependencies', []),
            'dependents': feature.get('dependents', []),
            'blockers': feature.get('blockers', [])
        }
    
    def validate_registry(self) -> List[str]:
        """Validate registry for consistency"""
        errors = []
        features = self.registry['features']
        
        # Check for valid dependency references
        for feature_id, feature in features.items():
            for dep_id in feature.get('dependencies', []):
                if dep_id not in features:
                    errors.append(f"{feature_id}: Invalid dependency {dep_id}")
            
            for dep_id in feature.get('dependents', []):
                if dep_id not in features:
                    errors.append(f"{feature_id}: Invalid dependent {dep_id}")
        
        # Check for circular dependencies (simple check)
        for feature_id, feature in features.items():
            deps = feature.get('dependencies', [])
            if feature_id in deps:
                errors.append(f"{feature_id}: Self-dependency detected")
        
        return errors
    
    def generate_report(self) -> str:
        """Generate comprehensive feature status report"""
        metadata = self.registry['metadata']
        features = self.registry['features']
        
        report = []
        report.append("# Feature Status Report")
        report.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        report.append("")
        
        # Summary
        report.append("## Summary")
        report.append(f"- **Total Features**: {metadata['total_features']}")
        report.append(f"- **Completed**: {metadata['completed_features']}")
        report.append(f"- **In Progress**: {metadata['in_progress_features']}")
        report.append(f"- **Planned**: {metadata['planned_features']}")
        report.append("")
        
        # By Phase
        phase_summary = {}
        for feature in features.values():
            phase = feature['phase']
            status = feature['status']
            if phase not in phase_summary:
                phase_summary[phase] = {}
            phase_summary[phase][status] = phase_summary[phase].get(status, 0) + 1
        
        report.append("## Progress by Phase")
        for phase in sorted(phase_summary.keys()):
            phase_name = self.registry['phases'].get(phase, phase)
            summary = phase_summary[phase]
            total = sum(summary.values())
            completed = summary.get('COMPLETE', 0)
            report.append(f"- **{phase} ({phase_name})**: {completed}/{total} complete")
        report.append("")
        
        # By Category
        category_summary = {}
        for feature in features.values():
            category = feature['category']
            status = feature['status']
            if category not in category_summary:
                category_summary[category] = {}
            category_summary[category][status] = category_summary[category].get(status, 0) + 1
        
        report.append("## Progress by Category")
        for category in sorted(category_summary.keys()):
            category_desc = self.registry['categories'].get(category, category)
            summary = category_summary[category]
            total = sum(summary.values())
            completed = summary.get('COMPLETE', 0)
            report.append(f"- **{category}** ({category_desc}): {completed}/{total} complete")
        report.append("")
        
        # Current Work
        in_progress = [f for f in features.values() if f['status'] == 'IN_PROGRESS']
        if in_progress:
            report.append("## Currently In Progress")
            for feature in in_progress:
                report.append(f"- **{feature['id']}**: {feature['name']} (Phase {feature['phase']})")
            report.append("")
        
        # Blockers
        blocked_features = [f for f in features.values() if f.get('blockers')]
        if blocked_features:
            report.append("## Blocked Features")
            for feature in blocked_features:
                blockers = ", ".join(feature['blockers'])
                report.append(f"- **{feature['id']}**: {feature['name']} - Blocked by: {blockers}")
            report.append("")
        
        return "\\n".join(report)

def main():
    parser = argparse.ArgumentParser(description="Feature Tracker for STM32H753ZI project")
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # List command
    list_parser = subparsers.add_parser('list', help='List features')
    list_parser.add_argument('--status', choices=['COMPLETE', 'IN_PROGRESS', 'PLANNED'], help='Filter by status')
    list_parser.add_argument('--phase', help='Filter by phase')
    list_parser.add_argument('--category', help='Filter by category')
    
    # Show command
    show_parser = subparsers.add_parser('show', help='Show feature details')
    show_parser.add_argument('feature_id', help='Feature ID to show')
    
    # Update command
    update_parser = subparsers.add_parser('update', help='Update feature')
    update_parser.add_argument('feature_id', help='Feature ID to update')
    update_parser.add_argument('--status', choices=['COMPLETE', 'IN_PROGRESS', 'PLANNED'], help='Update status')
    update_parser.add_argument('--effort-actual', type=int, help='Update actual effort hours')
    
    # Report command
    subparsers.add_parser('report', help='Generate status report')
    
    # Dependencies command
    deps_parser = subparsers.add_parser('dependencies', help='Show feature dependencies')
    deps_parser.add_argument('feature_id', help='Feature ID to analyze')
    
    # Validate command
    subparsers.add_parser('validate', help='Validate registry consistency')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    try:
        tracker = FeatureTracker()
        
        if args.command == 'list':
            features = tracker.list_features(args.status, args.phase, args.category)
            print(f"{'ID':<8} {'Status':<12} {'Phase':<6} {'Category':<12} {'Name'}")
            print("-" * 80)
            for feature in features:
                print(f"{feature['id']:<8} {feature['status']:<12} {feature['phase']:<6} {feature['category']:<12} {feature['name']}")
        
        elif args.command == 'show':
            feature = tracker.get_feature(args.feature_id)
            if not feature:
                print(f"Feature {args.feature_id} not found")
                return
            
            print(f"# {feature['id']}: {feature['name']}")
            print(f"**Status**: {feature['status']}")
            print(f"**Phase**: {feature['phase']}")
            print(f"**Priority**: {feature['priority']}")
            print(f"**Category**: {feature['category']}")
            print(f"**Description**: {feature['description']}")
            print()
            
            if feature.get('acceptance_criteria'):
                print("**Acceptance Criteria**:")
                for criteria in feature['acceptance_criteria']:
                    print(f"- {criteria}")
                print()
            
            if feature.get('dependencies'):
                print(f"**Dependencies**: {', '.join(feature['dependencies'])}")
            if feature.get('dependents'):
                print(f"**Dependents**: {', '.join(feature['dependents'])}")
            if feature.get('blockers'):
                print(f"**Blockers**: {', '.join(feature['blockers'])}")
            
            if feature.get('effort_estimate_hours'):
                estimate = feature['effort_estimate_hours']
                actual = feature.get('effort_actual_hours', 0)
                print(f"**Effort**: {actual}/{estimate} hours")
        
        elif args.command == 'update':
            updates = {}
            if args.status:
                updates['status'] = args.status
                feature = tracker.get_feature(args.feature_id)
                if args.status == 'COMPLETE' and feature and not feature.get('completion_date'):
                    updates['completion_date'] = datetime.now().strftime('%Y-%m-%d')
            if args.effort_actual:
                updates['effort_actual_hours'] = args.effort_actual
            
            tracker.update_feature(args.feature_id, **updates)
            print(f"Updated {args.feature_id}: {updates}")
        
        elif args.command == 'report':
            report = tracker.generate_report()
            print(report)
        
        elif args.command == 'dependencies':
            deps = tracker.get_dependencies(args.feature_id)
            feature = tracker.get_feature(args.feature_id)
            if not feature:
                print(f"Feature {args.feature_id} not found")
                return
            
            print(f"# Dependencies for {args.feature_id}: {feature['name']}")
            print()
            
            if deps['dependencies']:
                print("**Depends on**:")
                for dep_id in deps['dependencies']:
                    dep_feature = tracker.get_feature(dep_id)
                    name = dep_feature['name'] if dep_feature else 'Unknown'
                    status = dep_feature['status'] if dep_feature else 'Unknown'
                    print(f"- {dep_id}: {name} ({status})")
                print()
            
            if deps['dependents']:
                print("**Required by**:")
                for dep_id in deps['dependents']:
                    dep_feature = tracker.get_feature(dep_id)
                    name = dep_feature['name'] if dep_feature else 'Unknown'
                    status = dep_feature['status'] if dep_feature else 'Unknown'
                    print(f"- {dep_id}: {name} ({status})")
                print()
            
            if deps['blockers']:
                print("**Blocked by**:")
                for blocker in deps['blockers']:
                    print(f"- {blocker}")
        
        elif args.command == 'validate':
            errors = tracker.validate_registry()
            if errors:
                print("Validation errors found:")
                for error in errors:
                    print(f"- {error}")
                sys.exit(1)
            else:
                print("✅ Registry validation passed")
    
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
