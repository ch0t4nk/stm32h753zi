#!/usr/bin/env python3
"""
Auto-generated embedding model configuration script
Generated: 2025-08-07T10:40:05.330256
"""

# SSOT-based model selection examples:

from embedding_model_config import EmbeddingConfigManager

# Initialize configuration manager
config_manager = EmbeddingConfigManager()

# Example 1: Select best model for quality (development use)
best_quality_model = config_manager.select_best_model(
    optimize_for="quality", 
    domain="stm32_hal", 
    allow_fallback=True
)
print(f"Best quality model: {best_quality_model}")

# Example 2: Select fastest model (production use)
fastest_model = config_manager.select_best_model(
    optimize_for="speed",
    domain="motor_control",
    allow_fallback=True
)
print(f"Fastest model: {fastest_model}")

# Example 3: Get model configuration
if best_quality_model:
    config = config_manager.get_model_config(best_quality_model)
    print(f"Model config: {config.name} - {config.dimensions}D")
    
    # Get domain-specific optimization
    if config.domain_optimizations:
        hal_config = config.domain_optimizations.get('stm32_hal', {})
        print(f"STM32 HAL batch size: {hal_config.get('batch_size', 10)}")

# Available models:
# - Instructor-XL (GPU): Available (instructor, 768D)
# - Instructor-XL (CPU): Available (instructor, 768D)
# - nomic-embed-text: Available (ollama, 768D)
# - mxbai-embed-large: Available (ollama, 1024D)
