#!/usr/bin/env python3
"""
Documentation Search and Index Management Tool
Unified tool for all documentation search and index creation functionality
Combines and replaces older search_docs.py and create_instruction_index.py
Uses SSOT documentation configuration for all paths
"""

import argparse
import json
import re
import time
from collections import defaultdict
from pathlib import Path

import yaml

# SSOT Documentation Configuration - matches documentation_config.h
WORKSPACE_ROOT = Path('/workspaces/code')
DOC_INDEXES_DIR = WORKSPACE_ROOT / "docs" / "indexes"
REFERENCE_DIR = WORKSPACE_ROOT / "00_reference"

# Search index file paths (SSOT)
STM32H7_INDEX_PATH = DOC_INDEXES_DIR / "STM32H7_FULL_INDEX.json"
L6470_INDEX_PATH = DOC_INDEXES_DIR / "L6470_SEARCH_INDEX.json"
STM32H7_COPILOT_INDEX_YAML = DOC_INDEXES_DIR / "STM32H7_COPILOT_INDEX.yaml"
STM32H7_COPILOT_INDEX_JSON = DOC_INDEXES_DIR / "STM32H7_COPILOT_INDEX.json"

# Documentation source paths (SSOT)
STM32H7_DOC_ROOT = REFERENCE_DIR / "stm32h753xx_User_Manual_md"
L6470_DOC_ROOT = REFERENCE_DIR / "x_cube_spn2_markdown_docs"


def create_stm32h7_index():
    """Create comprehensive STM32H7 documentation index"""
    print("🔍 Creating STM32H7 documentation index...")

    index_data = {
        'peripheral_index': defaultdict(list),
        'function_index': defaultdict(list),
        'concept_index': defaultdict(list),
        'file_metadata': {},
        'statistics': {}
    }

    total_files = 0
    total_size = 0

    if not STM32H7_DOC_ROOT.exists():
        print(
            f"❌ STM32H7 documentation directory not found: {STM32H7_DOC_ROOT}")
        return False

    for md_file in STM32H7_DOC_ROOT.rglob('*.md'):
        total_files += 1
        file_size = md_file.stat().st_size
        total_size += file_size

        relative_path = str(md_file.relative_to(STM32H7_DOC_ROOT))

        # Store file metadata
        index_data['file_metadata'][relative_path] = {
            'size_kb': round(file_size / 1024, 1),
            'full_path': str(md_file)
        }

        # Extract keywords from file content
        keywords = extract_keywords_from_file(md_file)

        # Index by peripheral
        for keyword in keywords:
            peripheral_match = re.match(
                r'^(GPIO|UART|USART|SPI|I2C|ADC|DAC|TIM|DMA|DMAMUX|RCC|PWR|EXTI|NVIC|CRC|RNG|HASH|CRYP|LTDC|DSI|DCMI|ETH|USB|CAN|FDCAN|SAI|DFSDM|MDIOS|SDMMC|FMC|QUADSPI|OCTOSPI|HSEM|WWDG|IWDG|RTC|TAMP|LPTIM|LPUART|SWPMI|OPAMP|COMP|VREFBUF|CORDIC|FMAC)',
                keyword)
            if peripheral_match:
                index_data['peripheral_index'][peripheral_match.group(
                    1)].append(relative_path)

            # Index functions
            if keyword.startswith(('HAL_', 'LL_')):
                index_data['function_index'][keyword].append(relative_path)

            # Index concepts
            concept_keywords = [
                'interrupt',
                'IRQ',
                'callback',
                'DMA',
                'initialization',
                'configuration',
                'polling',
                'blocking',
                'buffer',
                'FIFO',
                'register',
                'clock',
                'timeout',
                'error']
            if any(concept in keyword.lower() for concept in concept_keywords):
                index_data['concept_index'][keyword.lower()].append(
                    relative_path)

    # Add statistics
    index_data['statistics'] = {
        'total_files': total_files,
        'total_size_mb': round(total_size / (1024 * 1024), 1),
        'peripherals': len(index_data['peripheral_index']),
        'functions': len(index_data['function_index']),
        'concepts': len(index_data['concept_index']),
        'created_timestamp': time.time()
    }

    # Save index
    DOC_INDEXES_DIR.mkdir(parents=True, exist_ok=True)
    with open(STM32H7_INDEX_PATH, 'w') as f:
        json.dump(dict(index_data), f, indent=2)

    print(
        f"✅ STM32H7 index created: {total_files} files, {round(total_size / (1024 * 1024), 1)}MB")
    return True


def create_l6470_index():
    """Create L6470 documentation index"""
    print("🔍 Creating L6470 documentation index...")

    index_data = {
        'register_index': defaultdict(list),
        'function_index': defaultdict(list),
        'command_index': defaultdict(list),
        'file_metadata': {},
        'statistics': {}
    }

    total_files = 0
    total_size = 0

    if not L6470_DOC_ROOT.exists():
        print(f"❌ L6470 documentation directory not found: {L6470_DOC_ROOT}")
        return False

    for md_file in L6470_DOC_ROOT.rglob('*.md'):
        total_files += 1
        file_size = md_file.stat().st_size
        total_size += file_size

        relative_path = str(md_file.relative_to(L6470_DOC_ROOT))

        # Store file metadata
        index_data['file_metadata'][relative_path] = {
            'size_kb': round(file_size / 1024, 1),
            'full_path': str(md_file)
        }

        # Extract L6470-specific keywords
        try:
            with open(md_file, 'r', encoding='utf-8') as f:
                content = f.read()
        except BaseException:
            continue

        # L6470 registers
        registers = re.findall(
            r'\b(ABS_POS|EL_POS|MARK|SPEED|ACC|DEC|MAX_SPEED|MIN_SPEED|KVAL_HOLD|KVAL_RUN|KVAL_ACC|KVAL_DEC|INT_SPEED|ST_SLP|FN_SLP_ACC|FN_SLP_DEC|K_THERM|ADC_OUT|OCD_TH|STALL_TH|FS_SPD|STEP_MODE|ALARM_EN|CONFIG|STATUS)\b',
            content)
        for reg in registers:
            index_data['register_index'][reg].append(relative_path)

        # L6470 functions
        functions = re.findall(r'\b(L6470_[A-Za-z0-9_]+)\b', content)
        for func in functions:
            index_data['function_index'][func].append(relative_path)

        # L6470 commands
        commands = re.findall(
            r'\b(NOP|SetParam|GetParam|Run|StepClock|Move|GoTo|GoTo_DIR|GoUntil|ReleaseSW|GoHome|GoMark|ResetPos|ResetDevice|SoftStop|HardStop|SoftHiZ|HardHiZ|GetStatus)\b',
            content)
        for cmd in commands:
            index_data['command_index'][cmd].append(relative_path)

    # Add statistics
    index_data['statistics'] = {
        'total_files': total_files,
        'total_size_mb': round(total_size / (1024 * 1024), 1),
        'registers': len(index_data['register_index']),
        'functions': len(index_data['function_index']),
        'commands': len(index_data['command_index']),
        'created_timestamp': time.time()
    }

    # Save index
    with open(L6470_INDEX_PATH, 'w') as f:
        json.dump(dict(index_data), f, indent=2)

    print(
        f"✅ L6470 index created: {total_files} files, {round(total_size / (1024 * 1024), 1)}MB")
    return True


def extract_keywords_from_file(file_path):
    """Extract relevant keywords from a markdown file"""
    keywords = set()

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except BaseException:
        return keywords

    # Extract function names (HAL_*, LL_*)
    hal_functions = re.findall(r'\b(HAL_[A-Z_][A-Za-z0-9_]*)\b', content)
    ll_functions = re.findall(r'\b(LL_[A-Z_][A-Za-z0-9_]*)\b', content)
    keywords.update(hal_functions + ll_functions)

    # Extract peripheral names
    peripherals = re.findall(
        r'\b(GPIO|UART|USART|SPI|I2C|ADC|DAC|TIM|DMA|DMAMUX|RCC|PWR|EXTI|NVIC|CRC|RNG|HASH|CRYP|LTDC|DSI|DCMI|ETH|USB|CAN|FDCAN|SAI|DFSDM|MDIOS|SDMMC|FMC|QUADSPI|OCTOSPI|HSEM|WWDG|IWDG|RTC|TAMP|LPTIM|LPUART|SWPMI|OPAMP|COMP|VREFBUF|CORDIC|FMAC)\b',
        content)
    keywords.update(peripherals)

    # Extract structure names
    structs = re.findall(
        r'\b([A-Z][A-Za-z0-9_]*_(?:InitTypeDef|HandleTypeDef|TypeDef))\b',
        content)
    keywords.update(structs)

    # Extract defines/constants
    defines = re.findall(r'#define\s+([A-Z_][A-Z0-9_]*)', content)
    keywords.update(defines)

    return keywords


def search_documentation(search_type, query, scope='all'):
    """Unified search across all documentation"""
    results = []

    if scope in ['all', 'STM32H7']:
        results.extend(search_stm32h7(search_type, query))

    if scope in ['all', 'L6470']:
        results.extend(search_l6470(search_type, query))

    return results


def search_stm32h7(search_type, query):
    """Search STM32H7 documentation"""
    if not STM32H7_INDEX_PATH.exists():
        print(f"❌ STM32H7 index not found. Run with --create-indexes first.")
        return []

    with open(STM32H7_INDEX_PATH, 'r') as f:
        index = json.load(f)

    results = []

    if search_type == 'peripheral':
        query = query.upper()
        if query in index['peripheral_index']:
            files = index['peripheral_index'][query]
            for file in files[:10]:  # Limit results
                if file in index['file_metadata']:
                    results.append({
                        'file': file,
                        'source': 'STM32H7',
                        'type': 'peripheral',
                        'size_kb': index['file_metadata'][file]['size_kb'],
                        'full_path': index['file_metadata'][file]['full_path']
                    })

    elif search_type == 'function':
        if query in index['function_index']:
            files = index['function_index'][query]
            for file in files[:10]:
                if file in index['file_metadata']:
                    results.append({
                        'file': file,
                        'source': 'STM32H7',
                        'type': 'function',
                        'size_kb': index['file_metadata'][file]['size_kb'],
                        'full_path': index['file_metadata'][file]['full_path']
                    })

    elif search_type == 'concept':
        query = query.lower()
        for concept, files in index['concept_index'].items():
            if query in concept:
                for file in files[:5]:  # Fewer results for concepts
                    if file in index['file_metadata']:
                        results.append({
                            'file': file,
                            'source': 'STM32H7',
                            'type': 'concept',
                            'concept': concept,
                            'size_kb': index['file_metadata'][file]['size_kb'],
                            'full_path': index['file_metadata'][file]['full_path']
                        })

    return results


def search_l6470(search_type, query):
    """Search L6470 documentation"""
    if not L6470_INDEX_PATH.exists():
        print(f"❌ L6470 index not found. Run with --create-indexes first.")
        return []

    with open(L6470_INDEX_PATH, 'r') as f:
        index = json.load(f)

    results = []

    if search_type == 'register':
        query = query.upper()
        if query in index['register_index']:
            files = index['register_index'][query]
            for file in files[:10]:
                if file in index['file_metadata']:
                    results.append({
                        'file': file,
                        'source': 'L6470',
                        'type': 'register',
                        'size_kb': index['file_metadata'][file]['size_kb'],
                        'full_path': index['file_metadata'][file]['full_path']
                    })

    elif search_type == 'function':
        if query in index['function_index']:
            files = index['function_index'][query]
            for file in files[:10]:
                if file in index['file_metadata']:
                    results.append({
                        'file': file,
                        'source': 'L6470',
                        'type': 'function',
                        'size_kb': index['file_metadata'][file]['size_kb'],
                        'full_path': index['file_metadata'][file]['full_path']
                    })

    elif search_type == 'command':
        if query in index['command_index']:
            files = index['command_index'][query]
            for file in files[:10]:
                if file in index['file_metadata']:
                    results.append({
                        'file': file,
                        'source': 'L6470',
                        'type': 'command',
                        'size_kb': index['file_metadata'][file]['size_kb'],
                        'full_path': index['file_metadata'][file]['full_path']
                    })

    return results


def display_results(results, query):
    """Display search results in a formatted way"""
    if not results:
        print(f"❌ No results found for '{query}'")
        return

    print(f"🔍 Found {len(results)} results for '{query}':")
    print("=" * 50)

    # Group by source
    stm32h7_results = [r for r in results if r['source'] == 'STM32H7']
    l6470_results = [r for r in results if r['source'] == 'L6470']

    if stm32h7_results:
        print(f"\n📘 STM32H7 Documentation ({len(stm32h7_results)} files):")
        for result in stm32h7_results:
            print(
                f"   • {result['file']} ({result['size_kb']}KB) - {result['type']}")

    if l6470_results:
        print(f"\n📗 L6470 Documentation ({len(l6470_results)} files):")
        for result in l6470_results:
            print(
                f"   • {result['file']} ({result['size_kb']}KB) - {result['type']}")


def main():
    """Main function with command-line interface"""
    parser = argparse.ArgumentParser(
        description='Documentation Search and Index Management Tool')

    subparsers = parser.add_subparsers(
        dest='command', help='Available commands')

    # Index management commands
    index_parser = subparsers.add_parser(
        'index', help='Manage documentation indexes')
    index_parser.add_argument(
        'action',
        choices=[
            'create',
            'recreate'],
        help='Index action')
    index_parser.add_argument(
        '--scope',
        choices=[
            'all',
            'STM32H7',
            'L6470'],
        default='all',
        help='Index scope')

    # Search commands
    search_parser = subparsers.add_parser(
        'search', help='Search documentation')
    search_parser.add_argument(
        'type',
        choices=[
            'peripheral',
            'function',
            'register',
            'command',
            'concept'],
        help='Search type')
    search_parser.add_argument('query', help='Search query')
    search_parser.add_argument(
        '--scope',
        choices=[
            'all',
            'STM32H7',
            'L6470'],
        default='all',
        help='Search scope')

    args = parser.parse_args()

    if args.command == 'index':
        print("📚 Documentation Index Management")
        print("=" * 40)

        if args.scope in ['all', 'STM32H7']:
            create_stm32h7_index()

        if args.scope in ['all', 'L6470']:
            create_l6470_index()

    elif args.command == 'search':
        print("🔍 Documentation Search")
        print("=" * 40)

        results = search_documentation(args.type, args.query, args.scope)
        display_results(results, args.query)

    else:
        parser.print_help()


if __name__ == "__main__":
    main()
