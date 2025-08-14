#!/usr/bin/env python3
"""
STATUS.md Compliance Checker

Validates STATUS.md against status-maintenance.instructions.md requirements.
Ensures proper formatting, required sections, and Copilot context completeness.

Usage:
    python3 scripts/validate_status_compliance.py [--verbose] [--fix]

Author: STM32H753ZI Development Team
Date: August 7, 2025
"""

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple


class StatusComplianceChecker:
    """Validates STATUS.md compliance with instruction requirements"""
    
    def __init__(self, workspace_root: str = "/workspaces/code", verbose: bool = False):
        self.workspace = Path(workspace_root).resolve()
        self.status_file = self.workspace / "STATUS.md"
        self.verbose = verbose
        self.issues = []
        
        # Required sections from instructions
        self.required_sections = [
            "# STM32H753ZI Project Status",
            "## 🔧 Setup Summary", 
            "## ✅ Progress So Far",
            "## 🔄 Current Work",
            "## 🔄 Current Technical Context for Copilot Continuation",
            "## ⏭️ Next Steps",
            "## 🧠 Notes & Observations", 
            "## 📊 Key Metrics",
            "## 🎯 Architecture Status",
            "## 🔗 Quick References",
            "## 🤖 Enhanced Status Auto-Update Configuration"
        ]
        
        # Required technical context subsections
        self.required_tech_context = [
            "**Phase Status**:",
            "**Active Development State**:",
            "**Technical Architecture Decisions**:",
            "**Key Implementation Files**:",
            "**Performance Validation Results**:",
            "**Build System Status**:",
            "**Next Development Priorities**:",
            "**Conversation Continuity Markers**:"
        ]

    def log(self, message: str) -> None:
        """Log message if verbose mode enabled"""
        if self.verbose:
            print(f"[CHECK] {message}")

    def check_file_exists(self) -> bool:
        """Check if STATUS.md exists"""
        if not self.status_file.exists():
            self.issues.append("CRITICAL: STATUS.md file not found")
            return False
        return True

    def check_required_sections(self, content: str) -> bool:
        """Check if all required sections are present"""
        self.log("Checking required sections...")
        missing_sections = []
        
        for section in self.required_sections:
            if section not in content:
                missing_sections.append(section)
                
        if missing_sections:
            self.issues.append(f"Missing required sections: {missing_sections}")
            return False
            
        self.log(f"✅ All {len(self.required_sections)} required sections found")
        return True

    def check_header_format(self, content: str) -> bool:
        """Check if header format is correct"""
        self.log("Checking header format...")
        header_issues = []
        
        # Check for required header fields
        required_headers = [
            r"\*\*Last Updated\*\*:",
            r"\*\*Status\*\*:",
            r"\*\*Deployment\*\*:",
            r"\*\*AI Infrastructure\*\*:"
        ]
        
        for header_pattern in required_headers:
            if not re.search(header_pattern, content):
                header_issues.append(f"Missing header: {header_pattern}")
                
        if header_issues:
            self.issues.append(f"Header format issues: {header_issues}")
            return False
            
        self.log("✅ Header format correct")
        return True

    def check_technical_context(self, content: str) -> bool:
        """Check technical context section completeness"""
        self.log("Checking technical context section...")
        
        # Find the technical context section
        tech_context_match = re.search(
            r"## 🔄 Current Technical Context for Copilot Continuation(.*?)(?=## |$)",
            content, re.DOTALL
        )
        
        if not tech_context_match:
            self.issues.append("Technical context section not found")
            return False
            
        tech_context = tech_context_match.group(1)
        missing_subsections = []
        
        for subsection in self.required_tech_context:
            if subsection not in tech_context:
                missing_subsections.append(subsection)
                
        if missing_subsections:
            self.issues.append(f"Missing technical context subsections: {missing_subsections}")
            return False
            
        # Check for specific phase status format
        phase_patterns = [
            r"\*\*Phase 1\.1\*\*:",
            r"\*\*Phase 1\.2\*\*:",
            r"\*\*Phase 1\.3\*\*:",
            r"\*\*Phase 1\.4\*\*:"
        ]
        
        missing_phases = []
        for phase_pattern in phase_patterns:
            if not re.search(phase_pattern, tech_context):
                missing_phases.append(phase_pattern)
                
        if missing_phases:
            self.issues.append(f"Missing phase status entries: {missing_phases}")
            return False
            
        self.log("✅ Technical context section complete")
        return True

    def check_metrics_format(self, content: str) -> bool:
        """Check metrics section formatting"""
        self.log("Checking metrics format...")
        
        # Find metrics section
        metrics_match = re.search(
            r"## 📊 Key Metrics(.*?)(?=## |$)",
            content, re.DOTALL
        )
        
        if not metrics_match:
            self.issues.append("Key Metrics section not found")
            return False
            
        metrics = metrics_match.group(1)
        
        # Check for required metrics
        required_metrics = [
            r"\*\*Firmware Size\*\*:",
            r"\*\*RAM Usage\*\*:",
            r"\*\*FreeRTOS Performance\*\*:",
            r"\*\*Control Performance\*\*:",
            r"\*\*Safety Response\*\*:"
        ]
        
        missing_metrics = []
        for metric_pattern in required_metrics:
            if not re.search(metric_pattern, metrics):
                missing_metrics.append(metric_pattern)
                
        if missing_metrics:
            self.issues.append(f"Missing required metrics: {missing_metrics}")
            return False
            
        self.log("✅ Metrics section complete")
        return True

    def check_status_indicators(self, content: str) -> bool:
        """Check for proper status emoji usage"""
        self.log("Checking status indicators...")
        
        # Count status emoji usage
        status_emojis = {
            "✅": len(re.findall(r"✅", content)),
            "🚀": len(re.findall(r"🚀", content)),
            "🔄": len(re.findall(r"🔄", content)),
            "⏸️": len(re.findall(r"⏸️", content)),
            "❌": len(re.findall(r"❌", content)),
            "🧪": len(re.findall(r"🧪", content))
        }
        
        self.log(f"Status emoji usage: {status_emojis}")
        
        # Should have at least some status indicators
        if status_emojis["✅"] == 0:
            self.issues.append("No completion indicators (✅) found")
            return False
            
        return True

    def check_build_commands(self, content: str) -> bool:
        """Check for essential build commands"""
        self.log("Checking build commands...")
        
        essential_commands = [
            "./scripts/fix_cmake.sh",
            "python3 scripts/auto_update_status.py"
        ]
        
        missing_commands = []
        for command in essential_commands:
            if command not in content:
                missing_commands.append(command)
                
        if missing_commands:
            self.issues.append(f"Missing essential commands: {missing_commands}")
            return False
            
        self.log("✅ Build commands present")
        return True

    def run_compliance_check(self) -> Dict:
        """Run complete compliance check"""
        self.log("Starting STATUS.md compliance check...")
        
        if not self.check_file_exists():
            return {"compliant": False, "issues": self.issues}
            
        try:
            content = self.status_file.read_text(encoding="utf-8")
        except Exception as e:
            self.issues.append(f"Error reading STATUS.md: {e}")
            return {"compliant": False, "issues": self.issues}
            
        # Run all checks
        checks = [
            self.check_required_sections(content),
            self.check_header_format(content),
            self.check_technical_context(content),
            self.check_metrics_format(content),
            self.check_status_indicators(content),
            self.check_build_commands(content)
        ]
        
        compliant = all(checks)
        
        result = {
            "compliant": compliant,
            "issues": self.issues,
            "checks_passed": sum(checks),
            "total_checks": len(checks),
            "file_size": len(content),
            "line_count": len(content.splitlines())
        }
        
        if compliant:
            self.log("✅ STATUS.md is fully compliant with instructions")
        else:
            self.log(f"❌ STATUS.md has {len(self.issues)} compliance issues")
            
        return result


def main():
    parser = argparse.ArgumentParser(description="Validate STATUS.md compliance")
    parser.add_argument("--verbose", "-v", action="store_true", 
                       help="Enable verbose output")
    parser.add_argument("--workspace", default="/workspaces/code",
                       help="Workspace root directory")
    
    args = parser.parse_args()
    
    try:
        checker = StatusComplianceChecker(
            workspace_root=args.workspace,
            verbose=args.verbose
        )
        
        result = checker.run_compliance_check()
        
        # Print results
        print(f"\n📊 STATUS.md Compliance Report")
        print(f"=" * 40)
        print(f"File Size: {result['file_size']} bytes")
        print(f"Line Count: {result['line_count']} lines")
        print(f"Checks Passed: {result['checks_passed']}/{result['total_checks']}")
        
        if result["compliant"]:
            print("✅ STATUS.md is COMPLIANT with instructions")
            return 0
        else:
            print("❌ STATUS.md has COMPLIANCE ISSUES:")
            for i, issue in enumerate(result["issues"], 1):
                print(f"  {i}. {issue}")
            return 1
            
    except Exception as e:
        print(f"❌ Error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
