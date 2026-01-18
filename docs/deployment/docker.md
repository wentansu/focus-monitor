# Run with Docker

Run SAM locally using Docker. This uses the same Dockerfile as Railway, so your local environment matches deployment exactly.

## Prerequisites

- [Docker](https://docs.docker.com/get-docker/) installed
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

## 2. Build

```bash
docker build -t sam-hackathon-quickstart .
```

This copies your `configs/` and `src/` into the SAM base image and installs any additional dependencies from `pyproject.toml`.

## 3. Run

```bash
# Detached mode (recommended) - runs in background, returns immediately
docker run -d --rm -p 8000:8000 --env-file .env --name sam-app sam-hackathon-quickstart

# Or interactive mode - shows logs in terminal (Ctrl+C to stop)
docker run --rm -it -p 8000:8000 --env-file .env sam-hackathon-quickstart
```

If using detached mode:
- View logs: `docker logs sam-app` or `docker logs -f sam-app` (follow)
- Stop: `docker stop sam-app`

## 4. Access

Open [http://localhost:8000](http://localhost:8000) in your browser.

## Development Workflow

After modifying agents in `configs/` or tools in `src/`:

```bash
# Rebuild (fast)
docker build -t sam-hackathon-quickstart .

# Run again
docker run --rm -it -p 8000:8000 --env-file .env sam-hackathon-quickstart
```

## Optional: Persistent Storage

By default, data is lost when the container stops. For persistence, see [Persistent Storage with Supabase](../persistence.md).
