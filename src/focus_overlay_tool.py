import subprocess
import sys
import os
from typing import Any, Dict, Optional

from google.adk.tools import ToolContext


async def trigger_overlay_if_discard(
    verdict: str,
    reason: Optional[str] = None,
    tool_context: Optional[ToolContext] = None,
    tool_config: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    """
    SAFE tool module for SAM:
    - No tkinter import here
    - Spawns focus_overlay.py in a new process ONLY when verdict == DISCARD
    """
    verdict_norm = (verdict or "").strip().upper()
    if verdict_norm != "DISCARD":
        return {"status": "skipped", "verdict": verdict_norm}

    # Default overlay script (the GUI file)
    overlay_path = "src/focus_overlay.py"

    # Allow YAML/tool_config override if you ever want it
    if isinstance(tool_config, dict) and tool_config.get("overlay_script_path"):
        overlay_path = tool_config["overlay_script_path"]

    # Make it absolute so it works no matter what the working directory is
    overlay_abs = os.path.abspath(overlay_path)

    # Spawn detached process (non-blocking)
    subprocess.Popen([sys.executable, overlay_abs], close_fds=True)

    return {
        "status": "triggered",
        "verdict": verdict_norm,
        "reason": reason or "",
        "overlay_script_path": overlay_abs,
    }
