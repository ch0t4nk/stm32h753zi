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

# SSOT Documentation Configuration - matches documentation_config.h
WORKSPACE_ROOT = Path("/workspaces/code")
DOC_INDEXES_DIR = WORKSPACE_ROOT / "docs" / "indexes"
REFERENCE_DIR = WORKSPACE_ROOT / "00_reference"

# Search index file paths (SSOT)
STM32H7_INDEX_PATH = DOC_INDEXES_DIR / "STM32H7_FULL_INDEX.json"
L6470_INDEX_PATH = DOC_INDEXES_DIR / "L6470_SEARCH_INDEX.json"
STM32H7_COPILOT_INDEX_YAML = DOC_INDEXES_DIR / "STM32H7_COPILOT_INDEX.yaml"
STM32H7_COPILOT_INDEX_JSON = DOC_INDEXES_DIR / "STM32H7_COPILOT_INDEX.json"

# Documentation source paths (SSOT) - UNIFIED STM32H7 DOCUMENTATION
STM32H7_DOC_LOCATIONS = [
    REFERENCE_DIR,  # Root-level .md files (RM0433, OM0253, ES0392, etc.)
    REFERENCE_DIR / "stm32h753xx_User_Manual_md",  # STM32H7 HAL docs
]
L6470_DOC_ROOT = REFERENCE_DIR / "x_cube_spn2_markdown_docs"


def create_unified_stm32h7_index():
    """Create unified STM32H7 documentation index from all sources."""
    print("🔍 Creating unified STM32H7 documentation index...")

    index = {
        "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
        "description": "Unified STM32H7 documentation search index",
        "sources": [str(path) for path in STM32H7_DOC_LOCATIONS],
        "file_metadata": {},
        "keyword_index": defaultdict(list),
        "concept_index": defaultdict(list),
        "function_index": defaultdict(list),
        "register_index": defaultdict(list),
        "peripheral_index": defaultdict(list),
    }

    total_files = 0
    total_size = 0

    for doc_location in STM32H7_DOC_LOCATIONS:
        print(f"   📂 Processing: {doc_location}")

        if doc_location == REFERENCE_DIR:
            # Process only .md files in root
            md_files = list(doc_location.glob("*.md"))
        else:
            # Process all .md files recursively in subdirectories
            md_files = list(doc_location.rglob("*.md"))

        for md_file in md_files:
            try:
                content = md_file.read_text(encoding="utf-8", errors="ignore")
                size_kb = len(content) // 1024

                # Use relative path from workspace root for consistency
                relative_path = str(md_file.relative_to(WORKSPACE_ROOT))

                # Store file metadata
                index["file_metadata"][relative_path] = {
                    "size_kb": size_kb,
                    "full_path": str(md_file),
                }

                # Extract and index different types of keywords
                _extract_unified_keywords(content, relative_path, index)

                total_files += 1
                total_size += len(content)

            except Exception as e:
                print(f"   ⚠️ Error processing {md_file}: {e}")

    # Save the unified index
    STM32H7_INDEX_PATH.parent.mkdir(parents=True, exist_ok=True)
    with open(STM32H7_INDEX_PATH, "w") as f:
        json.dump(index, f, indent=2)

    print(
        f"✅ Unified STM32H7 index created: {total_files} files, "
        f"{round(total_size / (1024 * 1024), 1)}MB"
    )
    print(f"   📄 Keywords: {len(index['keyword_index'])}")
    print(f"   🧠 Concepts: {len(index['concept_index'])}")
    print(f"   ⚙️ Functions: {len(index['function_index'])}")
    print(f"   📟 Registers: {len(index['register_index'])}")
    print(f"   🔧 Peripherals: {len(index['peripheral_index'])}")
    return True


def create_copilot_optimized_index():
    """Create a Copilot-optimized quick reference index for token
    efficiency."""
    print("🤖 Creating Copilot-optimized reference index...")

    if not STM32H7_INDEX_PATH.exists():
        print("❌ Main STM32H7 index not found. Create it first.")
        return False

    with open(STM32H7_INDEX_PATH, "r") as f:
        main_index = json.load(f)

    # Create optimized index with most common queries
    copilot_index = {
        "version": "2.0",
        "timestamp": time.strftime("%Y-%m-%d %H:%M:%S"),
        "description": "Copilot-optimized STM32H7 + L6470 quick reference",
        "usage": "Optimized for token efficiency in AI assistance",
        # Quick reference by category (most common queries)
        "quick_reference": {
            "motor_control": {
                "description": "Stepper motor control with L6470 driver",
                "key_files": [],
                "peripherals": ["SPI", "GPIO", "TIM"],
                "functions": [],
                "registers": ["ABS_POS", "SPEED", "ACC", "DEC", "CONFIG"],
            },
            "communication": {
                "description": "SPI, I2C, UART, CAN protocols",
                "key_files": [],
                "peripherals": ["SPI", "I2C", "UART", "CAN", "FDCAN"],
                "functions": [],
                "concepts": ["daisy_chain", "interrupt", "dma"],
            },
            "safety": {
                "description": "Safety systems, error handling, MPU",
                "key_files": [],
                "peripherals": ["MPU", "WWDG", "IWDG"],
                "concepts": [
                    "memory_protection",
                    "error_correction",
                    "hardware_errata",
                ],
            },
            "architecture": {
                "description": "STM32H7 system architecture and performance",
                "key_files": [],
                "concepts": ["cache", "TCM", "AXI", "performance_tuning"],
            },
        },
        # Most relevant files by category
        "essential_files": {
            "motor_control": [],
            "communication": [],
            "safety": [],
            "architecture": [],
            "hal_reference": [],
        },
        # Function quick lookup (top 100 most relevant)
        "top_functions": {},
        # Common code patterns
        "code_patterns": {
            "l6470_init": "L6470 initialization sequence",
            "spi_setup": "SPI configuration for L6470",
            "i2c_encoder": "I2C setup for AS5600 encoder",
            "gpio_motor": "GPIO configuration for motor control",
            "safety_init": "Safety system initialization",
        },
    }

    # Populate essential files for each category
    for category, files in main_index.get("concept_index", {}).items():
        if "motor" in category or "stepper" in category:
            copilot_index["essential_files"]["motor_control"].extend(files[:3])
        elif (
            "communication" in category
            or "spi" in category
            or "i2c" in category
        ):
            copilot_index["essential_files"]["communication"].extend(files[:3])
        elif (
            "safety" in category
            or "protection" in category
            or "errata" in category
        ):
            copilot_index["essential_files"]["safety"].extend(files[:3])
        elif "architecture" in category or "memory" in category:
            copilot_index["essential_files"]["architecture"].extend(files[:3])
        elif "hal" in category:
            copilot_index["essential_files"]["hal_reference"].extend(files[:3])

    # Remove duplicates and limit files per category
    for category in copilot_index["essential_files"]:
        files = list(set(copilot_index["essential_files"][category]))[:5]
        copilot_index["essential_files"][category] = files

    # Top functions (limit to most important)
    function_count = {}
    for func, files in main_index.get("function_index", {}).items():
        function_count[func] = len(files)

    # Get top 50 functions by frequency
    top_functions = sorted(
        function_count.items(), key=lambda x: x[1], reverse=True
    )[:50]
    for func, count in top_functions:
        primary_file = (
            main_index["function_index"][func][0]
            if main_index["function_index"][func]
            else None
        )
        copilot_index["top_functions"][func] = {
            "file_count": count,
            "primary_file": primary_file,
        }

    # Save the optimized index
    copilot_yaml_path = STM32H7_COPILOT_INDEX_YAML
    copilot_json_path = STM32H7_COPILOT_INDEX_JSON

    # YAML format for human readability
    try:
        import yaml

        with open(copilot_yaml_path, "w") as f:
            yaml.dump(copilot_index, f, default_flow_style=False, indent=2)
    except ImportError:
        print("   ⚠️ YAML not available, skipping YAML output")

    # JSON format for programmatic access
    with open(copilot_json_path, "w") as f:
        json.dump(copilot_index, f, indent=2)

    print("✅ Copilot-optimized index created")
    total_files = sum(
        len(files) for files in copilot_index["essential_files"].values()
    )
    print(f"   📄 Essential files: {total_files}")
    print(f"   ⚙️ Top functions: {len(copilot_index['top_functions'])}")
    print(f"   💾 JSON: {copilot_json_path.name}")
    print(f"   📝 YAML: {copilot_yaml_path.name}")

    return True


def create_l6470_index():
    """Create L6470 documentation index"""
    print("🔍 Creating L6470 documentation index...")

    index_data = {
        "register_index": defaultdict(list),
        "function_index": defaultdict(list),
        "command_index": defaultdict(list),
        "file_metadata": {},
        "statistics": {},
    }

    total_files = 0
    total_size = 0

    if not L6470_DOC_ROOT.exists():
        print(f"❌ L6470 documentation directory not found: {L6470_DOC_ROOT}")
        return False

    for md_file in L6470_DOC_ROOT.rglob("*.md"):
        total_files += 1
        file_size = md_file.stat().st_size
        total_size += file_size

        relative_path = str(md_file.relative_to(L6470_DOC_ROOT))

        # Store file metadata
        index_data["file_metadata"][relative_path] = {
            "size_kb": round(file_size / 1024, 1),
            "full_path": str(md_file),
        }

        # Extract L6470-specific keywords
        try:
            with open(md_file, "r", encoding="utf-8") as f:
                content = f.read()
        except BaseException:
            continue

        # L6470 registers
        registers = re.findall(
            r"\b(ABS_POS|EL_POS|MARK|SPEED|ACC|DEC|MAX_SPEED|MIN_SPEED|KVAL_HOLD|KVAL_RUN|KVAL_ACC|KVAL_DEC|INT_SPEED|ST_SLP|FN_SLP_ACC|FN_SLP_DEC|K_THERM|ADC_OUT|OCD_TH|STALL_TH|FS_SPD|STEP_MODE|ALARM_EN|CONFIG|STATUS)\b",  # noqa: E501
            content,
        )
        for reg in registers:
            index_data["register_index"][reg].append(relative_path)

        # L6470 functions
        functions = re.findall(r"\b(L6470_[A-Za-z0-9_]+)\b", content)
        for func in functions:
            index_data["function_index"][func].append(relative_path)

        # L6470 commands
        commands = re.findall(
            r"\b(NOP|SetParam|GetParam|Run|StepClock|Move|GoTo|GoTo_DIR|GoUntil|ReleaseSW|GoHome|GoMark|ResetPos|ResetDevice|SoftStop|HardStop|SoftHiZ|HardHiZ|GetStatus)\b",  # noqa: E501
            content,
        )
        for cmd in commands:
            index_data["command_index"][cmd].append(relative_path)

    # Add statistics
    index_data["statistics"] = {
        "total_files": total_files,
        "total_size_mb": round(total_size / (1024 * 1024), 1),
        "registers": len(index_data["register_index"]),
        "functions": len(index_data["function_index"]),
        "commands": len(index_data["command_index"]),
        "created_timestamp": time.time(),
    }

    # Save index
    with open(L6470_INDEX_PATH, "w") as f:
        json.dump(dict(index_data), f, indent=2)

    print(
        f"✅ L6470 index created: {total_files} files, {round(total_size / (1024 * 1024), 1)}MB"
    )  # noqa: E501
    return True


def _extract_unified_keywords(content, file_path, index):
    """Extract and index all types of keywords optimized for Copilot usage."""

    # STM32H7 Peripherals (from original pattern)
    peripherals = re.findall(
        r"\b(GPIO|UART|USART|SPI|I2C|ADC|DAC|TIM|DMA|DMAMUX|RCC|PWR|EXTI|NVIC|CRC|RNG|HASH|CRYP|LTDC|DSI|DCMI|ETH|USB|CAN|FDCAN|SAI|DFSDM|MDIOS|SDMMC|FMC|QUADSPI|OCTOSPI|HSEM|WWDG|IWDG|RTC|TAMP|LPTIM|LPUART|BDMA|MDMA|SWPMI|SPDIFRX|COMP|OPAMP|VREF|TSC|FIREWALL|MPU|FPU|ITM|DWT|SYSTICK)\b",  # noqa: E501
        content,
    )
    for peripheral in peripherals:
        index["peripheral_index"][peripheral].append(file_path)

    # L6470 Registers (for L6470 docs or mixed content)
    l6470_registers = re.findall(
        r"\b(ABS_POS|EL_POS|MARK|SPEED|ACC|DEC|MAX_SPEED|MIN_SPEED|KVAL_HOLD|KVAL_RUN|KVAL_ACC|KVAL_DEC|INT_SPEED|ST_SLP|FN_SLP_ACC|FN_SLP_DEC|K_THERM|ADC_OUT|OCD_TH|STALL_TH|FS_SPD|STEP_MODE|ALARM_EN|CONFIG|STATUS)\b",  # noqa: E501
        content,
    )
    for reg in l6470_registers:
        index["register_index"][reg].append(file_path)

    # Functions (HAL, LL, L6470)
    functions = re.findall(
        r"\b(HAL_[A-Z_][A-Za-z0-9_]*|LL_[A-Z_][A-Za-z0-9_]*|L6470_[A-Za-z0-9_]+)\b",  # noqa: E501
        content,
    )
    for func in functions:
        index["function_index"][func].append(file_path)

    # L6470 Commands
    l6470_commands = re.findall(
        r"\b(NOP|SetParam|GetParam|Run|StepClock|Move|GoTo|GoTo_DIR|GoUntil|ReleaseSW|GoHome|GoMark|ResetPos|ResetDevice|SoftStop|HardStop|SoftHiZ|HardHiZ|GetStatus)\b",  # noqa: E501
        content,
    )
    for cmd in l6470_commands:
        index["function_index"][cmd].append(file_path)

    # COPILOT-OPTIMIZED CONCEPT EXTRACTION
    # Group concepts by common Copilot query patterns

    # Motor Control & Stepper Motor Concepts
    motor_pattern = (
        r"\b(stepper|motor|L6470|IHM02A1|step|microstep|torque|"
        r"current|position|velocity|acceleration|deceleration)\b"
    )
    if re.search(motor_pattern, content, re.IGNORECASE):
        index["concept_index"]["motor_control"].append(file_path)
        index["concept_index"]["stepper_motor"].append(file_path)

    # Encoder & Feedback Concepts
    encoder_pattern = (
        r"\b(encoder|AS5600|magnetic|position|feedback|"
        r"closed.loop|sensor|angle|rotation)\b"
    )
    if re.search(encoder_pattern, content, re.IGNORECASE):
        index["concept_index"]["encoder_feedback"].append(file_path)
        index["concept_index"]["position_sensing"].append(file_path)

    # Communication Protocols
    comm_pattern = (
        r"\b(SPI|I2C|UART|CAN|FDCAN|communication|protocol|"
        r"interface|bus|message|frame)\b"
    )
    if re.search(comm_pattern, content, re.IGNORECASE):
        index["concept_index"]["communication"].append(file_path)
    if re.search(
        r"\b(SPI|MOSI|MISO|SCK|CS|daisy.chain)\b", content, re.IGNORECASE
    ):
        index["concept_index"]["spi_communication"].append(file_path)
    if re.search(
        r"\b(I2C|SCL|SDA|address|slave|master)\b", content, re.IGNORECASE
    ):
        index["concept_index"]["i2c_communication"].append(file_path)
    if re.search(
        r"\b(CAN|FDCAN|identifier|filter|mailbox|FIFO)\b",
        content,
        re.IGNORECASE,
    ):
        index["concept_index"]["can_communication"].append(file_path)
    if re.search(
        r"\b(UART|USART|baud|rate|TX|RX|DMA|interrupt)\b",
        content,
        re.IGNORECASE,
    ):
        index["concept_index"]["uart_communication"].append(file_path)

    # Safety & Error Handling
    safety_pattern = (
        r"\b(safety|fault|error|protection|watchdog|MPU|ECC|"
        r"memory.protection|fail.safe)\b"
    )
    if re.search(safety_pattern, content, re.IGNORECASE):
        index["concept_index"]["safety_systems"].append(file_path)
    if re.search(
        r"\b(errata|bug|workaround|limitation|silicon|revision)\b",
        content,
        re.IGNORECASE,
    ):
        index["concept_index"]["hardware_errata"].append(file_path)
    if re.search(
        r"\b(MPU|memory.protection|region|access|privilege)\b",
        content,
        re.IGNORECASE,
    ):
        index["concept_index"]["memory_protection"].append(file_path)
    ecc_pattern = (
        r"\b(ECC|error.correction|single.bit|double.bit|"
        r"memory.integrity)\b"
    )
    if re.search(ecc_pattern, content, re.IGNORECASE):
        index["concept_index"]["error_correction"].append(file_path)

    # STM32H7 Architecture & Performance
    arch_pattern = (
        r"\b(STM32H7|Cortex.M7|ARM|architecture|cache|TCM|AXI|"
        r"AHB|performance)\b"
    )
    if re.search(arch_pattern, content, re.IGNORECASE):
        index["concept_index"]["stm32h7_architecture"].append(file_path)
    mem_pattern = (
        r"\b(cache|ITCM|DTCM|memory.interface|bus.matrix|" r"performance)\b"
    )
    if re.search(mem_pattern, content, re.IGNORECASE):
        index["concept_index"]["memory_architecture"].append(file_path)
    if re.search(
        r"\b(clock|PLL|prescaler|frequency|HSE|HSI|LSE|LSI)\b",
        content,
        re.IGNORECASE,
    ):
        index["concept_index"]["clock_configuration"].append(file_path)

    # GPIO & Pin Configuration
    if re.search(
        r"\b(GPIO|pin|port|alternate.function|EXTI|interrupt)\b",
        content,
        re.IGNORECASE,
    ):
        index["concept_index"]["gpio_configuration"].append(file_path)

    # Timer & PWM
    timer_pattern = (
        r"\b(timer|TIM|PWM|capture|compare|encoder.mode|" r"trigger)\b"
    )
    if re.search(timer_pattern, content, re.IGNORECASE):
        index["concept_index"]["timer_pwm"].append(file_path)

    # DMA & Data Transfer
    dma_pattern = (
        r"\b(DMA|DMAMUX|stream|channel|transfer|memory.to.memory|"
        r"peripheral.to.memory)\b"
    )
    if re.search(dma_pattern, content, re.IGNORECASE):
        index["concept_index"]["dma_transfer"].append(file_path)

    # Interrupt & Exception Handling
    irq_pattern = (
        r"\b(interrupt|IRQ|NVIC|priority|handler|callback|" r"exception)\b"
    )
    if re.search(irq_pattern, content, re.IGNORECASE):
        index["concept_index"]["interrupt_handling"].append(file_path)

    # Power Management
    power_pattern = (
        r"\b(power|low.power|sleep|standby|stop|shutdown|" r"wake.up)\b"
    )
    if re.search(power_pattern, content, re.IGNORECASE):
        index["concept_index"]["power_management"].append(file_path)

    # Debug & Development
    debug_pattern = (
        r"\b(debug|trace|SWD|JTAG|breakpoint|watchpoint|ITM|" r"ETM)\b"
    )
    if re.search(debug_pattern, content, re.IGNORECASE):
        index["concept_index"]["debug_development"].append(file_path)

    # HAL & LL Library Usage
    hal_pattern = (
        r"\b(HAL|LL|driver|initialization|configuration|typedef|" r"handle)\b"
    )
    if re.search(hal_pattern, content, re.IGNORECASE):
        index["concept_index"]["hal_ll_usage"].append(file_path)

    # Real-time & Timing
    rt_pattern = (
        r"\b(real.time|deterministic|latency|jitter|timing|"
        r"synchronization)\b"
    )
    if re.search(rt_pattern, content, re.IGNORECASE):
        index["concept_index"]["realtime_timing"].append(file_path)

    # Control Systems
    ctrl_pattern = (
        r"\b(control|PID|feedback|loop|setpoint|plant|" r"controller)\b"
    )
    if re.search(ctrl_pattern, content, re.IGNORECASE):
        index["concept_index"]["control_systems"].append(file_path)

    # General keywords (technical terms) - Limited for efficiency
    keywords = re.findall(r"\b[A-Z][A-Z0-9_]{2,}\b", content)
    for keyword in keywords[:25]:  # Reduced from 50 to 25 for efficiency
        index["keyword_index"][keyword].append(file_path)


def extract_keywords_from_file(file_path):
    """Extract relevant keywords from a markdown file"""
    keywords = set()

    try:
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()
    except BaseException:
        return keywords

    # Extract function names (HAL_*, LL_*)
    hal_functions = re.findall(r"\b(HAL_[A-Z_][A-Za-z0-9_]*)\b", content)
    ll_functions = re.findall(r"\b(LL_[A-Z_][A-Za-z0-9_]*)\b", content)
    keywords.update(hal_functions + ll_functions)

    # Extract peripheral names
    peripherals = re.findall(
        r"\b(GPIO|UART|USART|SPI|I2C|ADC|DAC|TIM|DMA|DMAMUX|RCC|PWR|EXTI|NVIC|CRC|RNG|HASH|CRYP|LTDC|DSI|DCMI|ETH|USB|CAN|FDCAN|SAI|DFSDM|MDIOS|SDMMC|FMC|QUADSPI|OCTOSPI|HSEM|WWDG|IWDG|RTC|TAMP|LPTIM|LPUART|SWPMI|OPAMP|COMP|VREFBUF|CORDIC|FMAC)\b",  # noqa: E501
        content,
    )
    keywords.update(peripherals)

    # Extract structure names
    structs = re.findall(
        r"\b([A-Z][A-Za-z0-9_]*_(?:InitTypeDef|HandleTypeDef|TypeDef))\b",
        content,
    )
    keywords.update(structs)

    # Extract defines/constants
    defines = re.findall(r"#define\s+([A-Z_][A-Z0-9_]*)", content)
    keywords.update(defines)

    return keywords


def search_documentation(search_type, query, scope="all"):
    """Unified search across all documentation"""
    results = []

    if scope in ["all", "STM32H7"]:
        results.extend(search_stm32h7(search_type, query))

    if scope in ["all", "L6470"]:
        results.extend(search_l6470(search_type, query))

    return results


def search_stm32h7(search_type, query):
    """Search STM32H7 documentation"""
    if not STM32H7_INDEX_PATH.exists():
        print("❌ STM32H7 index not found. Run with --create-indexes first.")
        return []

    with open(STM32H7_INDEX_PATH, "r") as f:
        index = json.load(f)

    results = []

    if search_type == "peripheral":
        query = query.upper()
        if query in index["peripheral_index"]:
            files = index["peripheral_index"][query]
            for file in files[:10]:  # Limit results
                if file in index["file_metadata"]:
                    results.append(
                        {
                            "file": file,
                            "source": "STM32H7",
                            "type": "peripheral",
                            "size_kb": index["file_metadata"][file]["size_kb"],
                            "full_path": index["file_metadata"][file][
                                "full_path"
                            ],
                        }
                    )

    elif search_type == "function":
        if query in index["function_index"]:
            files = index["function_index"][query]
            for file in files[:10]:
                if file in index["file_metadata"]:
                    results.append(
                        {
                            "file": file,
                            "source": "STM32H7",
                            "type": "function",
                            "size_kb": index["file_metadata"][file]["size_kb"],
                            "full_path": index["file_metadata"][file][
                                "full_path"
                            ],
                        }
                    )

    elif search_type == "concept":
        query = query.lower()
        for concept, files in index["concept_index"].items():
            if query in concept:
                for file in files[:5]:  # Fewer results for concepts
                    if file in index["file_metadata"]:
                        results.append(
                            {
                                "file": file,
                                "source": "STM32H7",
                                "type": "concept",
                                "concept": concept,
                                "size_kb": index["file_metadata"][file][
                                    "size_kb"
                                ],
                                "full_path": index["file_metadata"][file][
                                    "full_path"
                                ],  # noqa: E501
                            }
                        )

    return results


def search_l6470(search_type, query):
    """Search L6470 documentation"""
    if not L6470_INDEX_PATH.exists():
        print("❌ L6470 index not found. Run with --create-indexes first.")
        return []

    with open(L6470_INDEX_PATH, "r") as f:
        index = json.load(f)

    results = []

    if search_type == "register":
        query = query.upper()
        if query in index["register_index"]:
            files = index["register_index"][query]
            for file in files[:10]:
                if file in index["file_metadata"]:
                    results.append(
                        {
                            "file": file,
                            "source": "L6470",
                            "type": "register",
                            "size_kb": index["file_metadata"][file]["size_kb"],
                            "full_path": index["file_metadata"][file][
                                "full_path"
                            ],
                        }
                    )

    elif search_type == "function":
        if query in index["function_index"]:
            files = index["function_index"][query]
            for file in files[:10]:
                if file in index["file_metadata"]:
                    results.append(
                        {
                            "file": file,
                            "source": "L6470",
                            "type": "function",
                            "size_kb": index["file_metadata"][file]["size_kb"],
                            "full_path": index["file_metadata"][file][
                                "full_path"
                            ],
                        }
                    )

    elif search_type == "command":
        if query in index["command_index"]:
            files = index["command_index"][query]
            for file in files[:10]:
                if file in index["file_metadata"]:
                    results.append(
                        {
                            "file": file,
                            "source": "L6470",
                            "type": "command",
                            "size_kb": index["file_metadata"][file]["size_kb"],
                            "full_path": index["file_metadata"][file][
                                "full_path"
                            ],
                        }
                    )

    return results


def display_results(results, query):
    """Display search results in a formatted way"""
    if not results:
        print(f"❌ No results found for '{query}'")
        return

    print(f"🔍 Found {len(results)} results for '{query}':")
    print("=" * 50)

    # Group by source
    stm32h7_results = [r for r in results if r["source"] == "STM32H7"]
    l6470_results = [r for r in results if r["source"] == "L6470"]

    if stm32h7_results:
        print(f"\n📘 STM32H7 Documentation ({len(stm32h7_results)} files):")
        for result in stm32h7_results:
            print(
                f"   • {result['file']} ({result['size_kb']}KB) - {result['type']}"
            )  # noqa: E501

    if l6470_results:
        print(f"\n📗 L6470 Documentation ({len(l6470_results)} files):")
        for result in l6470_results:
            print(
                f"   • {result['file']} ({result['size_kb']}KB) - {result['type']}"
            )  # noqa: E501


def main():
    """Main function with command-line interface"""
    parser = argparse.ArgumentParser(
        description="Documentation Search and Index Management Tool"
    )

    subparsers = parser.add_subparsers(
        dest="command", help="Available commands"
    )

    # Index management commands
    index_parser = subparsers.add_parser(
        "index", help="Manage documentation indexes"
    )
    index_parser.add_argument(
        "action", choices=["create", "recreate"], help="Index action"
    )
    index_parser.add_argument(
        "--scope",
        choices=["all", "STM32H7", "L6470"],
        default="all",
        help="Index scope",
    )

    # Search commands
    search_parser = subparsers.add_parser(
        "search", help="Search documentation"
    )
    search_parser.add_argument(
        "type",
        choices=["peripheral", "function", "register", "command", "concept"],
        help="Search type",
    )
    search_parser.add_argument("query", help="Search query")
    search_parser.add_argument(
        "--scope",
        choices=["all", "STM32H7", "L6470"],
        default="all",
        help="Search scope",
    )

    args = parser.parse_args()

    if args.command == "index":
        print("📚 Documentation Index Management")
        print("=" * 40)

        if args.scope in ["all", "STM32H7"]:
            create_unified_stm32h7_index()
            # Also create Copilot-optimized index
            create_copilot_optimized_index()

        if args.scope in ["all", "L6470"]:
            create_l6470_index()

    elif args.command == "search":
        print("🔍 Documentation Search")
        print("=" * 40)

        results = search_documentation(args.type, args.query, args.scope)
        display_results(results, args.query)

    else:
        parser.print_help()


if __name__ == "__main__":
    main()
