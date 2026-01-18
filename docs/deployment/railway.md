# Deploy to Railway

[Railway](https://railway.com) offers a simple way to deploy SAM with automatic builds from GitHub.

## 1. Clone and Push to Your GitHub

```bash
git clone https://github.com/SolaceDev/solace-agent-mesh-hackathon-quickstart.git
cd solace-agent-mesh-hackathon-quickstart
```

Create a new repository in your GitHub account, then push:

```bash
git remote set-url origin https://github.com/YOUR_USERNAME/your-repo-name.git
git push -u origin main
```

## 2. Create a Railway Account

Go to [railway.com](https://railway.com) and sign up (free tier available, no credit card required).

## 3. Deploy to Railway

1. In Railway, click **New Project** → **Deploy from GitHub repo**
2. Connect your GitHub account if prompted
3. Select your repository
4. Railway will detect the Dockerfile and start building (expect 4-5 minutes)

## 4. Configure Environment Variables

Once the service is created, go to **Variables** tab and add the required variables:

### Required Variables

See [LLM Setup](../llm-setup.md) for free API options.

| Variable                          | Description                                      | Example                        |
| --------------------------------- | ------------------------------------------------ | ------------------------------ |
| `LLM_SERVICE_API_KEY`             | Your LLM provider API key                        | `your-api-key`                 |
| `LLM_SERVICE_ENDPOINT`            | LLM API endpoint URL                             | `https://api.cerebras.ai/v1`   |
| `LLM_SERVICE_PLANNING_MODEL_NAME` | Model for complex planning tasks                 | `openai/zai-glm-4.7`           |
| `LLM_SERVICE_GENERAL_MODEL_NAME`  | Model for general tasks                          | `openai/zai-glm-4.7`           |
| `SOLACE_DEV_MODE`                 | Use in-memory broker (no external Solace needed) | `true`                         |

### Copy-Paste Template (for Railway Raw Editor)

```env
LLM_SERVICE_API_KEY=your-api-key
LLM_SERVICE_ENDPOINT=https://api.cerebras.ai/v1
LLM_SERVICE_PLANNING_MODEL_NAME=openai/zai-glm-4.7
LLM_SERVICE_GENERAL_MODEL_NAME=openai/zai-glm-4.7
SOLACE_DEV_MODE=true
```

> **Tip:** Go to **Variables** → **Raw Editor**, paste the above, replace `your-api-key` with your API key, and save.

After saving, redeploy the service to apply the new variables.

## 5. Expose the Web UI

1. Go to **Settings** tab → **Networking**
2. Under **Public Networking**, click **Generate Domain**
3. Enter port `8000` when prompted
4. Railway will create a public URL (e.g., `https://your-repo-name-xxxx.up.railway.app`)

## 6. Access Your Deployment

Open the generated URL in your browser. You should see the SAM Web UI with the example agents ready to use.

> **Security Note:** The SAM Web UI has no authentication—anyone with your Railway URL can use your deployed instance. If you share the URL with teammates, be aware that all usage consumes your LLM API key quota. Monitor your API usage if sharing broadly.

## Deploy Changes

After making changes to your agents:

```bash
git add .
git commit -m "Update agents"
git push
```

Railway will automatically redeploy with your changes.

## Optional: Persistent Storage

By default, data is lost on Railway restarts. For persistence, see [Persistent Storage with Supabase](../persistence.md).
