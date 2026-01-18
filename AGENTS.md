# AGENTS.md - AI Assistant Instructions

This file provides context for AI coding assistants working with this Solace Agent Mesh (SAM) hackathon quickstart project.

## Project Overview

This is a **hackathon quickstart template** for building SAM applications. The infrastructure is pre-configured—focus on creating agents and tools.

**Tech Stack:**

- Solace Agent Mesh 1.13.3 (multi-agent AI framework)
- Python 3.11 with uv package manager
- Docker for containerized deployment
- OpenAI-compatible LLM endpoints (Cerebras, etc.)

## Quick Reference

| Task                 | Command/Location                                                                                      |
| -------------------- | ----------------------------------------------------------------------------------------------------- |
| Run locally (Docker) | `docker build -t sam-hackathon-quickstart . && docker run -d --rm -p 8000:8000 --env-file .env --name sam-app sam-hackathon-quickstart` |
| Run locally (CLI)    | `uv sync && uv run sam run configs/`                                                                  |
| Web UI               | http://localhost:8000                                                                                 |
| Create new agent     | Add YAML file in `configs/agents/`                                                                    |
| Create custom tool   | Add Python function in `src/`, reference in agent YAML                                                |

## Context7: Your SAM Documentation Source

Before implementing agents, tools, or any SAM-specific code, query Context7 to get the correct patterns and signatures. SAM has specific conventions for YAML structure, tool function signatures, and configuration that differ from general Python patterns—Context7 provides the authoritative reference.

**Library ID:** `solacelabs/solace-agent-mesh`

**When to query Context7:**

- Creating or modifying agent YAML files
- Writing custom Python tool functions
- Configuring broker connections, LLM settings, or services
- Debugging SAM-specific errors or unexpected behavior
- Questions about available built-in tools or agent types

**Example queries:**

- "How do I define a custom tool function in SAM?"
- "What fields are required in an agent_card?"
- "Show me the tool_config parameter pattern"

This ensures your implementation matches SAM's actual API rather than assumptions based on general Python conventions.

## Agent Patterns

SAM agents have progressive levels of capability. Start simple, add tools as needed.

### Level 1: Basic (Instruction-Only)

Pure LLM reasoning—no tools, just a system prompt.

```yaml
# configs/agents/my-agent.yaml
!include ../shared_config.yaml

apps:
  - name: my-agent_app
    app_module: solace_agent_mesh.agent.sac.app
    broker:
      <<: *broker_connection
    app_config:
      agent_name: "MyAgent"
      model: *general_model
      instruction: |
        You are a helpful assistant that [does X].
        # Your agent's personality and capabilities go here.

      agent_card:
        description: >
          [Agent Name] - [What it does]. Use this agent when the user asks
          "[example query 1]", "[example query 2]", or needs help with [domain].
```

### Level 2: Enriched (Built-in Tools)

Add SAM's built-in tool groups for common capabilities.

```yaml
instruction: |
  You are a helpful assistant with file management capabilities...

tools:
  - tool_type: builtin-group
    group_name: artifact_management # create, read, update, list, delete files
```

**Available built-in groups:** `artifact_management`, `web_request_tools`, `image_tools`, and more. Query Context7 for the full list.

### Level 3a: Advanced (Custom Python Tools)

Write Python functions for custom integrations.

```yaml
tools:
  - tool_type: python
    component_module: "src.my_tools" # Python module path
    component_base_path: .
    function_name: "my_custom_function" # Function to expose
    tool_config: # Config passed to function
      api_key: ${MY_API_KEY,}
```

**Python tool signature:**

```python
# src/my_tools.py
import logging
from typing import Any, Dict, Optional

log = logging.getLogger(__name__)

async def my_custom_function(
    query: str,                                   # LLM provides
    count: int = 10,                              # LLM provides (with default)
    tool_context: Optional[Any] = None,           # Framework injects (session info)
    tool_config: Optional[Dict[str, Any]] = None, # From YAML tool_config section
) -> Dict[str, Any]:
    """Short description shown to the LLM."""
    log_id = f"[MyTools:my_custom_function:{query[:20]}]"
    log.debug(f"{log_id} Starting")

    api_key = tool_config.get("api_key") if tool_config else None

    try:
        # Your implementation here
        result = do_something(query, api_key)
        log.info(f"{log_id} Success")
        return {"status": "success", "data": result}
    except Exception as e:
        log.error(f"{log_id} Failed: {e}", exc_info=True)
        return {"status": "error", "message": str(e)}
```

### Level 3b: Advanced (MCP Integration)

Connect to MCP servers without writing Python code.

```yaml
tools:
  - tool_type: mcp
    connection_params:
      type: streamable-http
      url: "https://mcp.example.com/mcp" # Remote MCP server
  # Or for local MCP servers:
  # - tool_type: mcp
  #   connection_params:
  #     type: stdio
  #     command: "npx"
  #     args: ["-y", "@some/mcp-server"]
```

**When to use MCP vs Python:**

| Approach                     | Use When                                     |
| ---------------------------- | -------------------------------------------- |
| MCP (`tool_type: mcp`)       | Service already has an MCP server available  |
| Python (`tool_type: python`) | Custom logic needed, or no MCP server exists |

### Progression Summary

| Level    | What You Add                      | New Capabilities                           |
| -------- | --------------------------------- | ------------------------------------------ |
| Basic    | Just `instruction`                | LLM reasoning only                         |
| Enriched | `tool_type: builtin-group`        | Save/load files, use built-in tools        |
| Advanced | `tool_type: python` + custom code | Full custom integrations (APIs, databases) |
| Advanced | `tool_type: mcp` + server URL     | Connect to MCP servers (local or remote)   |

## Artifact Scoping

By default, artifacts created by agents are **session-scoped** - they disappear when you start a new conversation.

### Scoping Options

Artifacts created by agents are **session-scoped** — they exist only within the current conversation and disappear when you start a new chat.

### Project Knowledge vs Agent Artifacts

| Type                  | How Created                       | Shared?                           |
| --------------------- | --------------------------------- | --------------------------------- |
| **Project Knowledge** | Upload via UI (Files panel)       | ✅ Across all sessions in project |
| **Agent Artifacts**   | Created by `create_artifact` tool | ❌ Current session only           |

## Logging in Custom Tools

Add logging to your custom Python tools for easier debugging. SAM uses Python's standard `logging` module—your logs will appear in the console alongside framework logs.

### Quick Pattern

```python
import logging

log = logging.getLogger(__name__)

async def my_tool(param: str, ...) -> Dict[str, Any]:
    log_id = f"[MyTool:function_name:{param}]"  # Bracketed identifier for filtering
    log.debug(f"{log_id} Starting with param={param}")

    try:
        # ... your implementation ...
        log.info(f"{log_id} Success")
        return {"status": "success", ...}
    except Exception as e:
        log.error(f"{log_id} Failed: {e}", exc_info=True)  # exc_info captures stack trace
        return {"status": "error", "message": str(e)}
```

### Key Points

| Element                         | Purpose                                      |
| ------------------------------- | -------------------------------------------- |
| `logging.getLogger(__name__)`   | Module-level logger, configured by SAM       |
| `[ToolName:function:context]`   | Bracketed prefix for easy log filtering      |
| `log.debug(...)`                | Verbose details (params, intermediate state) |
| `log.info(...)`                 | Success messages, key operations             |
| `log.error(..., exc_info=True)` | Errors with full stack trace                 |

### Adjusting Log Levels

Agent YAML files control log verbosity:

```yaml
log:
  stdout_log_level: DEBUG # Show debug logs (default: INFO)
```

## Environment Variables

See [LLM API Setup](docs/llm-setup.md) for required variables and free API options. Use `.env.example` as a template.

## Guidelines for AI Assistants

### Use exact values from reference files

Unless the user specifies their own values, do not guess or invent configuration. Use the reference files as the source of truth:

| Config needed       | Read this file first         |
| ------------------- | ---------------------------- |
| LLM model names     | `docs/llm-setup.md`          |
| Environment vars    | `.env.example`               |
| YAML anchors        | `configs/shared_config.yaml` |

If the user provides their own API key, endpoint, or model preference, use their values. Otherwise, copy values character-for-character from the reference files—guessed model names cause silent failures that waste hackathon time.

### Running Docker containers

When running Docker containers, always use **detached mode** (`-d` flag) so the container runs in the background and returns control immediately. This works reliably across all AI coding assistants.

```bash
# Correct - detached mode
docker run -d --rm -p 8000:8000 --env-file .env --name sam-app sam-hackathon-quickstart

# Incorrect - blocks the terminal or may not work in some environments
docker run --rm -it -p 8000:8000 --env-file .env sam-hackathon-quickstart
```

After starting in detached mode:
- Check status: `docker ps`
- View logs: `docker logs sam-app` or `docker logs -f sam-app` (follow)
- Stop: `docker stop sam-app`

### Workflow for setup and deployment tasks

When the user asks you to run, deploy, or set up the project:

1. **Read the reference files first** - before creating `.env` or running commands, read `docs/llm-setup.md` and `.env.example` to understand the required configuration.

2. **Use user-provided values or copy from docs** - if the user provided specific values (API key, endpoint, model), use those. For anything not specified, copy exactly from the reference files.

3. **Execute the commands** - run the actual build/deploy commands rather than explaining what the user could do. When running Docker containers, always use detached mode (`-d`).

### Workflow for creating agents or tools

When the user asks you to create an agent, add a tool, or write SAM-specific code, follow this sequence:

1. **Query Context7 first** using library ID `/solacelabs/solace-agent-mesh`. SAM has specific conventions for YAML structure, tool function signatures, and configuration that differ from general Python patterns. Getting the correct patterns from Context7 prevents writing code that doesn't work with the framework.

2. **Check existing files** to match project conventions:

   - Look at any existing agent YAMLs in `configs/agents/` for patterns
   - Check `configs/shared_config.yaml` for YAML anchors to reuse
   - Review any existing tools in `src/` if present

3. **Implement the solution** - create the actual files rather than describing what to do. Hackathon participants want working code.

This sequence matters because SAM's patterns are framework-specific. Code that looks correct from general Python knowledge often fails—for example, tool functions require specific parameter names (`tool_context`, `tool_config`) that the framework injects.

### Working with code

Read and understand relevant files before proposing changes or creating configuration. Do not speculate about values you have not read from source files. If the user asks you to configure something, open and read the authoritative source first.

When the user asks for a change, implement it rather than describing what to do. Hackathon participants want working code, not explanations of what they could do themselves.

Base answers on actual file contents rather than assumptions. If unsure how something works, read the code first—this ensures accurate guidance that won't mislead participants.

### When creating agents

Follow the patterns established in existing agent files:

- Include `!include shared_config.yaml` at the top, which provides shared YAML anchors
- Use YAML anchors like `*broker_connection`, `*general_model`, `*default_artifact_service` to avoid duplicating configuration
- Include an `agent_card` section with `title` and `description` so the orchestrator can discover the agent
- Use unique agent names across the project to prevent conflicts
- Use environment variable substitution for secrets: `${VAR_NAME}` or `${VAR_NAME,default}`

### Writing effective agent_card descriptions

The Orchestrator routes user requests to agents based on their `agent_card.description`. This description is the primary signal the Orchestrator uses to decide which agent should handle a request—it needs sufficient context to make confident routing decisions.

**Write descriptions that explicitly state when to use the agent:**

```yaml
# Effective - explicit routing guidance
agent_card:
  title: "IP Finder"
  description: >
    IP Address Finder Agent - Retrieves the user's current public IP address
    from external services. Use this agent when the user asks "What is my IP?",
    "Show my IP address", "What's my public IP?", or needs network diagnostic
    information about their public-facing IP address.
```

```yaml
# Ineffective - too vague for reliable routing
agent_card:
  title: "IP Finder"
  description: "Finds the user's public IP address"
```

**Include these elements in your description:**

| Element         | Purpose                               | Example                                          |
| --------------- | ------------------------------------- | ------------------------------------------------ |
| Agent purpose   | What the agent does                   | "Retrieves the user's current public IP address" |
| Trigger phrases | When to route to this agent           | "Use this agent when the user asks..."           |
| Example queries | Concrete phrasings users might say    | "'What is my IP?', 'Show my IP address'"         |
| Domain keywords | Related terms that indicate relevance | "network diagnostic", "public-facing IP"         |

The Orchestrator matches user intent against these descriptions. Verbose, explicit descriptions with multiple matching opportunities outperform terse ones.

### Preventing tool output hallucination

LLMs can ignore tool output and hallucinate fake data—even when the correct data is in context. For example, a tool returns IP `66.46.76.82` but the agent responds with `34.204.128.41`.

**Add explicit instructions to enforce tool output usage:**

```yaml
instruction: |
  You are an IP address lookup agent.

  IMPORTANT - Tool Output Rules:
  - WAIT for the tool to return before responding
  - Use the EXACT value returned by the tool
  - NEVER guess, estimate, or make up values
  - If the tool fails, say so—don't fabricate a response

  Format your response as: "Your public IP address is: [exact tool result]"
```

**Why this happens:** The agent's LLM generates the final response. Even with tool output in context, the model may confidently generate fake data. Explicit instructions reduce this risk.

**High-risk scenarios:**

- Tools returning specific values (IPs, IDs, URLs, timestamps)
- Data lookups where accuracy matters
- Any tool where a plausible-but-wrong answer exists

**Lower-risk scenarios:**

- Text transformation (summarization, formatting)
- Tools returning large amounts of context
- Creative tasks where exact values don't matter

### Keep solutions minimal

This is a hackathon quickstart where speed matters—favor working code over perfect architecture:

- Start with the simplest approach that works
- Add complexity only when needed
- One agent per YAML file for clarity
- Don't create extra files, abstractions, or "improvements" beyond what was asked—over-engineering slows down iteration
- Treat `shared_config.yaml`, `orchestrator.yaml`, and `webui.yaml` as infrastructure (read, don't modify)

## Deployment

| Method   | Use Case                              |
| -------- | ------------------------------------- |
| Docker   | Quick testing, consistent environment |
| CLI (uv) | Active development, faster iteration  |
| Railway  | Public deployment, sharing            |

See [deployment guides](docs/deployment/) for details.
