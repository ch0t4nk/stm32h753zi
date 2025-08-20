#!/usr/bin/env python3
"""
Schema-based C Header Generator and Validator
Generates SSOT-compliant C headers from YAML register schemas
Part of STM32H753ZI safe development framework
"""

import sys
import yaml
import argparse
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any, Optional
import re


class SchemaValidator:
    """Validates register schemas for completeness and safety"""
    
    def __init__(self):
        self.errors = []
        self.warnings = []
        
    def validate_schema(self, schema: Dict[str, Any]) -> bool:
        """Validate complete schema structure"""
        self.errors.clear()
        self.warnings.clear()
        
        # Check required top-level sections
        required_sections = ['metadata', 'registers']
        for section in required_sections:
            if section not in schema:
                self.errors.append(f"Missing required section: {section}")
                
        if 'metadata' in schema:
            self._validate_metadata(schema['metadata'])
            
        if 'registers' in schema:
            self._validate_registers(schema['registers'])
            
        if 'validation' in schema:
            self._validate_safety_rules(schema['validation'])
            
        return len(self.errors) == 0
        
    def _validate_metadata(self, metadata: Dict[str, Any]):
        """Validate metadata section"""
        required_fields = ['chip', 'version', 'auto_generated_header']
        for field in required_fields:
            if field not in metadata:
                self.errors.append(f"Missing metadata field: {field}")
                
    def _validate_registers(self, registers: Dict[str, Any]):
        """Validate register definitions"""
        used_addresses = set()
        
        for reg_name, reg_def in registers.items():
            # Check required fields
            required_fields = ['address', 'size_bytes', 'mask', 'type', 'description', 'access']
            for field in required_fields:
                if field not in reg_def:
                    self.errors.append(f"Register {reg_name}: missing field {field}")
                    
            # Check for address conflicts
            if 'address' in reg_def:
                addr = reg_def['address']
                if addr in used_addresses:
                    self.errors.append(f"Register {reg_name}: address conflict at 0x{addr:02X}")
                used_addresses.add(addr)
                
            # Validate mask against size
            if 'mask' in reg_def and 'size_bytes' in reg_def:
                size_bytes = reg_def['size_bytes']
                mask = reg_def['mask']
                max_mask = (1 << (size_bytes * 8)) - 1
                if mask > max_mask:
                    self.errors.append(f"Register {reg_name}: mask 0x{mask:X} exceeds {size_bytes} bytes")
                    
            # Check safety-critical registers have validation
            if reg_def.get('type') in ['configuration', 'protection']:
                if 'valid_range' not in reg_def and 'fields' not in reg_def:
                    self.warnings.append(f"Register {reg_name}: configuration register without validation")
                    
    def _validate_safety_rules(self, validation: Dict[str, Any]):
        """Validate safety validation rules"""
        if 'critical_registers' not in validation:
            self.warnings.append("No critical registers defined in validation")
            
        if 'safe_defaults' not in validation:
            self.warnings.append("No safe defaults defined in validation")
            
    def get_report(self) -> str:
        """Get validation report"""
        report = []
        if self.errors:
            report.append("ERRORS:")
            for error in self.errors:
                report.append(f"  ❌ {error}")
                
        if self.warnings:
            report.append("WARNINGS:")
            for warning in self.warnings:
                report.append(f"  ⚠️  {warning}")
                
        if not self.errors and not self.warnings:
            report.append("✅ Schema validation passed!")
            
        return "\n".join(report)


class CHeaderGenerator:
    """Generates SSOT-compliant C headers from validated schemas"""
    
    def __init__(self):
        self.workspace_root = Path(__file__).parent.parent

    def _sanitize_identifier(self, name: str) -> str:
        """Return a safe C identifier: replace non-alphanum/_ with '_' and ensure it doesn't start with a digit."""
        # Replace any character that isn't A-Za-z0-9_ with underscore
        s = re.sub(r'[^A-Za-z0-9_]', '_', name)
        # Collapse multiple underscores
        s = re.sub(r'__+', '_', s)
        # Ensure it doesn't start with a digit
        if re.match(r'^[0-9]', s):
            s = '_' + s
        return s
        
    def generate_header(self, schema: Dict[str, Any], output_path: Optional[str] = None) -> str:
        """Generate C header from schema"""
        if output_path is None:
            output_path = schema['metadata']['auto_generated_header']
            
        # Make path relative to workspace root
        full_output_path = self.workspace_root / output_path
        
        header_content = self._generate_header_content(schema)
        
        # Ensure output directory exists
        full_output_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Write header file
        with open(full_output_path, 'w', encoding='utf-8') as f:
            f.write(header_content)
            
        return str(full_output_path)
        
    def _generate_header_content(self, schema: Dict[str, Any]) -> str:
        """Generate the actual C header content"""
        metadata = schema['metadata']
        chip = metadata['chip']
        # sanitized chip prefix for macro generation (e.g., L6470, AS5600)
        chip_prefix = self._sanitize_identifier(str(chip)).upper()

        # accumulator for optional legacy aliases (unprefixed macros)
        alias_lines: List[str] = []

        lines = []

        # Header preamble
        lines.extend(self._generate_preamble(metadata))

        # Include guard (sanitized)
        guard_name = self._sanitize_identifier(f"{chip}_REGISTERS_GENERATED_H").upper()
        lines.append(f"#ifndef {guard_name}")
        lines.append(f"#define {guard_name}")
        lines.append("")

        # Includes
        lines.extend(self._generate_includes(schema))

        # Register address definitions
        lines.extend(self._generate_register_addresses(schema['registers'], chip_prefix, alias_lines))

        # Register mask definitions
        lines.extend(self._generate_register_masks(schema['registers'], chip_prefix, alias_lines))

        # Field definitions (for bitfield registers)
        lines.extend(self._generate_field_definitions(schema['registers'], chip_prefix, alias_lines))

        # Combined register macros (for multi-byte registers)
        if 'combined_registers' in schema:
            lines.extend(self._generate_combined_register_macros(schema['combined_registers'], chip_prefix, alias_lines))

        # Command definitions
        if 'commands' in schema:
            lines.extend(self._generate_command_definitions(schema['commands'], chip_prefix, alias_lines))

        # Validation constants
        if 'validation' in schema:
            lines.extend(self._generate_validation_constants(schema['validation'], chip_prefix, alias_lines))

        # Function declarations for register access
        lines.extend(self._generate_function_declarations(schema, chip))

        # Emit optional legacy aliases under a guarded macro for safe migration
        # Legacy aliases are now opt-in. To enable legacy (unprefixed) names,
        # define ENABLE_LEGACY_REG_NAMES before including this header.
        if alias_lines:
            lines.append("#ifdef ENABLE_LEGACY_REG_NAMES")
            lines.append("/* Legacy register names enabled */")
            lines.extend(alias_lines)
            lines.append("#endif /* ENABLE_LEGACY_REG_NAMES */")

        # Close include guard
        lines.append("")
        lines.append(f"#endif /* {guard_name} */")
        lines.append("")

        return "\n".join(lines)
        
    def _generate_preamble(self, metadata: Dict[str, Any]) -> List[str]:
        """Generate file header and documentation"""
        chip = metadata['chip']
        version = metadata['version']
        
        lines = [
            "/**",
            f" * @file {metadata['auto_generated_header'].split('/')[-1]}",
            f" * @brief {chip} Register Definitions - Auto-Generated from Schema",
            f" * @version {version}",
            f" * @date {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
            " *",
            " * ⚠️  WARNING: AUTO-GENERATED FILE - DO NOT EDIT MANUALLY",
            " * This file is generated from YAML schema definitions.",
            " * Modify the schema file and regenerate instead.",
            " *",
            f" * Schema source: schemas/{chip.lower()}_registers.yaml",
            " * Generator: scripts/schema_generator.py",
            " *",
            " * @note Generated for STM32H753ZI stepper motor control project",
            " * @note Follows SSOT configuration management principles",
            " */"
        ]
        
        if 'datasheet' in metadata:
            lines.insert(-2, f" * @note Based on {chip} datasheet {metadata['datasheet']}")
            
        return lines
        
    def _generate_includes(self, schema: Dict[str, Any]) -> List[str]:
        """Generate necessary includes"""
        lines = [
            "",
            "/* Required includes for SSOT integration */",
            "#include <stdint.h>",
            "#include <stdbool.h>",
            "#include \"common/error_codes.h\"",
            ""
        ]
        
        # Add chip-specific includes
        chip = schema['metadata']['chip']
        if chip == 'L6470':
            lines.insert(-1, "#include \"hal_abstraction/hal_abstraction.h\"")
        elif chip == 'AS5600':
            lines.insert(-1, "#include \"hal_abstraction/hal_abstraction.h\"")
            
        return lines
        
    def _generate_register_addresses(self, registers: Dict[str, Any], chip_prefix: str, alias_lines: List[str]) -> List[str]:
        """Generate register address definitions (prefixed) and optional legacy aliases"""

        lines = [
            "/* ========================================================================== */",
            f"/* Register Address Definitions                                              */",
            "/* ========================================================================== */"
        ]

        for reg_name, reg_def in registers.items():
            addr = reg_def['address']
            desc = reg_def.get('description', '')
            sanitized_reg = self._sanitize_identifier(reg_name).upper()
            # Prefixed macro
            pref_macro = f"{chip_prefix}_{sanitized_reg}_ADDR"
            lines.append(f"#define {pref_macro}    0x{addr:02X}    /* {desc} */")
            # Legacy alias: unprefixed macro -> prefixed macro
            unpref = self._sanitize_identifier(f"{reg_name}_ADDR").upper()
            alias_lines.append(f"#define {unpref} {pref_macro}")
            # CHIP_ alias for board-level generic usage
            chip_alias = f"CHIP_{sanitized_reg}_ADDR"
            alias_lines.append(f"#define {chip_alias} {pref_macro}")

        lines.append("")
        return lines
        
    def _generate_register_masks(self, registers: Dict[str, Any], chip_prefix: str, alias_lines: List[str]) -> List[str]:
        """Generate register mask definitions (prefixed) and legacy aliases"""
        lines = [
            "/* ========================================================================== */",
            f"/* Register Mask Definitions                                                 */",
            "/* ========================================================================== */"
        ]

        for reg_name, reg_def in registers.items():
            if 'mask' in reg_def:
                mask = reg_def['mask']
                size_bytes = reg_def['size_bytes']
                sanitized_reg = self._sanitize_identifier(reg_name).upper()

                pref_macro = f"{chip_prefix}_{sanitized_reg}_MASK"
                if size_bytes == 1:
                    lines.append(f"#define {pref_macro}        0x{mask:02X}")
                elif size_bytes == 2:
                    lines.append(f"#define {pref_macro}        0x{mask:04X}")
                elif size_bytes == 3:
                    lines.append(f"#define {pref_macro}        0x{mask:06X}")
                else:
                    lines.append(f"#define {pref_macro}        0x{mask:08X}")

                # Legacy alias
                unpref = self._sanitize_identifier(f"{reg_name}_MASK").upper()
                alias_lines.append(f"#define {unpref} {pref_macro}")
                # CHIP_ alias
                chip_alias_mask = f"CHIP_{sanitized_reg}_MASK"
                alias_lines.append(f"#define {chip_alias_mask} {pref_macro}")

        lines.append("")
        return lines
        
    def _generate_field_definitions(self, registers: Dict[str, Any], chip_prefix: str, alias_lines: List[str]) -> List[str]:
        """Generate bitfield definitions for registers with fields"""
        lines = [
            "/* ========================================================================== */",
            f"/* Register Field Definitions                                                */",
            "/* ========================================================================== */"
        ]
        
        for reg_name, reg_def in registers.items():
            if 'fields' in reg_def:
                lines.append(f"/* {reg_name} register fields */")
                
                for field_name, field_def in reg_def['fields'].items():
                    if 'bits' in field_def:
                        bits = field_def['bits']
                        if len(bits) == 2:  # Range [start, end]
                            start_bit, end_bit = bits
                            width = end_bit - start_bit + 1
                            mask = ((1 << width) - 1) << start_bit
                            sanitized_reg = self._sanitize_identifier(reg_name).upper()
                            sanitized_field = self._sanitize_identifier(field_name).upper()
                            pos_macro_pref = f"{chip_prefix}_{sanitized_reg}_{sanitized_field}_POS"
                            msk_macro_pref = f"{chip_prefix}_{sanitized_reg}_{sanitized_field}_MSK"
                            lines.append(f"#define {pos_macro_pref}    {start_bit}")
                            lines.append(f"#define {msk_macro_pref}    0x{mask:04X}")

                            # Legacy aliases
                            pos_unpref = self._sanitize_identifier(f"{reg_name}_{field_name}_POS").upper()
                            msk_unpref = self._sanitize_identifier(f"{reg_name}_{field_name}_MSK").upper()
                            alias_lines.append(f"#define {pos_unpref} {pos_macro_pref}")
                            alias_lines.append(f"#define {msk_unpref} {msk_macro_pref}")
                            
                            # Generate value definitions if present
                            if 'values' in field_def:
                                for value, description in field_def['values'].items():
                                    shifted_value = value << start_bit
                                    desc_sanitized = self._sanitize_identifier(str(description)).upper()
                                    val_macro_pref = f"{chip_prefix}_{sanitized_reg}_{sanitized_field}_{desc_sanitized}"
                                    lines.append(f"#define {val_macro_pref}    0x{shifted_value:04X}")
                                    # Legacy alias for value
                                    val_unpref = self._sanitize_identifier(f"{reg_name}_{field_name}_{description}").upper()
                                    alias_lines.append(f"#define {val_unpref} {val_macro_pref}")
                                    
                lines.append("")
                
        return lines
        
    def _generate_combined_register_macros(self, combined_regs: Dict[str, Any], chip_prefix: str, alias_lines: List[str]) -> List[str]:
        """Generate macros for combined multi-byte registers"""
        lines = [
            "/* ========================================================================== */",
            f"/* Combined Register Macros (Multi-byte registers)                          */",
            "/* ========================================================================== */"
        ]
        
        for reg_name, reg_def in combined_regs.items():
            high_reg = reg_def['high_reg']
            low_reg = reg_def['low_reg']
            total_bits = reg_def['total_bits']
            reg_base = self._sanitize_identifier(reg_name).upper()
            reg_pref = f"{chip_prefix}_{reg_base}"
            lines.append(f"/* {reg_name}: {reg_def.get('description', '')} */")
            lines.append(f"#define {reg_pref}_COMBINE(high, low)    (((high) << 8) | (low))")
            lines.append(f"#define {reg_pref}_HIGH(val)            (((val) >> 8) & 0x{(1 << (total_bits - 8)) - 1:02X})")
            lines.append(f"#define {reg_pref}_LOW(val)             ((val) & 0xFF)")
            # Legacy aliases
            alias_lines.append(f"#define {reg_base}_COMBINE(high, low) {reg_pref}_COMBINE(high, low)")
            alias_lines.append(f"#define {reg_base}_HIGH(val) {reg_pref}_HIGH(val)")
            alias_lines.append(f"#define {reg_base}_LOW(val) {reg_pref}_LOW(val)")
            lines.append("")
            
        return lines
        
    def _generate_command_definitions(self, commands: Dict[str, Any], chip_prefix: str, alias_lines: List[str]) -> List[str]:
        """Generate command definitions"""
        lines = [
            "/* ========================================================================== */",
            f"/* Command Definitions                                                       */",
            "/* ========================================================================== */"
        ]
        
        for cmd_name, cmd_def in commands.items():
            opcode = cmd_def['opcode']
            desc = cmd_def.get('description', '')
            cmd_sanitized = self._sanitize_identifier(cmd_name).upper()
            cmd_pref = f"{chip_prefix}_CMD_{cmd_sanitized}"
            lines.append(f"#define {cmd_pref}    0x{opcode:02X}    /* {desc} */")
            # Legacy alias
            unpref = self._sanitize_identifier(f"CMD_{cmd_name}").upper()
            alias_lines.append(f"#define {unpref} {cmd_pref}")
            
        lines.append("")
        return lines
        
    def _generate_validation_constants(self, validation: Dict[str, Any], chip_prefix: str, alias_lines: List[str]) -> List[str]:
        """Generate validation constants"""
        lines = [
            "/* ========================================================================== */",
            f"/* Validation Constants                                                      */",
            "/* ========================================================================== */"
        ]
        
        if 'safe_defaults' in validation:
            lines.append("/* Safe default values */")
            for reg, value in validation['safe_defaults'].items():
                sanitized_reg = self._sanitize_identifier(reg).upper()
                safe_pref = f"{chip_prefix}_{sanitized_reg}_SAFE_DEFAULT"
                if isinstance(value, int):
                    lines.append(f"#define {safe_pref}    0x{value:04X}")
                else:
                    lines.append(f"#define {safe_pref}    {value}")
                # Legacy alias
                unpref = self._sanitize_identifier(f"{reg}_SAFE_DEFAULT").upper()
                alias_lines.append(f"#define {unpref} {safe_pref}")

            lines.append("")
            
        return lines
        
    def _generate_function_declarations(self, schema: Dict[str, Any], chip: str) -> List[str]:
        """Generate function declarations for register access"""
        chip = chip.lower()
        
        lines = [
            "/* ========================================================================== */",
            f"/* Register Access Function Declarations                                    */",
            "/* ========================================================================== */",
            "",
            "#ifdef __cplusplus",
            "extern \"C\" {",
            "#endif",
            "",
            "/**",
            f" * @brief Validate {chip.upper()} register value",
            " * @param reg_addr Register address",
            " * @param value Value to validate",
            " * @return true if valid, false otherwise",
            " */",
            f"bool {chip}_validate_register_value(uint8_t reg_addr, uint32_t value);",
            "",
            "/**",
            f" * @brief Get safe default value for {chip.upper()} register",
            " * @param reg_addr Register address",
            " * @return Safe default value",
            " */",
            f"uint32_t {chip}_get_safe_default(uint8_t reg_addr);",
            "",
            "/**",
            f" * @brief Check if {chip.upper()} register is critical for safety",
            " * @param reg_addr Register address",
            " * @return true if critical, false otherwise",
            " */",
            f"bool {chip}_is_critical_register(uint8_t reg_addr);",
            "",
            "#ifdef __cplusplus",
            "}",
            "#endif",
            ""
        ]
        
        return lines

def main():
    """Main schema generator function"""
    parser = argparse.ArgumentParser(description="Generate C headers from register schemas")
    parser.add_argument("schema_file", help="Path to YAML schema file")
    parser.add_argument("--output", "-o", help="Output header file path (overrides schema)")
    parser.add_argument("--validate-only", "-v", action="store_true", help="Only validate, don't generate")
    parser.add_argument("--verbose", action="store_true", help="Verbose output")
    
    args = parser.parse_args()
    
    # Load schema
    try:
        with open(args.schema_file, 'r') as f:
            schema = yaml.safe_load(f)
    except Exception as e:
        print(f"❌ Error loading schema: {e}")
        return 1
        
    # Validate schema
    validator = SchemaValidator()
    is_valid = validator.validate_schema(schema)
    
    if args.verbose or not is_valid:
        print(validator.get_report())
        
    if not is_valid:
        print("❌ Schema validation failed!")
        return 1
        
    if args.validate_only:
        print("✅ Schema validation passed!")
        return 0
        
    # Generate header
    try:
        generator = CHeaderGenerator()
        output_path = generator.generate_header(schema, args.output)
        print(f"Generated header: {output_path}")
        
        if args.verbose:
            print(f"Generated from: {args.schema_file}")
            print(f"Chip: {schema['metadata']['chip']}")
            print(f"Registers: {len(schema.get('registers', {}))}")
            if 'commands' in schema:
                print(f"Commands: {len(schema['commands'])}")
                
    except Exception as e:
        print(f"Error generating header: {e}")
        return 1
        
    return 0

if __name__ == "__main__":
    sys.exit(main())
