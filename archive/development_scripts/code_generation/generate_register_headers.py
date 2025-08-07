#!/usr/bin/env python3
"""
Register Schema to C Header Generator
Auto-generates SSOT-compliant C headers from YAML register schemas
for STM32H753ZI stepper motor project.

Supports L6470 and AS5600 register definitions with validation.
"""

import yaml
import argparse
import os
from datetime import datetime
from pathlib import Path


class RegisterHeaderGenerator:
    """Generates C headers from register schema YAML files."""
    
    def __init__(self, schema_file: str, output_dir: str = "src/config"):
        self.schema_file = schema_file
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        with open(schema_file, 'r') as f:
            self.schema = yaml.safe_load(f)
            
        self.chip_name = self.schema['metadata']['chip']
        self.chip_lower = self.chip_name.lower()
        self.chip_upper = self.chip_name.upper()
        
    def generate_header(self) -> str:
        """Generate complete C header file content."""
        header_content = []
        
        # File header with metadata
        header_content.append(self._generate_file_header())
        
        # Include guards
        guard_name = f"{self.chip_upper}_REGISTERS_GENERATED_H"
        header_content.append(f"#ifndef {guard_name}")
        header_content.append(f"#define {guard_name}")
        header_content.append("")
        
        # Includes and dependencies
        header_content.append(self._generate_includes())
        
        # Register address definitions
        header_content.append(self._generate_register_addresses())
        
        # Register bit field definitions
        header_content.append(self._generate_bit_fields())
        
        # Command definitions (if present)
        if 'commands' in self.schema:
            header_content.append(self._generate_commands())
            
        # Combined register helper macros
        if 'combined_registers' in self.schema:
            header_content.append(self._generate_combined_registers())
            
        # Validation constants
        header_content.append(self._generate_validation_constants())
        
        # Function declarations
        header_content.append(self._generate_function_declarations())
        
        # Close include guard
        header_content.append(f"#endif /* {guard_name} */")
        header_content.append("")
        
        return '\n'.join(header_content)
        
    def _generate_file_header(self) -> str:
        """Generate file header with metadata and warnings."""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S UTC")
        
        return f"""/**
 * @file {self.chip_lower}_registers_generated.h
 * @brief {self.chip_name} Register Definitions - AUTO-GENERATED
 * 
 * THIS FILE IS AUTO-GENERATED FROM SCHEMA: {os.path.basename(self.schema_file)}
 * DO NOT EDIT MANUALLY - CHANGES WILL BE OVERWRITTEN
 * 
 * Generated: {timestamp}
 * Chip: {self.schema['metadata']['chip']}
 * Datasheet: {self.schema['metadata'].get('datasheet', 'N/A')}
 * Framework: {self.schema['metadata'].get('framework', 'Custom')}
 * 
 * @note This file provides SSOT register definitions for embedded driver
 * @note development. All register addresses, bit fields, and validation
 * @note constants are generated from the schema to ensure consistency.
 * 
 * @warning SAFETY CRITICAL - Validate all register configurations
 * @warning before deployment. Incorrect settings may damage hardware.
 */"""
        
    def _generate_includes(self) -> str:
        """Generate necessary includes."""
        includes = [
            "#include <stdint.h>",
            "#include <stdbool.h>",
            "",
            "/* SSOT Configuration Dependencies */",
            "#include \"hardware_config.h\"",
            "#include \"safety_config.h\"",
            "",
            "#ifdef __cplusplus",
            "extern \"C\" {",
            "#endif",
            ""
        ]
        return '\n'.join(includes)
        
    def _generate_register_addresses(self) -> str:
        """Generate register address definitions."""
        content = [
            f"/* ========================================================================== */",
            f"/* {self.chip_name} Register Addresses                                       */",
            f"/* ========================================================================== */",
            ""
        ]
        
        registers = self.schema.get('registers', {})
        for reg_name, reg_info in registers.items():
            address = reg_info['address']
            description = reg_info.get('description', '')
            
            # Generate register address constant
            const_name = f"{self.chip_upper}_REG_{reg_name}"
            content.append(f"#define {const_name:<30} 0x{address:02X}    /* {description} */")
            
        content.append("")
        return '\n'.join(content)
        
    def _generate_bit_fields(self) -> str:
        """Generate bit field definitions for registers with fields."""
        content = [
            f"/* ========================================================================== */", 
            f"/* {self.chip_name} Register Bit Field Definitions                           */",
            f"/* ========================================================================== */",
            ""
        ]
        
        registers = self.schema.get('registers', {})
        for reg_name, reg_info in registers.items():
            if 'fields' not in reg_info:
                continue
                
            content.append(f"/* {reg_name} Register Bit Fields */")
            
            for field_name, field_info in reg_info['fields'].items():
                if 'bits' in field_info:
                    bits = field_info['bits']
                    if len(bits) == 2:  # Range of bits
                        start_bit, end_bit = bits[0], bits[1]
                        if start_bit == end_bit:  # Single bit
                            mask = 1 << start_bit
                            const_name = f"{self.chip_upper}_{reg_name}_{field_name}"
                            content.append(f"#define {const_name:<40} (1U << {start_bit})")
                        else:  # Multi-bit field
                            mask = ((1 << (end_bit - start_bit + 1)) - 1) << start_bit
                            const_name = f"{self.chip_upper}_{reg_name}_{field_name}_Pos"
                            content.append(f"#define {const_name:<40} {start_bit}U")
                            const_name = f"{self.chip_upper}_{reg_name}_{field_name}_Msk"
                            content.append(f"#define {const_name:<40} (0x{mask:X}U)")
                            
                # Generate value definitions if present
                if 'values' in field_info:
                    for value, description in field_info['values'].items():
                        const_name = f"{self.chip_upper}_{reg_name}_{field_name}_{value}"
                        content.append(f"#define {const_name:<40} {value}U    /* {description} */")
                        
            content.append("")
            
        return '\n'.join(content)
        
    def _generate_commands(self) -> str:
        """Generate command definitions."""
        content = [
            f"/* ========================================================================== */",
            f"/* {self.chip_name} Command Definitions                                      */", 
            f"/* ========================================================================== */",
            ""
        ]
        
        commands = self.schema.get('commands', {})
        for cmd_name, cmd_info in commands.items():
            opcode = cmd_info['opcode']
            description = cmd_info.get('description', '')
            
            const_name = f"{self.chip_upper}_CMD_{cmd_name}"
            content.append(f"#define {const_name:<30} 0x{opcode:02X}    /* {description} */")
            
        content.append("")
        return '\n'.join(content)
        
    def _generate_combined_registers(self) -> str:
        """Generate helper macros for combined registers."""
        content = [
            f"/* ========================================================================== */",
            f"/* {self.chip_name} Combined Register Helper Macros                          */",
            f"/* ========================================================================== */",
            ""
        ]
        
        combined = self.schema.get('combined_registers', {})
        for comb_name, comb_info in combined.items():
            high_reg = comb_info['high_reg']
            low_reg = comb_info['low_reg']
            total_bits = comb_info['total_bits']
            
            # Macro to combine high and low bytes
            macro_name = f"{self.chip_upper}_COMBINE_{comb_name}"
            content.append(f"#define {macro_name}(high, low) \\")
            content.append(f"    (((uint16_t)(high) << 8) | (uint16_t)(low))")
            content.append("")
            
            # Macro to extract high byte
            macro_name = f"{self.chip_upper}_GET_{comb_name}_HIGH"
            content.append(f"#define {macro_name}(value) \\")
            content.append(f"    ((uint8_t)((value) >> 8))")
            content.append("")
            
            # Macro to extract low byte  
            macro_name = f"{self.chip_upper}_GET_{comb_name}_LOW"
            content.append(f"#define {macro_name}(value) \\")
            content.append(f"    ((uint8_t)((value) & 0xFF))")
            content.append("")
            
        return '\n'.join(content)
        
    def _generate_validation_constants(self) -> str:
        """Generate validation and safety constants."""
        content = [
            f"/* ========================================================================== */",
            f"/* {self.chip_name} Validation Constants                                     */",
            f"/* ========================================================================== */",
            ""
        ]
        
        # Safe defaults
        if 'safe_defaults' in self.schema:
            content.append(f"/* Safe Default Values */")
            defaults = self.schema['safe_defaults']
            for reg_name, default_value in defaults.items():
                const_name = f"{self.chip_upper}_SAFE_DEFAULT_{reg_name}"
                if isinstance(default_value, int):
                    content.append(f"#define {const_name:<40} 0x{default_value:02X}U")
                else:
                    content.append(f"#define {const_name:<40} {default_value}")
            content.append("")
            
        # Validation limits
        if 'validation' in self.schema:
            validation = self.schema['validation']
            
            # Critical registers list
            if 'critical_registers' in validation:
                content.append(f"/* Critical Registers for Safety Validation */")
                critical_regs = validation['critical_registers']
                content.append(f"#define {self.chip_upper}_CRITICAL_REG_COUNT    {len(critical_regs)}U")
                
                # Generate array of critical register addresses
                reg_list = []
                for reg_name in critical_regs:
                    reg_list.append(f"{self.chip_upper}_REG_{reg_name}")
                    
                content.append(f"#define {self.chip_upper}_CRITICAL_REGS \\")
                content.append(f"    {{ {', '.join(reg_list)} }}")
                content.append("")
                
        return '\n'.join(content)
        
    def _generate_function_declarations(self) -> str:
        """Generate function declarations for register operations."""
        content = [
            f"/* ========================================================================== */",
            f"/* {self.chip_name} Register Operation Functions                             */",
            f"/* ========================================================================== */",
            "",
            "/**",
            f" * @brief Validate {self.chip_name} register configuration",
            " * @param reg_addr Register address",
            " * @param value Register value",
            " * @return true if valid, false otherwise",
            " */",
            f"bool {self.chip_lower}_validate_register(uint8_t reg_addr, uint32_t value);",
            "",
            "/**",
            f" * @brief Get register size in bytes",
            " * @param reg_addr Register address", 
            " * @return Size in bytes (1-3)",
            " */",
            f"uint8_t {self.chip_lower}_get_register_size(uint8_t reg_addr);",
            "",
            "/**",
            f" * @brief Check if register is read-only",
            " * @param reg_addr Register address",
            " * @return true if read-only, false if writable",
            " */", 
            f"bool {self.chip_lower}_is_register_readonly(uint8_t reg_addr);",
            "",
            "/**",
            f" * @brief Get safe default value for register",
            " * @param reg_addr Register address",
            " * @return Default value or 0 if no default defined",
            " */",
            f"uint32_t {self.chip_lower}_get_safe_default(uint8_t reg_addr);",
            "",
            "#ifdef __cplusplus",
            "}",
            "#endif",
            ""
        ]
        
        return '\n'.join(content)
        
    def write_header(self) -> str:
        """Generate and write header file."""
        header_content = self.generate_header()
        
        # Determine output filename
        output_file = self.output_dir / f"{self.chip_lower}_registers_generated.h"
        
        with open(output_file, 'w') as f:
            f.write(header_content)
            
        return str(output_file)
        
    def generate_validation_code(self) -> str:
        """Generate corresponding C implementation for validation functions."""
        content = []
        
        # File header
        content.append(f"/**")
        content.append(f" * @file {self.chip_lower}_registers_generated.c")
        content.append(f" * @brief {self.chip_name} Register Validation Implementation - AUTO-GENERATED")
        content.append(f" * ")
        content.append(f" * THIS FILE IS AUTO-GENERATED FROM SCHEMA: {os.path.basename(self.schema_file)}")
        content.append(f" * DO NOT EDIT MANUALLY - CHANGES WILL BE OVERWRITTEN")
        content.append(f" */")
        content.append("")
        
        # Includes
        content.append(f"#include \"{self.chip_lower}_registers_generated.h\"")
        content.append("")
        
        # Register size lookup table
        content.append(f"/* Register size lookup table */")
        content.append(f"static const uint8_t {self.chip_lower}_register_sizes[] = {{")
        
        registers = self.schema.get('registers', {})
        for reg_name, reg_info in registers.items():
            size = reg_info.get('size_bytes', 1)
            address = reg_info['address']
            content.append(f"    [{address}] = {size},  /* {reg_name} */")
            
        content.append("};")
        content.append("")
        
        # Register access type lookup
        content.append(f"/* Register access type lookup */")
        content.append(f"static const bool {self.chip_lower}_register_readonly[] = {{")
        
        for reg_name, reg_info in registers.items():
            access = reg_info.get('access', 'read_write')
            is_readonly = 'true' if access == 'read_only' else 'false'
            address = reg_info['address']
            content.append(f"    [{address}] = {is_readonly},  /* {reg_name} */")
            
        content.append("};")
        content.append("")
        
        # Implementation of validation functions
        content.append(self._generate_validation_implementation())
        
        return '\n'.join(content)
        
    def _generate_validation_implementation(self) -> str:
        """Generate implementation of validation functions."""
        content = []
        
        # Size function
        content.append(f"uint8_t {self.chip_lower}_get_register_size(uint8_t reg_addr) {{")
        content.append(f"    if (reg_addr < sizeof({self.chip_lower}_register_sizes)) {{")
        content.append(f"        return {self.chip_lower}_register_sizes[reg_addr];")
        content.append(f"    }}")
        content.append(f"    return 0;  /* Invalid register */")
        content.append(f"}}")
        content.append("")
        
        # Read-only check function
        content.append(f"bool {self.chip_lower}_is_register_readonly(uint8_t reg_addr) {{")
        content.append(f"    if (reg_addr < sizeof({self.chip_lower}_register_readonly)) {{")
        content.append(f"        return {self.chip_lower}_register_readonly[reg_addr];")
        content.append(f"    }}")
        content.append(f"    return true;  /* Default to read-only for safety */")
        content.append(f"}}")
        content.append("")
        
        # Validation function
        content.append(f"bool {self.chip_lower}_validate_register(uint8_t reg_addr, uint32_t value) {{")
        content.append(f"    /* Basic size validation */")
        content.append(f"    uint8_t size = {self.chip_lower}_get_register_size(reg_addr);")
        content.append(f"    if (size == 0) return false;")
        content.append("")
        content.append(f"    /* Value range validation based on size */")
        content.append(f"    uint32_t max_value = (1UL << (size * 8)) - 1;")
        content.append(f"    if (value > max_value) return false;")
        content.append("")
        content.append(f"    /* Register-specific validation */")
        content.append(f"    switch (reg_addr) {{")
        
        # Add specific validation for registers with valid_range
        registers = self.schema.get('registers', {})
        for reg_name, reg_info in registers.items():
            if 'valid_range' in reg_info:
                address = reg_info['address']
                min_val, max_val = reg_info['valid_range']
                content.append(f"        case {self.chip_upper}_REG_{reg_name}:")
                content.append(f"            return (value >= 0x{min_val:X} && value <= 0x{max_val:X});")
                
        content.append(f"        default:")
        content.append(f"            return true;  /* No specific validation */")
        content.append(f"    }}")
        content.append(f"}}")
        content.append("")
        
        # Safe defaults function
        content.append(f"uint32_t {self.chip_lower}_get_safe_default(uint8_t reg_addr) {{")
        content.append(f"    switch (reg_addr) {{")
        
        # Add safe defaults from schema
        if 'safe_defaults' in self.schema:
            defaults = self.schema['safe_defaults']
            for reg_name, default_value in defaults.items():
                if reg_name in [r for r in registers.keys()]:  # Ensure register exists
                    address = registers[reg_name]['address']
                    content.append(f"        case {self.chip_upper}_REG_{reg_name}:")
                    content.append(f"            return 0x{default_value:X};")
                    
        content.append(f"        default:")
        content.append(f"            return 0;  /* No default defined */")
        content.append(f"    }}")
        content.append(f"}}")
        
        return '\n'.join(content)
        
    def write_validation_code(self) -> str:
        """Generate and write validation C file."""
        validation_content = self.generate_validation_code()
        
        output_file = self.output_dir / f"{self.chip_lower}_registers_generated.c"
        
        with open(output_file, 'w') as f:
            f.write(validation_content)
            
        return str(output_file)

def main():
    """Main function for command-line usage."""
    parser = argparse.ArgumentParser(description='Generate C headers from register schemas')
    parser.add_argument('schema_file', help='Path to YAML schema file')
    parser.add_argument('--output-dir', default='src/config', 
                       help='Output directory for generated files')
    parser.add_argument('--validation', action='store_true',
                       help='Also generate validation C implementation')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.schema_file):
        print(f"Error: Schema file {args.schema_file} not found")
        return 1
        
    try:
        generator = RegisterHeaderGenerator(args.schema_file, args.output_dir)
        
        # Generate header file
        header_file = generator.write_header()
        print(f"Generated header: {header_file}")
        
        # Generate validation code if requested
        if args.validation:
            validation_file = generator.write_validation_code()
            print(f"Generated validation: {validation_file}")
            
        print("Register header generation complete!")
        return 0
        
    except Exception as e:
        print(f"Error generating headers: {e}")
        return 1

if __name__ == '__main__':
    exit(main())
