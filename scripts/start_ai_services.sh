#!/bin/bash
# Ollama Startup Script for STM32H753ZI Development Environment
# Ensures Ollama service runs persistently with GPU acceleration

set -e

echo "🚀 Starting Ollama service for STM32H753ZI development..."
echo "📊 GPU: RTX 4080 SUPER (14.7 GiB available)"
echo "🧠 Models: mxbai-embed-large (669MB), nomic-embed-text (274MB)"

# Check if Ollama is already running
if pgrep -f "ollama serve" > /dev/null; then
    echo "✅ Ollama is already running (PID: $(pgrep -f 'ollama serve'))"
    echo "🔗 Available at: http://127.0.0.1:11434"
    exit 0
fi

# Check if Ollama is installed
if ! command -v ollama &> /dev/null; then
    echo "❌ Ollama not found. Please install Ollama first."
    exit 1
fi

# Start Ollama service
echo "🔧 Starting Ollama service..."
echo "   Host: 127.0.0.1:11434"
echo "   GPU: CUDA with RTX 4080 SUPER"
echo "   Context: STM32H753ZI semantic search"

# Start Ollama in background with proper logging
nohup ollama serve > /tmp/ollama.log 2>&1 &
OLLAMA_PID=$!

# Wait for service to start
echo "⏳ Waiting for Ollama to initialize..."
sleep 3

# Verify service is running
if pgrep -f "ollama serve" > /dev/null; then
    echo "✅ Ollama started successfully (PID: $OLLAMA_PID)"
    echo "📋 Log file: /tmp/ollama.log"
    
    # Test connection
    echo "🧪 Testing connection..."
    sleep 2
    if curl -s http://127.0.0.1:11434/api/tags > /dev/null; then
        echo "✅ Ollama API is accessible"
        
        # Show available models
        echo "📚 Available embedding models:"
        ollama list | grep -E "(mxbai-embed|nomic-embed)" || echo "   (No embedding models found - please pull mxbai-embed-large)"
        
        echo ""
        echo "🎯 Ollama is ready for semantic search!"
        echo "   Usage: python scripts/stm32_semantic_search.py concept 'GPIO configuration' --scope STM32H7"
    else
        echo "⚠️  Ollama started but API not responding yet. Give it a moment..."
    fi
else
    echo "❌ Failed to start Ollama service"
    exit 1
fi

echo "📝 To stop Ollama: pkill -f 'ollama serve'"
echo "📊 To monitor: tail -f /tmp/ollama.log"
