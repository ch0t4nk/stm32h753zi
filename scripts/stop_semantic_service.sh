#!/bin/bash
# STM32 Semantic Search Service Stop Script
# Stops the background semantic search service

set -e

SERVICE_NAME="STM32 Semantic Search Service"
PID_FILE="/tmp/semantic_search_service.pid"
LOG_FILE="/tmp/semantic_search_service.log"

echo "🛑 Stopping $SERVICE_NAME..."

if [ ! -f "$PID_FILE" ]; then
    echo "⚠️  PID file not found. Service may not be running."
    
    # Try to find and kill any running instances
    RUNNING_PIDS=$(pgrep -f "semantic_search_service.py" || true)
    if [ -n "$RUNNING_PIDS" ]; then
        echo "🔍 Found running service processes: $RUNNING_PIDS"
        echo "🧹 Cleaning up..."
        kill $RUNNING_PIDS
        sleep 2
        echo "✅ Service processes stopped"
    else
        echo "✅ No running service found"
    fi
    exit 0
fi

SERVICE_PID=$(cat "$PID_FILE")

if ps -p "$SERVICE_PID" > /dev/null 2>&1; then
    echo "🔄 Stopping service (PID: $SERVICE_PID)..."
    kill "$SERVICE_PID"
    
    # Wait for graceful shutdown
    sleep 3
    
    # Force kill if still running
    if ps -p "$SERVICE_PID" > /dev/null 2>&1; then
        echo "⚡ Force killing service..."
        kill -9 "$SERVICE_PID"
    fi
    
    echo "✅ Service stopped"
else
    echo "⚠️  Service not running (PID: $SERVICE_PID)"
fi

# Clean up PID file
rm -f "$PID_FILE"

echo "🧹 Cleanup complete"
echo "📋 Last 10 log entries:"
if [ -f "$LOG_FILE" ]; then
    tail -10 "$LOG_FILE"
else
    echo "   (No log file found)"
fi
