#!/usr/bin/env python3
"""
STM32H7xx Nucleo BSP Documentation Converter
Converts HTML documentation to markdown for enhanced search integration
"""

import os
import re
import json
from pathlib import Path
from bs4 import BeautifulSoup
import argparse


class NucleoBSPConverter:
    """Convert STM32H7xx Nucleo BSP HTML documentation to markdown"""

    def __init__(self, source_dir, output_dir):
        self.source_dir = Path(source_dir)
        self.output_dir = Path(output_dir)
        self.converted_files = []
        self.functions_found = set()
        self.constants_found = set()

    def convert_html_to_markdown(self, html_file):
        """Convert a single HTML file to markdown"""

        try:
            with open(html_file, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()

            soup = BeautifulSoup(content, "html.parser")

            # Extract title
            title_elem = soup.find("title")
            title = title_elem.get_text() if title_elem else html_file.stem
            title = title.replace("STM32H7xx_Nucleo BSP User Manual: ", "")

            # Start markdown content
            markdown = f"# {title}\n\n"

            # Extract main content area
            content_div = (
                soup.find("div", class_="contents")
                or soup.find("div", id="doc-content")
                or soup.body
            )

            if content_div:
                markdown += self._process_content_div(content_div)

            return markdown, title

        except Exception as e:
            print(f"Error processing {html_file}: {e}")
            return None, None

    def _process_content_div(self, div):
        """Process the main content div and extract useful information"""
        markdown = ""

        # Find function declarations
        functions = div.find_all(
            ["h3", "h4"], string=re.compile(r"BSP_.*|BUTTON_.*|LED_.*")
        )
        for func in functions:
            markdown += f"## {func.get_text()}\n\n"

            # Add function details
            next_sibling = func.find_next_sibling()
            while (next_sibling and
                   next_sibling.name not in ["h1", "h2", "h3", "h4"]):
                if next_sibling.name == "div" and next_sibling.find("table"):
                    table = next_sibling.find("table")
                    markdown += self._process_function_table(table)
                elif next_sibling.name == "p":
                    text = next_sibling.get_text().strip()
                    if text:
                        markdown += f"{text}\n\n"
                next_sibling = next_sibling.find_next_sibling()

        # Find constant definitions
        constants = div.find_all(
            string=re.compile(
                r"#define\s+.*BSP_.*|#define\s+.*LED_.*|#define\s+.*BUTTON_.*"
            )
        )
        if constants:
            markdown += "## Constants and Definitions\n\n"
            for const in constants:
                const_text = const.strip()
                if const_text:
                    markdown += f"```c\n{const_text}\n```\n\n"
                    # Extract constant name for indexing
                    match = re.search(r"#define\s+(\w+)", const_text)
                    if match:
                        self.constants_found.add(match.group(1))

        # Find function prototypes
        code_blocks = div.find_all("code")
        for code in code_blocks:
            code_text = code.get_text().strip()
            if (
                "BSP_" in code_text or "BUTTON_" in code_text or 
                "LED_" in code_text
            ) and "(" in code_text:
                markdown += f"```c\n{code_text}\n```\n\n"
                # Extract function name for indexing
                match = re.search(r"(\w+)\s*\(", code_text)
                if match:
                    self.functions_found.add(match.group(1))

        # Find tables with parameter descriptions
        tables = div.find_all("table")
        for table in tables:
            markdown += self._process_table(table)

        return markdown

    def _process_function_table(self, table):
        """Process function parameter/return value tables"""
        markdown = ""

        # Check if this is a parameters table
        headers = table.find_all("th")
        if headers and any("parameter" in h.get_text().lower() for h in headers):
            markdown += "| Parameter | Description |\n"
            markdown += "|-----------|-------------|\n"

            rows = table.find_all("tr")[1:]  # Skip header
            for row in rows:
                cells = row.find_all("td")
                if len(cells) >= 2:
                    param = cells[0].get_text().strip()
                    desc = cells[1].get_text().strip()
                    markdown += f"| {param} | {desc} |\n"

            markdown += "\n"

        return markdown

    def _process_table(self, table):
        """Process general tables"""
        markdown = ""

        rows = table.find_all("tr")
        if not rows:
            return markdown

        # Process header
        header_row = rows[0]
        headers = header_row.find_all(["th", "td"])
        if headers:
            markdown += (
                "| " + " | ".join(h.get_text().strip() for h in headers) + " |\n"
            )
            markdown += (
                "|"
                + "|".join(["-" * max(3, len(h.get_text().strip())) for h in headers])
                + "|\n"
            )

        # Process data rows
        for row in rows[1:]:
            cells = row.find_all("td")
            if cells:
                markdown += (
                    "| " + " | ".join(c.get_text().strip() for c in cells) + " |\n"
                )

        markdown += "\n"
        return markdown

    def convert_all(self):
        """Convert all HTML files in the source directory"""

        print(f"🔄 Converting STM32H7xx Nucleo BSP documentation...")
        print(f"📁 Source: {self.source_dir}")
        print(f"📁 Output: {self.output_dir}")

        # Create output directory
        self.output_dir.mkdir(parents=True, exist_ok=True)

        # Find all HTML files
        html_files = list(self.source_dir.glob("*.html"))

        print(f"📄 Found {len(html_files)} HTML files to convert")

        conversion_summary = {
            "source_directory": str(self.source_dir),
            "output_directory": str(self.output_dir),
            "total_files": len(html_files),
            "converted_files": [],
            "functions_found": [],
            "constants_found": [],
            "conversion_errors": [],
        }

        for html_file in html_files:
            try:
                print(f"🔄 Converting: {html_file.name}")

                markdown_content, title = self.convert_html_to_markdown(html_file)

                if markdown_content:
                    # Create output filename
                    output_file = self.output_dir / f"{html_file.stem}.md"

                    # Write markdown file
                    with open(output_file, "w", encoding="utf-8") as f:
                        f.write(markdown_content)

                    self.converted_files.append(str(output_file))
                    conversion_summary["converted_files"].append(
                        {
                            "source": str(html_file),
                            "output": str(output_file),
                            "title": title,
                        }
                    )

                    print(f"✅ Converted: {output_file.name}")
                else:
                    print(f"⚠️  Skipped: {html_file.name} (no content)")

            except Exception as e:
                error_msg = f"Error converting {html_file}: {e}"
                print(f"❌ {error_msg}")
                conversion_summary["conversion_errors"].append(error_msg)

        # Update summary with found functions and constants
        conversion_summary["functions_found"] = sorted(list(self.functions_found))
        conversion_summary["constants_found"] = sorted(list(self.constants_found))

        # Save conversion summary
        summary_file = self.output_dir / "conversion_summary.json"
        with open(summary_file, "w", encoding="utf-8") as f:
            json.dump(conversion_summary, f, indent=2)

        print(f"\n✅ Conversion complete!")
        print(f"📄 Converted: {len(self.converted_files)} files")
        print(f"🔧 Functions found: {len(self.functions_found)}")
        print(f"📋 Constants found: {len(self.constants_found)}")
        print(f"📊 Summary saved: {summary_file}")

        return conversion_summary

    def create_index_metadata(self):
        """Create metadata for search index integration"""

        metadata = {
            "documentation_set": "STM32H7xx_Nucleo_BSP",
            "description": "STM32H7xx Nucleo-144 Board Support Package Documentation",
            "version": "STM32H7 HAL Version",
            "total_files": len(self.converted_files),
            "functions": sorted(list(self.functions_found)),
            "constants": sorted(list(self.constants_found)),
            "search_categories": {
                "functions": ["BSP_LED_*", "BSP_BUTTON_*", "BSP_COM_*"],
                "constants": ["LED_*", "BUTTON_*", "COM_*"],
                "concepts": ["board_support", "nucleo_144", "bsp_drivers"],
            },
            "integration_ready": True,
        }

        metadata_file = self.output_dir / "index_metadata.json"
        with open(metadata_file, "w", encoding="utf-8") as f:
            json.dump(metadata, f, indent=2)

        print(f"📋 Index metadata created: {metadata_file}")
        return metadata


def main():
    parser = argparse.ArgumentParser(
        description="Convert STM32H7xx Nucleo BSP HTML docs to markdown"
    )
    parser.add_argument(
        "--source",
        default="/workspaces/code/00_reference/STM32H7xx_Nucleo_BSP_User_Manual",
        help="Source directory with HTML files",
    )
    parser.add_argument(
        "--output",
        default="/workspaces/code/00_reference/stm32h7xx_nucleo_bsp_md",
        help="Output directory for markdown files",
    )

    args = parser.parse_args()

    # Create converter and run conversion
    converter = NucleoBSPConverter(args.source, args.output)
    summary = converter.convert_all()
    metadata = converter.create_index_metadata()

    print(f"\n🎯 Next steps:")
    print(f"1. Update src/config/documentation_config.h with new paths")
    print(f"2. Generate search index:")
    print(
        f"   python3 scripts/documentation_manager.py index create --scope NUCLEO_BSP"
    )
    print(f"3. Test search functionality:")
    print(
        f"   python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP"
    )

    return summary


if __name__ == "__main__":
    main()
