# GPU Acceleration Setup for STM32 Semantic Search

## Overview
This guide explains how to enable GPU acceleration for Ollama in the STM32H753ZI dev container to achieve maximum performance for semantic search operations.

## Current Status
- ✅ **Dev Container**: CUDA 12.1 base image with GPU support configured
- ✅ **Ollama**: Installed with automatic GPU detection
- ✅ **Model**: mxbai-embed-large preloaded on container start
- ✅ **Wrapper Script**: `./scripts/stm32_search.sh` handles virtual environment automatically

## Host Prerequisites

### 1. NVIDIA GPU and Drivers
Your host machine needs:
- NVIDIA GPU with CUDA support (GTX 1060 or newer recommended)
- NVIDIA drivers version 535+ installed
- Minimum 4GB GPU memory for mxbai-embed-large model

### 2. NVIDIA Container Toolkit
Install on your host system:

**Ubuntu/Debian:**
```bash
# Add NVIDIA package repository
distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | sudo apt-key add -
curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | sudo tee /etc/apt/sources.list.d/nvidia-docker.list

# Install NVIDIA Container Toolkit
sudo apt-get update && sudo apt-get install -y nvidia-container-toolkit

# Restart Docker
sudo systemctl restart docker
```

**Verify Installation:**
```bash
# Test GPU access in Docker
docker run --rm --gpus all nvidia/cuda:12.1-base-ubuntu22.04 nvidia-smi
```

### 3. Docker GPU Configuration
Add to your Docker daemon configuration (`/etc/docker/daemon.json`):
```json
{
    "default-runtime": "nvidia",
    "runtimes": {
        "nvidia": {
            "path": "nvidia-container-runtime",
            "runtimeArgs": []
        }
    }
}
```

## Dev Container Configuration

### GPU Support Added
The dev container now includes:

**Dockerfile Updates:**
- NVIDIA CUDA 12.1 base image
- NVIDIA drivers and container toolkit
- Ollama with GPU support
- CUDA environment variables

**devcontainer.json Updates:**
- GPU passthrough with `--gpus=all`
- NVIDIA device access
- Automatic Ollama setup script

### Automatic Setup
On container start, the system automatically:
1. Starts Ollama service with GPU detection
2. Pulls mxbai-embed-large model if needed
3. Preloads model for optimal performance
4. Reports GPU acceleration status

## Usage

### Recommended: Wrapper Script
```bash
# Automatically handles virtual environment and dependencies
./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7
```

### Direct Usage
```bash
# Manual virtual environment usage
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "GPIO" --scope STM32H7
```

## Performance Monitoring

### Check GPU Acceleration
```bash
# Ollama model status - should show GPU processor
ollama ps

# NVIDIA GPU utilization during search
nvidia-smi --loop=1

# Test search performance
time ./scripts/stm32_search.sh concept "stepper motor control" --scope all
```

### Expected Performance
- **CPU Only**: ~2-5 seconds per search with mock embeddings
- **GPU Accelerated**: ~0.5-1 second per search with real embeddings
- **Model Loading**: ~10-30 seconds initial load (cached afterward)

## Troubleshooting

### GPU Not Detected
If `ollama ps` shows "100% CPU" instead of GPU:

1. **Check Host GPU:**
   ```bash
   nvidia-smi  # On host machine
   ```

2. **Verify Container GPU Access:**
   ```bash
   # Inside dev container
   nvidia-smi  # Should work if properly configured
   ```

3. **Restart Ollama with GPU:**
   ```bash
   pkill ollama
   CUDA_VISIBLE_DEVICES=all ollama serve &
   ollama pull mxbai-embed-large
   ```

### Container Rebuild Required
After updating GPU configuration, rebuild the dev container:
1. **VS Code**: Command Palette → "Dev Containers: Rebuild Container"
2. **CLI**: `docker system prune && code .`

### Performance Issues
- **Insufficient GPU Memory**: Use smaller model or reduce batch size
- **Driver Conflicts**: Ensure host NVIDIA drivers are up to date
- **Docker Issues**: Restart Docker daemon after NVIDIA toolkit installation

## Fallback Mode
The system gracefully falls back to CPU/mock embeddings if:
- No GPU available
- CUDA drivers missing
- Ollama service unavailable

Mock embeddings still provide functional semantic search for development.

## Future Enhancements
- **Multi-GPU Support**: Scale across multiple GPUs for larger models
- **Model Optimization**: Quantized models for faster inference
- **Batch Processing**: Bulk document embedding for faster database rebuilds

## GPU Resource Usage
- **Model Size**: 669 MB (mxbai-embed-large)
- **Runtime Memory**: ~1-2 GB GPU RAM during inference
- **CPU Fallback**: Available if GPU resources unavailable

**Status**: GPU acceleration configured and ready for container rebuild.
