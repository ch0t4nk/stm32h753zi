#!/usr/bin/env python3
"""
STM32-aware document chunking for semantic search
Implements smart chunking strategies for different document types
"""

import re
import json
from typing import List, Dict, Any, Tuple
from pathlib import Path
from dataclasses import dataclass
from enum import Enum


class ChunkType(Enum):
    HAL_FUNCTION = "HAL_FUNCTION"
    REGISTER_MAP = "REGISTER_MAP"
    CODE_EXAMPLE = "CODE_EXAMPLE"
    SAFETY_GUIDE = "SAFETY_GUIDE"
    CONFIG_SEQUENCE = "CONFIG_SEQUENCE"
    GENERAL = "GENERAL"


@dataclass
class ChunkConfig:
    target_size: int
    overlap: int
    preserve_blocks: List[str]


@dataclass
class DocumentChunk:
    content: str
    chunk_type: ChunkType
    metadata: Dict[str, Any]
    start_position: int
    end_position: int
    chunk_index: int


class STM32DocumentChunker:
    """Smart chunking for STM32 documentation"""

    def __init__(self):
        self.chunk_configs = {
            ChunkType.HAL_FUNCTION: ChunkConfig(
                target_size=1024,
                overlap=128,
                preserve_blocks=["signature", "parameters", "examples"],
            ),
            ChunkType.REGISTER_MAP: ChunkConfig(
                target_size=768,
                overlap=64,
                preserve_blocks=["bit_fields", "reset_values", "timing"],
            ),
            ChunkType.CODE_EXAMPLE: ChunkConfig(
                target_size=512,
                overlap=64,
                preserve_blocks=[
                    "init_sequence",
                    "error_handling",
                    "comments",
                ],
            ),
            ChunkType.SAFETY_GUIDE: ChunkConfig(
                target_size=1024,
                overlap=128,
                preserve_blocks=["requirements", "procedures", "validation"],
            ),
            ChunkType.CONFIG_SEQUENCE: ChunkConfig(
                target_size=768,
                overlap=96,
                preserve_blocks=["steps", "parameters", "verification"],
            ),
            ChunkType.GENERAL: ChunkConfig(
                target_size=512, overlap=64, preserve_blocks=[]
            ),
        }

        # Patterns for document type detection
        self.hal_function_pattern = re.compile(
            r"(?:HAL_\w+|BSP_\w+)\s*\([^)]*\)", re.MULTILINE
        )
        self.register_pattern = re.compile(
            r"(?:Register|Bit|Field).*?(?:0x[0-9A-Fa-f]+|\d+)", re.MULTILINE
        )
        self.code_pattern = re.compile(r"```[c|cpp]?\s*\n.*?\n```", re.DOTALL)
        self.safety_pattern = re.compile(
            r"(?:safety|fault|error|emergency|critical)", re.IGNORECASE
        )

    def classify_document_type(
        self, content: str, file_path: str = ""
    ) -> ChunkType:
        """Classify document type based on content and file path"""

        # Check file path for hints
        path_lower = file_path.lower()
        if "safety" in path_lower or "fault" in path_lower:
            return ChunkType.SAFETY_GUIDE

        # Count pattern matches
        hal_matches = len(self.hal_function_pattern.findall(content))
        register_matches = len(self.register_pattern.findall(content))
        code_matches = len(self.code_pattern.findall(content))
        safety_matches = len(self.safety_pattern.findall(content))

        # Classify based on dominant pattern
        if hal_matches > 5:
            return ChunkType.HAL_FUNCTION
        elif register_matches > 3:
            return ChunkType.REGISTER_MAP
        elif code_matches > 2:
            return ChunkType.CODE_EXAMPLE
        elif safety_matches > 3:
            return ChunkType.SAFETY_GUIDE
        else:
            return ChunkType.GENERAL

    def chunk_hal_functions(
        self, content: str, config: ChunkConfig
    ) -> List[DocumentChunk]:
        """Chunk HAL function documentation preserving function blocks"""
        chunks = []

        # Find function boundaries
        function_matches = list(self.hal_function_pattern.finditer(content))

        if not function_matches:
            # No HAL functions found, use general chunking
            return self.chunk_general(content, config)

        current_pos = 0
        chunk_index = 0

        for i, match in enumerate(function_matches):
            # Determine chunk boundaries
            start_pos = max(0, match.start() - 200)  # Include context before

            if i < len(function_matches) - 1:
                next_match = function_matches[i + 1]
                end_pos = min(len(content), next_match.start() - 100)
            else:
                end_pos = len(content)

            # Extract chunk content
            chunk_content = content[start_pos:end_pos].strip()

            if len(chunk_content) > 50:  # Minimum chunk size
                # Extract function name for metadata
                func_name = self.extract_function_name(match.group())

                chunk = DocumentChunk(
                    content=chunk_content,
                    chunk_type=ChunkType.HAL_FUNCTION,
                    metadata={
                        "function_name": func_name,
                        "peripheral": self.extract_peripheral(func_name),
                        "category": self.classify_function_category(func_name),
                        "has_code_example": bool(
                            self.code_pattern.search(chunk_content)
                        ),
                    },
                    start_position=start_pos,
                    end_position=end_pos,
                    chunk_index=chunk_index,
                )
                chunks.append(chunk)
                chunk_index += 1

        return chunks

    def chunk_registers(
        self, content: str, config: ChunkConfig
    ) -> List[DocumentChunk]:
        """Chunk register documentation keeping bit fields together"""
        chunks = []

        # Split by register sections (heuristic approach)
        register_sections = re.split(
            r"\n(?=(?:Register|Bit.*Field).*?:)", content
        )

        chunk_index = 0
        current_pos = 0

        for section in register_sections:
            if len(section.strip()) < 50:
                continue

            # Ensure section doesn't exceed target size
            if len(section) > config.target_size:
                # Split large sections while preserving bit field groups
                subsections = self.split_register_section(section, config)
                for subsection in subsections:
                    chunk = DocumentChunk(
                        content=subsection,
                        chunk_type=ChunkType.REGISTER_MAP,
                        metadata={
                            "register_name": self.extract_register_name(
                                subsection
                            ),
                            "bit_fields": self.count_bit_fields(subsection),
                            "has_reset_values": "reset" in subsection.lower(),
                        },
                        start_position=current_pos,
                        end_position=current_pos + len(subsection),
                        chunk_index=chunk_index,
                    )
                    chunks.append(chunk)
                    chunk_index += 1
                    current_pos += len(subsection)
            else:
                chunk = DocumentChunk(
                    content=section,
                    chunk_type=ChunkType.REGISTER_MAP,
                    metadata={
                        "register_name": self.extract_register_name(section),
                        "bit_fields": self.count_bit_fields(section),
                        "has_reset_values": "reset" in section.lower(),
                    },
                    start_position=current_pos,
                    end_position=current_pos + len(section),
                    chunk_index=chunk_index,
                )
                chunks.append(chunk)
                chunk_index += 1
                current_pos += len(section)

        return chunks

    def chunk_code_blocks(
        self, content: str, config: ChunkConfig
    ) -> List[DocumentChunk]:
        """Chunk code examples preserving initialization sequences"""
        chunks = []

        # Find code blocks
        code_matches = list(self.code_pattern.finditer(content))

        if not code_matches:
            return self.chunk_general(content, config)

        chunk_index = 0
        last_end = 0

        for match in code_matches:
            # Include context before and after code block
            context_start = max(last_end, match.start() - 300)
            context_end = min(len(content), match.end() + 200)

            chunk_content = content[context_start:context_end]

            chunk = DocumentChunk(
                content=chunk_content,
                chunk_type=ChunkType.CODE_EXAMPLE,
                metadata={
                    "code_language": self.detect_code_language(match.group()),
                    "has_initialization": self.has_init_sequence(
                        match.group()
                    ),
                    "has_error_handling": "error" in chunk_content.lower(),
                    "function_calls": self.extract_function_calls(
                        match.group()
                    ),
                },
                start_position=context_start,
                end_position=context_end,
                chunk_index=chunk_index,
            )
            chunks.append(chunk)
            chunk_index += 1
            last_end = context_end

        return chunks

    def chunk_general(
        self, content: str, config: ChunkConfig
    ) -> List[DocumentChunk]:
        """General chunking with overlap for unspecialized content"""
        chunks = []
        words = content.split()

        chunk_index = 0
        start_idx = 0

        while start_idx < len(words):
            # Calculate chunk boundaries
            end_idx = min(start_idx + config.target_size, len(words))

            # Find good breaking point (sentence end)
            if end_idx < len(words):
                for i in range(end_idx, max(start_idx, end_idx - 50), -1):
                    if words[i - 1].endswith((".", "!", "?", ":")):
                        end_idx = i
                        break

            chunk_words = words[start_idx:end_idx]
            chunk_content = " ".join(chunk_words)

            if len(chunk_content.strip()) > 50:
                chunk = DocumentChunk(
                    content=chunk_content,
                    chunk_type=ChunkType.GENERAL,
                    metadata={
                        "word_count": len(chunk_words),
                        "has_technical_terms": self.has_technical_terms(
                            chunk_content
                        ),
                        "stm32_references": self.count_stm32_references(
                            chunk_content
                        ),
                    },
                    start_position=start_idx,
                    end_position=end_idx,
                    chunk_index=chunk_index,
                )
                chunks.append(chunk)
                chunk_index += 1

            # Move to next chunk with overlap
            start_idx = max(start_idx + 1, end_idx - config.overlap)

        return chunks

    def chunk_document(
        self, content: str, file_path: str = "", force_type: ChunkType = None
    ) -> List[DocumentChunk]:
        """Main chunking interface"""

        # Determine document type
        doc_type = force_type or self.classify_document_type(
            content, file_path
        )
        config = self.chunk_configs[doc_type]

        # Apply appropriate chunking strategy
        if doc_type == ChunkType.HAL_FUNCTION:
            chunks = self.chunk_hal_functions(content, config)
        elif doc_type == ChunkType.REGISTER_MAP:
            chunks = self.chunk_registers(content, config)
        elif doc_type == ChunkType.CODE_EXAMPLE:
            chunks = self.chunk_code_blocks(content, config)
        else:
            chunks = self.chunk_general(content, config)

        # Add common metadata
        for chunk in chunks:
            chunk.metadata.update(
                {
                    "source_file": file_path,
                    "doc_type": doc_type.value,
                    "chunk_strategy": self.__class__.__name__,
                    "content_hash": hash(chunk.content),
                    "stm32_terms": self.extract_stm32_terms(chunk.content),
                }
            )

        return chunks

    # Helper methods
    def extract_function_name(self, function_signature: str) -> str:
        """Extract function name from signature"""
        match = re.search(r"(\w+)\s*\(", function_signature)
        return match.group(1) if match else "unknown"

    def extract_peripheral(self, function_name: str) -> str:
        """Extract peripheral type from function name"""
        if function_name.startswith("HAL_"):
            parts = function_name.split("_")
            return parts[1] if len(parts) > 1 else "unknown"
        return "unknown"

    def classify_function_category(self, function_name: str) -> str:
        """Classify function category"""
        if "Init" in function_name:
            return "initialization"
        elif "Config" in function_name:
            return "configuration"
        elif any(
            term in function_name
            for term in ["Start", "Stop", "Enable", "Disable"]
        ):
            return "control"
        elif any(term in function_name for term in ["Get", "Read", "Status"]):
            return "status"
        elif "Error" in function_name or "Fault" in function_name:
            return "error_handling"
        else:
            return "general"

    def extract_register_name(self, content: str) -> str:
        """Extract register name from content"""
        match = re.search(r"(\w+).*?(?:Register|Reg)", content)
        return match.group(1) if match else "unknown"

    def count_bit_fields(self, content: str) -> int:
        """Count bit field definitions in register content"""
        return len(re.findall(r"Bit\s+\d+", content))

    def split_register_section(
        self, section: str, config: ChunkConfig
    ) -> List[str]:
        """Split large register sections intelligently"""
        # Simple approach: split by bit field groups
        bit_field_splits = re.split(r"\n(?=Bit\s+\d+)", section)

        chunks = []
        current_chunk = ""

        for split in bit_field_splits:
            if len(current_chunk + split) > config.target_size:
                if current_chunk:
                    chunks.append(current_chunk)
                current_chunk = split
            else:
                current_chunk += "\n" + split if current_chunk else split

        if current_chunk:
            chunks.append(current_chunk)

        return chunks

    def detect_code_language(self, code_block: str) -> str:
        """Detect programming language in code block"""
        if "```c" in code_block:
            return "c"
        elif "```cpp" in code_block:
            return "cpp"
        elif any(term in code_block for term in ["HAL_", "GPIO_", "#include"]):
            return "c"
        else:
            return "unknown"

    def has_init_sequence(self, code_block: str) -> bool:
        """Check if code block contains initialization sequence"""
        init_keywords = ["Init", "Config", "Setup", "Enable"]
        return any(keyword in code_block for keyword in init_keywords)

    def extract_function_calls(self, code_block: str) -> List[str]:
        """Extract function calls from code block"""
        pattern = r"(\w+)\s*\("
        matches = re.findall(pattern, code_block)
        return list(set(matches))  # Remove duplicates

    def has_technical_terms(self, content: str) -> bool:
        """Check if content contains technical terms"""
        technical_terms = [
            "peripheral",
            "register",
            "configuration",
            "initialization",
            "interrupt",
            "DMA",
            "GPIO",
            "SPI",
            "UART",
            "I2C",
            "ADC",
        ]
        content_lower = content.lower()
        return any(term in content_lower for term in technical_terms)

    def count_stm32_references(self, content: str) -> int:
        """Count STM32-specific references"""
        stm32_pattern = re.compile(r"STM32|HAL_|BSP_|GPIO_|TIM_|SPI_|UART_")
        return len(stm32_pattern.findall(content))

    def extract_stm32_terms(self, content: str) -> List[str]:
        """Extract STM32-specific terms from content"""
        stm32_pattern = re.compile(
            r"\b(?:STM32\w*|HAL_\w+|BSP_\w+|GPIO_\w+)\b"
        )
        matches = stm32_pattern.findall(content)
        return list(set(matches))  # Remove duplicates


def main():
    """Test the chunking system"""
    chunker = STM32DocumentChunker()

    # Test with sample STM32 content
    sample_content = """
    # HAL_GPIO_Init Function
    
    HAL_GPIO_Init function initializes GPIO peripheral with specified configuration.
    
    ## Function Signature
    ```c
    HAL_StatusTypeDef HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
    ```
    
    ## Parameters
    - GPIOx: GPIO peripheral base address
    - GPIO_Init: Pointer to GPIO initialization structure
    
    ## Example Usage
    ```c
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    ```
    """

    chunks = chunker.chunk_document(sample_content, "test_hal_function.md")

    print(f"Generated {len(chunks)} chunks:")
    for i, chunk in enumerate(chunks):
        print(f"\nChunk {i + 1}:")
        print(f"  Type: {chunk.chunk_type}")
        print(f"  Size: {len(chunk.content)} chars")
        print(f"  Metadata: {chunk.metadata}")
        print(f"  Content preview: {chunk.content[:100]}...")


if __name__ == "__main__":
    main()
