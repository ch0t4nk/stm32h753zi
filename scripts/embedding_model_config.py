#!/usr/bin/env python3
"""
SSOT-Based Embedding Model Configuration System
Dynamic model discovery and selection for STM32H753ZI semantic search optimization
"""

import json
import requests
import subprocess
import os
from pathlib import Path
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, asdict
from datetime import datetime

try:
    import torch
    TORCH_AVAILABLE = True
except ImportError:
    TORCH_AVAILABLE = False
    torch = None

try:
    from InstructorEmbedding import INSTRUCTOR
    INSTRUCTOR_AVAILABLE = True
except ImportError:
    INSTRUCTOR_AVAILABLE = False

WORKSPACE_ROOT = Path("/workspaces/code")
CONFIG_FILE = WORKSPACE_ROOT / "src/config/embedding_model_config.json"

@dataclass
class HardwareCapabilities:
    """Hardware capabilities for embedding model selection"""
    gpu_available: bool
    gpu_name: Optional[str]
    gpu_memory_gb: Optional[float]
    cpu_cores: int
    total_memory_gb: float
    cuda_version: Optional[str]

@dataclass
class ModelPerformance:
    """Performance characteristics of an embedding model"""
    avg_response_time_ms: float
    memory_usage_mb: float
    relevance_score: float
    batch_size_optimal: int
    quality_tier: str  # "excellent", "good", "minimum"
    speed_tier: str    # "fast", "moderate", "slow"

@dataclass
class EmbeddingModelConfig:
    """Complete configuration for an embedding model"""
    name: str
    type: str  # "instructor", "ollama", "mock"
    model_id: str
    dimensions: int
    device: str
    available: bool
    description: str
    instruction_template: Optional[str] = None
    performance: Optional[ModelPerformance] = None
    domain_optimizations: Optional[Dict[str, Any]] = None

class EmbeddingConfigManager:
    """SSOT-based embedding model configuration manager"""
    
    def __init__(self):
        self.hardware = self._detect_hardware()
        self.available_models = {}
        self.config_data = {}
        
        # Load existing configuration if available
        self._load_config()
        
        # Discover and update model availability
        self._discover_models()
        
    def _detect_hardware(self) -> HardwareCapabilities:
        """Detect current hardware capabilities"""
        print("🔍 Detecting hardware capabilities...")
        
        gpu_available = TORCH_AVAILABLE and torch is not None and torch.cuda.is_available()
        gpu_name = None
        gpu_memory_gb = None
        cuda_version = None
        
        if gpu_available and torch is not None:
            gpu_name = torch.cuda.get_device_name(0)
            gpu_memory_gb = torch.cuda.get_device_properties(0).total_memory / 1024**3
            # CUDA version detection is tricky due to torch version differences
            cuda_version = "detected"  # Simplified - we know CUDA is available if we get here
        
        # Get CPU and memory info
        cpu_cores = os.cpu_count() or 4
        
        try:
            # Try to get memory info
            with open('/proc/meminfo', 'r') as f:
                meminfo = f.read()
            total_memory_kb = int([line for line in meminfo.split('\n') if 'MemTotal' in line][0].split()[1])
            total_memory_gb = total_memory_kb / 1024 / 1024
        except:
            total_memory_gb = 8.0  # Fallback estimate
        
        hardware = HardwareCapabilities(
            gpu_available=gpu_available,
            gpu_name=gpu_name,
            gpu_memory_gb=gpu_memory_gb,
            cpu_cores=cpu_cores,
            total_memory_gb=total_memory_gb,
            cuda_version=cuda_version
        )
        
        print(f"✅ Hardware detected:")
        print(f"   GPU: {gpu_name if gpu_name else 'None'}")
        print(f"   Memory: {total_memory_gb:.1f}GB")
        print(f"   CPU Cores: {cpu_cores}")
        
        return hardware
    
    def _discover_ollama_models(self) -> List[Dict[str, Any]]:
        """Discover available Ollama models"""
        try:
            result = subprocess.run(['ollama', 'list'], capture_output=True, text=True, timeout=10)
            if result.returncode != 0:
                print("⚠️  Ollama not available or not running")
                return []
            
            lines = result.stdout.strip().split('\n')[1:]  # Skip header
            models = []
            
            for line in lines:
                if line.strip():
                    parts = line.split()
                    if len(parts) >= 3:
                        name = parts[0]
                        model_id = parts[1]
                        size = parts[2]
                        
                        # Extract size in MB
                        size_mb = 0
                        if 'MB' in size:
                            size_mb = float(size.replace('MB', '').strip())
                        elif 'GB' in size:
                            size_mb = float(size.replace('GB', '').strip()) * 1024
                        
                        models.append({
                            'name': name,
                            'model_id': model_id,
                            'size_mb': size_mb,
                            'available': True
                        })
            
            print(f"✅ Found {len(models)} Ollama models")
            return models
            
        except subprocess.TimeoutExpired:
            print("⚠️  Ollama list command timed out")
            return []
        except Exception as e:
            print(f"⚠️  Error discovering Ollama models: {e}")
            return []
    
    def _discover_instructor_models(self) -> List[Dict[str, Any]]:
        """Discover available Instructor models"""
        models = []
        
        if not INSTRUCTOR_AVAILABLE:
            print("⚠️  InstructorEmbedding not available")
            return models
        
        # Check for cached Instructor-XL model
        cache_dir = Path.home() / ".cache" / "huggingface" / "hub"
        instructor_xl_cache = cache_dir / "models--hkunlp--instructor-xl"
        
        if instructor_xl_cache.exists():
            models.append({
                'name': 'instructor-xl',
                'model_id': 'hkunlp/instructor-xl',
                'size_mb': 1340,  # Approximate size
                'available': True,
                'gpu_capable': self.hardware.gpu_available,
                'cpu_capable': True
            })
            print("✅ Found cached Instructor-XL model")
        else:
            print("⚠️  Instructor-XL model not cached (will download on first use)")
            models.append({
                'name': 'instructor-xl',
                'model_id': 'hkunlp/instructor-xl', 
                'size_mb': 1340,
                'available': False,  # Will be downloaded on first use
                'gpu_capable': self.hardware.gpu_available,
                'cpu_capable': True
            })
        
        return models
    
    def _discover_models(self):
        """Discover all available embedding models"""
        print("🔄 Discovering available embedding models...")
        
        # Discover Ollama models
        ollama_models = self._discover_ollama_models()
        
        # Discover Instructor models
        instructor_models = self._discover_instructor_models()
        
        # Create model configurations
        self.available_models = {}
        
        # Configure Instructor models
        for model in instructor_models:
            if model['name'] == 'instructor-xl':
                # GPU version
                if model['gpu_capable']:
                    self.available_models['instructor_xl_gpu'] = EmbeddingModelConfig(
                        name="Instructor-XL (GPU)",
                        type="instructor",
                        model_id=model['model_id'],
                        dimensions=768,
                        device="cuda",
                        available=model['available'],
                        description="High-quality instruction-tuned embeddings with GPU acceleration",
                        instruction_template="Represent the {domain} document for finding {task}:",
                        domain_optimizations={
                            "stm32_hal": {"batch_size": 25, "instruction": "Represent the STM32 HAL documentation for finding peripheral configuration and driver functions:"},
                            "motor_control": {"batch_size": 30, "instruction": "Represent the stepper motor control documentation for finding L6470 driver functions:"},
                            "project_source": {"batch_size": 20, "instruction": "Represent the embedded C source code for finding HAL abstraction functions:"}
                        }
                    )
                
                # CPU version
                self.available_models['instructor_xl_cpu'] = EmbeddingModelConfig(
                    name="Instructor-XL (CPU)",
                    type="instructor",
                    model_id=model['model_id'],
                    dimensions=768,
                    device="cpu",
                    available=model['available'],
                    description="High-quality instruction-tuned embeddings on CPU",
                    instruction_template="Represent the {domain} document for finding {task}:",
                    domain_optimizations={
                        "stm32_hal": {"batch_size": 8, "instruction": "Represent the STM32 HAL documentation for finding peripheral configuration:"},
                        "motor_control": {"batch_size": 10, "instruction": "Represent the stepper motor control documentation for finding L6470 functions:"},
                        "project_source": {"batch_size": 6, "instruction": "Represent the embedded C source code for finding functions:"}
                    }
                )
        
        # Configure Ollama models
        for model in ollama_models:
            if 'mxbai-embed-large' in model['name']:
                self.available_models['mxbai_embed_large'] = EmbeddingModelConfig(
                    name="mxbai-embed-large",
                    type="ollama", 
                    model_id="mxbai-embed-large",
                    dimensions=1024,
                    device="ollama",
                    available=True,
                    description="Ollama-hosted embedding model optimized for retrieval",
                    domain_optimizations={
                        "stm32_hal": {"batch_size": 15},
                        "motor_control": {"batch_size": 18},
                        "project_source": {"batch_size": 12}
                    }
                )
                
            elif 'nomic-embed-text' in model['name']:
                self.available_models['nomic_embed_text'] = EmbeddingModelConfig(
                    name="nomic-embed-text",
                    type="ollama",
                    model_id="nomic-embed-text",
                    dimensions=768,
                    device="ollama", 
                    available=True,
                    description="Smaller Ollama embedding model for fast inference",
                    domain_optimizations={
                        "stm32_hal": {"batch_size": 20},
                        "motor_control": {"batch_size": 25},
                        "project_source": {"batch_size": 15}
                    }
                )
        
        print(f"📊 Configured {len(self.available_models)} embedding models")
        for name, config in self.available_models.items():
            status = "✅" if config.available else "⏳"
            print(f"   {status} {config.name} ({config.type}, {config.dimensions}D)")
    
    def select_best_model(self, 
                         optimize_for: str = "quality",  # "quality", "speed", "memory"
                         domain: str = "general",
                         require_gpu: bool = False,
                         allow_fallback: bool = True) -> Optional[str]:
        """
        Select the best embedding model based on requirements
        
        Args:
            optimize_for: Optimization priority ("quality", "speed", "memory")
            domain: Target domain for optimization
            require_gpu: Whether GPU is required
            allow_fallback: Whether to allow fallback to less optimal models
            
        Returns:
            Model key for best match, or None if no suitable model found
        """
        available_configs = {k: v for k, v in self.available_models.items() if v.available}
        
        if not available_configs:
            print("❌ No available embedding models found")
            return None
        
        # Filter by GPU requirement
        if require_gpu:
            available_configs = {k: v for k, v in available_configs.items() 
                               if "gpu" in k.lower() or v.device == "cuda"}
        
        if not available_configs:
            if allow_fallback:
                print("⚠️  No GPU models available, falling back to CPU/Ollama models")
                available_configs = {k: v for k, v in self.available_models.items() if v.available}
            else:
                print("❌ No GPU models available and fallback disabled")
                return None
        
        # Score models based on optimization criteria
        model_scores = {}
        
        for key, config in available_configs.items():
            score = 0.0
            
            if optimize_for == "quality":
                # Prefer Instructor models for quality
                if config.type == "instructor":
                    score += 10.0
                    if "gpu" in key:
                        score += 5.0  # GPU boost for quality
                elif config.type == "ollama":
                    if "mxbai" in key:
                        score += 7.0  # mxbai is good quality
                    else:
                        score += 5.0
                        
            elif optimize_for == "speed":
                # Prefer GPU models and smaller models for speed
                if "gpu" in key:
                    score += 8.0
                elif config.type == "ollama":
                    score += 6.0  # Ollama can be fast
                    if "nomic" in key:
                        score += 2.0  # Smaller model
                        
            elif optimize_for == "memory":
                # Prefer smaller models
                if config.dimensions <= 768:
                    score += 8.0
                else:
                    score += 5.0
                    
                if config.type == "ollama":
                    score += 3.0  # Ollama uses less system memory
            
            # Domain-specific bonuses
            if config.domain_optimizations and domain in config.domain_optimizations:
                score += 2.0
            
            model_scores[key] = score
        
        # Select highest scoring model
        best_model = max(model_scores.items(), key=lambda x: x[1])
        
        print(f"🎯 Selected model: {self.available_models[best_model[0]].name}")
        print(f"   Criteria: optimize_for={optimize_for}, domain={domain}")
        print(f"   Score: {best_model[1]:.1f}")
        
        return best_model[0]
    
    def get_model_config(self, model_key: str) -> Optional[EmbeddingModelConfig]:
        """Get configuration for a specific model"""
        return self.available_models.get(model_key)
    
    def update_model_performance(self, model_key: str, performance: ModelPerformance):
        """Update performance data for a model"""
        if model_key in self.available_models:
            self.available_models[model_key].performance = performance
            self._save_config()
            print(f"✅ Updated performance data for {self.available_models[model_key].name}")
    
    def _load_config(self):
        """Load existing configuration from file"""
        if CONFIG_FILE.exists():
            try:
                with open(CONFIG_FILE, 'r') as f:
                    self.config_data = json.load(f)
                print(f"✅ Loaded existing configuration from {CONFIG_FILE}")
            except Exception as e:
                print(f"⚠️  Error loading config: {e}")
                self.config_data = {}
        else:
            self.config_data = {}
    
    def _save_config(self):
        """Save current configuration to file"""
        config_data = {
            "last_updated": datetime.now().isoformat(),
            "hardware": asdict(self.hardware),
            "models": {
                key: {
                    "name": config.name,
                    "type": config.type,
                    "model_id": config.model_id,
                    "dimensions": config.dimensions,
                    "device": config.device,
                    "available": config.available,
                    "description": config.description,
                    "instruction_template": config.instruction_template,
                    "performance": asdict(config.performance) if config.performance else None,
                    "domain_optimizations": config.domain_optimizations
                }
                for key, config in self.available_models.items()
            }
        }
        
        # Ensure directory exists
        CONFIG_FILE.parent.mkdir(parents=True, exist_ok=True)
        
        with open(CONFIG_FILE, 'w') as f:
            json.dump(config_data, f, indent=2)
        
        print(f"💾 Saved configuration to {CONFIG_FILE}")
    
    def generate_usage_script(self) -> str:
        """Generate a usage script for the current configuration"""
        script_content = f"""#!/usr/bin/env python3
\"\"\"
Auto-generated embedding model configuration script
Generated: {datetime.now().isoformat()}
\"\"\"

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
print(f"Best quality model: {{best_quality_model}}")

# Example 2: Select fastest model (production use)
fastest_model = config_manager.select_best_model(
    optimize_for="speed",
    domain="motor_control",
    allow_fallback=True
)
print(f"Fastest model: {{fastest_model}}")

# Example 3: Get model configuration
if best_quality_model:
    config = config_manager.get_model_config(best_quality_model)
    print(f"Model config: {{config.name}} - {{config.dimensions}}D")
    
    # Get domain-specific optimization
    if config.domain_optimizations:
        hal_config = config.domain_optimizations.get('stm32_hal', {{}})
        print(f"STM32 HAL batch size: {{hal_config.get('batch_size', 10)}}")

# Available models:
"""
        
        for key, config in self.available_models.items():
            status = "Available" if config.available else "Not Available"
            script_content += f"# - {config.name}: {status} ({config.type}, {config.dimensions}D)\n"
        
        return script_content
    
    def run_discovery_update(self):
        """Run a complete discovery and configuration update"""
        print("🔄 Running complete model discovery and configuration update...")
        
        self._discover_models()
        self._save_config()
        
        # Generate usage script
        usage_script = self.generate_usage_script()
        usage_script_path = WORKSPACE_ROOT / "scripts" / "embedding_model_usage.py"
        
        with open(usage_script_path, 'w') as f:
            f.write(usage_script)
        
        print(f"📄 Generated usage script: {usage_script_path}")
        print("🎉 Model discovery and configuration update complete!")

if __name__ == "__main__":
    config_manager = EmbeddingConfigManager()
    config_manager.run_discovery_update()
