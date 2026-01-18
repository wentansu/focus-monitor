# Run with CLI

Run SAM locally using the `sam` CLI. This guide uses [uv](https://docs.astral.sh/uv/) for fast Python package management.

## Prerequisites

- [uv](https://docs.astral.sh/uv/getting-started/installation/) installed (handles Python automatically)
- [LLM API credentials](../llm-setup.md)

## 1. Clone and Configure

```bash
git clone https://github.com/SolaceDev/solace-agent-mesh-hackathon-quickstart.git
cd solace-agent-mesh-hackathon-quickstart

# Create environment file
cp .env.example .env
```

Edit `.env` with your LLM credentials (see [LLM Setup](../llm-setup.md) for free API options):

```env
LLM_SERVICE_API_KEY=your-api-key
LLM_SERVICE_ENDPOINT=https://api.cerebras.ai/v1
LLM_SERVICE_PLANNING_MODEL_NAME=openai/zai-glm-4.7
LLM_SERVICE_GENERAL_MODEL_NAME=openai/zai-glm-4.7
SOLACE_DEV_MODE=true
```

## 2. Install Dependencies

```bash
uv sync
```

This installs SAM and any additional dependencies (like PyGithub for GitHub tools) defined in `pyproject.toml`.

## 3. Run

```bash
uv run sam run configs/
```

The `sam run` command loads all YAML files from `configs/`.

## 4. Access

Open [http://localhost:8000](http://localhost:8000) in your browser.

## Development Workflow

After modifying agents or tools, just restart:

```bash
# Stop with Ctrl+C, then:
uv run sam run configs/
```

No rebuild needed—changes are picked up immediately.

## Optional: Persistent Storage

By default, data is lost when SAM stops. For persistence, see [Persistent Storage with Supabase](../persistence.md).
