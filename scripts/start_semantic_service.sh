#!/bin/bash
# STM32 Semantic Search Service Startup Script
# Starts the semantic search service as a background daemon

set -e

SERVICE_NAME="STM32 Semantic Search Service"
SERVICE_SCRIPT="scripts/semantic_search_service.py"
SERVICE_PORT=8080
SERVICE_HOST="127.0.0.1"
PID_FILE="/tmp/semantic_search_service.pid"
LOG_FILE="/tmp/semantic_search_service.log"
WORKSPACE_ROOT="/workspaces/code"

echo "🚀 Starting $SERVICE_NAME..."

# Change to workspace directory
cd "$WORKSPACE_ROOT"

# Activate virtual environment
source .venv/bin/activate

# Check if service is already running
if [ -f "$PID_FILE" ]; then
    OLD_PID=$(cat "$PID_FILE")
    if ps -p "$OLD_PID" > /dev/null 2>&1; then
        echo "⚠️  Service already running (PID: $OLD_PID)"
        echo "   Use 'scripts/stop_semantic_service.sh' to stop it first"
        exit 1
    else
        echo "🧹 Cleaning up stale PID file"
        rm -f "$PID_FILE"
    fi
fi

# Start service in background
echo "🔧 Starting service on $SERVICE_HOST:$SERVICE_PORT"
echo "📋 Log file: $LOG_FILE"

nohup python "$SERVICE_SCRIPT" --mode service --host "$SERVICE_HOST" --port "$SERVICE_PORT" > "$LOG_FILE" 2>&1 &
SERVICE_PID=$!

# Save PID
echo "$SERVICE_PID" > "$PID_FILE"

# Wait a moment for startup
sleep 3

# Verify service is running
if ps -p "$SERVICE_PID" > /dev/null 2>&1; then
    echo "✅ Service started successfully (PID: $SERVICE_PID)"
    
    # Test health endpoint
    echo "🧪 Testing service health..."
    if curl -s "http://$SERVICE_HOST:$SERVICE_PORT/health" > /dev/null; then
        echo "✅ Service is responding to requests"
        echo ""
        echo "🎯 $SERVICE_NAME is ready!"
        echo "   Status: python scripts/stm32_semantic_search.py status"
        echo "   Search: python scripts/stm32_semantic_search.py concept 'GPIO' --scope STM32H7"
        echo "   Stop:   scripts/stop_semantic_service.sh"
        echo "   Logs:   tail -f $LOG_FILE"
    else
        echo "⚠️  Service started but not responding yet. Check logs:"
        echo "   tail -f $LOG_FILE"
    fi
else
    echo "❌ Failed to start service"
    rm -f "$PID_FILE"
    exit 1
fi
