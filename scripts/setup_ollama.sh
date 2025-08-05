#!/bin/bash
# Ollama Setup and Model Management Script
# Purpose: Ensure Ollama service is running with required models
# Usage: ./scripts/setup_ollama.sh

set -e

echo "🤖 Ollama Setup and Model Management"
echo "====================================="

# Check if Ollama is installed
if ! which ollama > /dev/null; then
    echo "❌ Ollama not found"
    echo "📋 Installing Ollama..."
    curl -fsSL https://ollama.ai/install.sh | sh
    echo "✅ Ollama installed"
fi

# Check if Ollama service is running
echo "🔍 Checking Ollama service status..."
if ! pgrep -f "ollama serve" > /dev/null; then
    echo "🚀 Starting Ollama service..."
    # Start Ollama in background
    nohup ollama serve > /tmp/ollama.log 2>&1 &
    sleep 5
    
    # Verify it started
    if pgrep -f "ollama serve" > /dev/null; then
        echo "✅ Ollama service started successfully"
    else
        echo "❌ Failed to start Ollama service"
        echo "📋 Check logs: /tmp/ollama.log"
        exit 1
    fi
else
    echo "✅ Ollama service already running"
fi

# Test API connection
echo "🔗 Testing Ollama API connection..."
RETRIES=0
MAX_RETRIES=10
while [ $RETRIES -lt $MAX_RETRIES ]; do
    if curl -s http://localhost:11434/api/tags > /dev/null; then
        echo "✅ Ollama API accessible"
        break
    else
        echo "⏳ Waiting for Ollama API... (attempt $((RETRIES + 1))/$MAX_RETRIES)"
        sleep 2
        RETRIES=$((RETRIES + 1))
    fi
done

if [ $RETRIES -eq $MAX_RETRIES ]; then
    echo "❌ Ollama API not accessible after $MAX_RETRIES attempts"
    echo "📋 Check if Ollama service is running: pgrep -f 'ollama serve'"
    exit 1
fi

# Check for required model
echo "📥 Checking for mxbai-embed-large model..."
if ! ollama list | grep -q "mxbai-embed-large"; then
    echo "⬇️  Downloading mxbai-embed-large model..."
    echo "    This may take several minutes depending on your connection..."
    
    # Download with progress
    ollama pull mxbai-embed-large
    
    if ollama list | grep -q "mxbai-embed-large"; then
        echo "✅ mxbai-embed-large model downloaded successfully"
    else
        echo "❌ Failed to download mxbai-embed-large model"
        exit 1
    fi
else
    echo "✅ mxbai-embed-large model already available"
fi

# Test model functionality
echo "🧪 Testing model functionality..."
if echo "test" | ollama run mxbai-embed-large > /dev/null 2>&1; then
    echo "✅ Model is working correctly"
else
    echo "⚠️  Model test failed - but model is available"
fi

# Show current models
echo "📋 Available models:"
ollama list

echo ""
echo "🎉 Ollama setup complete!"
echo "🔧 Service status: $(pgrep -f 'ollama serve' > /dev/null && echo 'Running' || echo 'Stopped')"
echo "🤖 Models available: $(ollama list | grep -c 'mxbai-embed-large' || echo '0') required models found"
echo ""
echo "💡 To use semantic search:"
echo "   ./scripts/stm32_search.sh concept \"GPIO configuration\" --scope STM32H7"
echo "   python scripts/stm32_semantic_search.py concept \"motor control\" --scope all"
