#!/bin/bash
#
# Heart Rate Monitor Quick Start Script
# 
# This script helps you get the heart rate monitor integration running quickly.
#

set -e

echo "================================================"
echo "Heart Rate Monitor - Solace Integration"
echo "================================================"
echo ""

# Check if we're in the right directory
if [ ! -f "pyproject.toml" ]; then
    echo "❌ Error: Please run this script from the project root directory"
    echo "   cd /Users/stevensu/solace-agent-mesh-hackathon-quickstart"
    exit 1
fi

# Check if .env exists
if [ ! -f ".env" ]; then
    echo "⚠️  Warning: .env file not found"
    echo "   Copying .env.example to .env..."
    cp .env.example .env
    echo "   ✅ Created .env file"
    echo ""
    echo "   ⚠️  IMPORTANT: Edit .env and add your LLM API key before running!"
    echo "   See docs/llm-setup.md for free API options"
    echo ""
fi

# Install dependencies
echo "📦 Installing dependencies..."
uv sync
echo "✅ Dependencies installed"
echo ""

# Test the integration
echo "🧪 Testing WebUI API connection..."
if uv run python test_integration.py; then
    echo ""
    echo "✅ All checks passed!"
    echo ""
    echo "================================================"
    echo "You're ready to start the heart rate monitor!"
    echo "================================================"
    echo ""
    echo "Next steps:"
    echo ""
    echo "1. In this terminal, start the Solace Agent Mesh:"
    echo "   uv run sam run configs/"
    echo ""
    echo "2. In a NEW terminal, run the heart rate monitor:"
    echo "   cd /Users/stevensu/solace-agent-mesh-hackathon-quickstart"
    echo "   uv run python src/main.py"
    echo ""
    echo "3. Open the WebUI in your browser:"
    echo "   http://localhost:8000"
    echo ""
    echo "Every 10 BPM readings will be sent to the orchestrator for AI analysis."
    echo ""
else
    echo ""
    echo "⚠️  WebUI API is not reachable yet."
    echo ""
    echo "This is normal if Solace Agent Mesh is not running."
    echo ""
    echo "Next steps:"
    echo ""
    echo "1. In this terminal, start the Solace Agent Mesh:"
    echo "   uv run sam run configs/"
    echo ""
    echo "2. Wait for the WebUI to start (you'll see 'Application startup complete')"
    echo ""
    echo "3. In a NEW terminal, run the heart rate monitor:"
    echo "   cd /Users/stevensu/solace-agent-mesh-hackathon-quickstart"
    echo "   uv run python src/main.py"
    echo ""
fi
