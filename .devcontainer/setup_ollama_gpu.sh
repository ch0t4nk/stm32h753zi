#!/bin/bash
# Ollama GPU Setup Script for STM32H753ZI Dev Container

echo "🚀 Starting Ollama with GPU acceleration..."

# Start Ollama service in background
ollama serve &
OLLAMA_PID=$!

# Wait for Ollama to be ready
echo "⏳ Waiting for Ollama service to start..."
sleep 5

# Check if service is running
if ! curl -s http://localhost:11434/api/tags > /dev/null; then
    echo "❌ Ollama service failed to start"
    exit 1
fi

echo "✅ Ollama service started successfully"

# Pull the embedding model if not already available
echo "📦 Ensuring mxbai-embed-large model is available..."
if ! ollama list | grep -q "mxbai-embed-large"; then
    echo "📥 Pulling mxbai-embed-large model..."
    ollama pull mxbai-embed-large
else
    echo "✅ mxbai-embed-large model already available"
fi

# Preload the model for faster access
echo "🔄 Preloading model for optimal performance..."
ollama run mxbai-embed-large ""

# Check GPU usage
echo "🔍 Checking GPU acceleration status..."
ollama ps

# Check NVIDIA GPU status if available
if command -v nvidia-smi &> /dev/null; then
    echo "🖥️  GPU Status:"
    nvidia-smi --query-gpu=name,driver_version,memory.total,memory.used --format=csv,noheader,nounits
else
    echo "⚠️  nvidia-smi not available - GPU status unknown"
fi

echo "🎉 Ollama setup complete!"
echo ""
echo "💡 Usage examples:"
echo "   /workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept \"GPIO configuration\" --scope STM32H7"
echo "   /workspaces/code/.venv/bin/python scripts/test_semantic_search_quick.py"
echo ""
