# LLM API Setup

Solace Agent Mesh needs an LLM API to function. Cerebras offers a free tier that works well.

## Cerebras (Free)

1. Sign up at [cloud.cerebras.ai](https://cloud.cerebras.ai)
2. Create an API key
3. Add to `.env`:

```env
LLM_SERVICE_ENDPOINT=https://api.cerebras.ai/v1
LLM_SERVICE_API_KEY=your-api-key
LLM_SERVICE_PLANNING_MODEL_NAME=openai/zai-glm-4.7
LLM_SERVICE_GENERAL_MODEL_NAME=openai/zai-glm-4.7
```

> **Note:** Use these model names exactly as shown. The `openai/` prefix and model IDs are specific to Cerebras's API—other model names will fail silently.

**Free tier:** 1M tokens/day

### Alternative Model

For a larger model (slower but more capable):

```env
LLM_SERVICE_PLANNING_MODEL_NAME=openai/qwen-3-235b-a22b-instruct-2507
LLM_SERVICE_GENERAL_MODEL_NAME=openai/qwen-3-235b-a22b-instruct-2507
```

## Need More?

If free tiers are insufficient, reach out to hackathon sponsors from Solace for a provisioned API key.
