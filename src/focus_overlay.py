import os
import sys
import time
import threading
import random
import subprocess
from pathlib import Path
from typing import Any, Dict, Optional

import tkinter as tk

# ---------- CONFIG ----------
# Set to 0 to remove the delay entirely (as you wanted)
DELAY_SECONDS = 0

OPACITY = 0.97
BUTTON_LOCK_SECONDS = 4
FLASH_MS = 260
GLITCH_MS = 120
SCAN_MS = 18

BTN_W = 520
BTN_H = 76
BTN_RADIUS = 28

# Optional debug log (helps you prove it launched)
DEBUG_LOG = True
# ---------------------------


def _log(msg: str) -> None:
    if not DEBUG_LOG:
        return
    try:
        log_path = Path(__file__).with_suffix(".runlog.txt")
        with open(log_path, "a", encoding="utf-8") as f:
            f.write(msg.rstrip() + "\n")
    except Exception:
        pass


# ---------------------------
# TOOL FUNCTION (SAM calls this)
# ---------------------------
async def trigger_overlay_if_discard(
    verdict: str,
    reason: Optional[str] = None,
    tool_context: Optional[Any] = None,   # keep signature flexible; no ADK import needed
    tool_config: Optional[Dict[str, Any]] = None,
) -> Dict[str, Any]:
    """
    Launch fullscreen overlay ONLY when verdict == DISCARD.
    Non-blocking: spawns a new process.
    """
    verdict_norm = (verdict or "").strip().upper()
    if verdict_norm != "DISCARD":
        return {"status": "skipped", "verdict": verdict_norm}

    # Spawn THIS SAME FILE in "overlay mode"
    script_path = Path(__file__).resolve()

    # Pass reason via env (safer than argv for weird characters)
    env = os.environ.copy()
    env["FOCUS_OVERLAY_REASON"] = reason or ""

    _log(f"[TOOL] Spawning overlay: {script_path} reason={env['FOCUS_OVERLAY_REASON']!r}")

    # Use sys.executable (same python environment SAM is using)
    subprocess.Popen(
        [sys.executable, str(script_path), "--overlay"],
        close_fds=True,
        env=env,
        cwd=str(script_path.parent),  # stable working directory
    )

    return {
        "status": "triggered",
        "verdict": verdict_norm,
        "reason": reason or "",
        "overlay_script_path": str(script_path),
    }


# ---------------------------
# OVERLAY UI
# ---------------------------
class FocusOverlay:
    def __init__(self, reason_text: str = ""):
        # ---- ROOT WINDOW ----
        self.root = tk.Tk()
        self.root.title("Focus Alert")
        self.root.attributes("-fullscreen", True)
        self.root.attributes("-topmost", True)
        self.root.overrideredirect(True)
        self.root.configure(bg="black")
        try:
            self.root.attributes("-alpha", OPACITY)
        except Exception:
            pass

        # Block close keys
        for key in ("<Escape>", "<Alt-F4>", "<Control-w>", "<Control-W>"):
            self.root.bind(key, lambda e: "break")

        # ---- SCREEN SIZE ----
        self.w = self.root.winfo_screenwidth()
        self.h = self.root.winfo_screenheight()

        # ---- STATE (define BEFORE draw) ----
        self._scan_y = int(self.h * 0.18)
        self._flash_on = True
        self._running = False
        self.btn_enabled = False

        # ---- COPY ----
        self.base_title = "FOCUS REQUIRED"
        # Add reason if provided
        if reason_text.strip():
            self.base_sub = f"IMMEDIATE DISCARD ALERT.\n{reason_text.strip()}"
        else:
            self.base_sub = "Attention has dropped.\nReturn to the task now."

        # ---- THEME (ALL RED) ----
        self.alert_red = "#FF1E1E"
        self.alert_dark = "#8B0000"
        self.alert_dim = "#4A0A0A"
        self.soft_gray = "#C8C8C8"
        self.panel_bg = "#0A0505"
        self.btn_bg = self.alert_red
        self.btn_bg_hover = "#FF4040"

        # ---- CANVAS ----
        self.c = tk.Canvas(self.root, width=self.w, height=self.h, bg="black", highlightthickness=0)
        self.c.pack(fill="both", expand=True)

        # ---- DRAW ----
        self._draw_background()
        self._draw_warning_band()
        self._draw_center_panel()
        self._draw_status_readout()
        self._draw_rounded_button()
        self._draw_lock_label()

        # Bind hover + click for the rounded button
        self.c.tag_bind(self.btn_tag, "<Enter>", self._btn_hover_on)
        self.c.tag_bind(self.btn_tag, "<Leave>", self._btn_hover_off)
        self.c.tag_bind(self.btn_tag, "<Button-1>", self._btn_click)

        # Start hidden, then show after delay
        self.hide()
        if DELAY_SECONDS > 0:
            threading.Thread(target=self._delayed_show, daemon=True).start()
        else:
            self.show()

        self.root.mainloop()

    # ---------- UI Helpers ----------
    def _rounded_rect(self, x1, y1, x2, y2, r, **kwargs):
        points = [
            x1+r, y1,
            x2-r, y1,
            x2, y1,
            x2, y1+r,
            x2, y2-r,
            x2, y2,
            x2-r, y2,
            x1+r, y2,
            x1, y2,
            x1, y2-r,
            x1, y1+r,
            x1, y1
        ]
        return self.c.create_polygon(points, smooth=True, **kwargs)

    def _set_button_enabled(self, enabled: bool):
        self.btn_enabled = enabled
        if enabled:
            self.c.itemconfig(self.btn_body, fill=self.btn_bg)
            self.c.itemconfig(self.btn_text, fill="white")
            self.c.itemconfig(self.btn_glow, outline=self.alert_red)
        else:
            self.c.itemconfig(self.btn_body, fill=self.alert_dim)
            self.c.itemconfig(self.btn_text, fill="#B4BFCC")
            self.c.itemconfig(self.btn_glow, outline=self.alert_dim)

    def _btn_hover_on(self, _e=None):
        if not self.btn_enabled:
            return
        self.c.itemconfig(self.btn_body, fill=self.btn_bg_hover)

    def _btn_hover_off(self, _e=None):
        if not self.btn_enabled:
            return
        self.c.itemconfig(self.btn_body, fill=self.btn_bg)

    def _btn_click(self, _e=None):
        if not self.btn_enabled:
            return
        self.exit_app()

    # ---------- DRAW ----------
    def _draw_background(self):
        step = 100
        for x in range(0, self.w + 1, step):
            self.c.create_line(x, 0, x, self.h, fill="#1A0505")
        for y in range(0, self.h + 1, step):
            self.c.create_line(0, y, self.w, y, fill="#1A0505")
        for y in range(0, self.h, 7):
            self.c.create_line(0, y, self.w, y, fill="#050202")

    def _draw_warning_band(self):
        self.band = self.c.create_rectangle(0, 0, self.w, 90, fill=self.alert_dark, outline="")
        self.band_text = self.c.create_text(
            self.w * 0.5, 45,
            text="ATTENTION ALERT  •  FOCUS REQUIRED  •  ATTENTION ALERT  •  FOCUS REQUIRED",
            fill="white",
            font=("Segoe UI", 18, "bold")
        )

    def _draw_center_panel(self):
        px1, py1 = int(self.w * 0.20), int(self.h * 0.23)
        px2, py2 = int(self.w * 0.80), int(self.h * 0.77)
        self.panel_coords = (px1, py1, px2, py2)

        self.c.create_rectangle(px1, py1, px2, py2, outline=self.alert_red, width=2)
        self.c.create_rectangle(px1+10, py1+10, px2-10, py2-10, outline=self.alert_dim, width=2)
        self.c.create_rectangle(px1+18, py1+18, px2-18, py2-18, fill=self.panel_bg, outline=self.alert_dim, width=2)

        cx = int(self.w * 0.5)
        cy = int(self.h * 0.42)

        self.text_main = self.c.create_text(
            cx, cy,
            text=self.base_title,
            fill="white",
            font=("Segoe UI", 56, "bold")
        )
        self.text_shadow = self.c.create_text(
            cx+2, cy+2,
            text=self.base_title,
            fill=self.alert_red,
            font=("Segoe UI", 56, "bold")
        )

        self.subtext = self.c.create_text(
            cx, int(self.h * 0.52),
            text=self.base_sub,
            fill=self.soft_gray,
            font=("Segoe UI", 20),
            justify="center"
        )

        self.scanline = self.c.create_rectangle(
            px1 + 20, self._scan_y, px2 - 20, self._scan_y + 60,
            fill=self.alert_red, outline="",
            stipple="gray25"
        )

    def _draw_status_readout(self):
        base_x = int(self.w * 0.22)
        base_y = int(self.h * 0.27)
        lines = [
            "STATUS: ALERT",
            "ACTION: FULLSCREEN OVERLAY",
            "RESPONSE: ACKNOWLEDGE"
        ]
        for i, line in enumerate(lines):
            self.c.create_text(
                base_x, base_y + i * 22,
                text=line,
                fill=self.alert_red,
                font=("Consolas", 12, "bold"),
                anchor="w"
            )

    def _draw_rounded_button(self):
        cx = int(self.w * 0.5)
        cy = int(self.h * 0.73)
        x1 = cx - BTN_W // 2
        y1 = cy - BTN_H // 2
        x2 = cx + BTN_W // 2
        y2 = cy + BTN_H // 2

        self.btn_tag = "ack_button"
        self.btn_glow = self._rounded_rect(
            x1-6, y1-6, x2+6, y2+6, BTN_RADIUS+6,
            fill="", outline=self.alert_red, width=2, tags=self.btn_tag
        )
        self.btn_body = self._rounded_rect(
            x1, y1, x2, y2, BTN_RADIUS,
            fill=self.btn_bg, outline="", width=0, tags=self.btn_tag
        )
        self.btn_text = self.c.create_text(
            cx, cy,
            text="I ACKNOWLEDGE",
            fill="white",
            font=("Segoe UI", 18, "bold"),
            tags=self.btn_tag
        )
        self._set_button_enabled(False)

    def _draw_lock_label(self):
        self.lock_label = self.c.create_text(
            self.w * 0.5, self.h * 0.67,
            text="ACKNOWLEDGEMENT LOCKED…",
            fill=self.soft_gray,
            font=("Consolas", 14, "bold")
        )

    # ---------- ANIMATION ----------
    def _flash_band(self):
        if not self._running or not self.root.winfo_viewable():
            return
        self._flash_on = not self._flash_on
        bg = "#3A0016" if self._flash_on else self.alert_dark
        fg = "white" if self._flash_on else "black"
        self.c.itemconfig(self.band, fill=bg)
        self.c.itemconfig(self.band_text, fill=fg)
        self.root.after(FLASH_MS, self._flash_band)

    def _scan_sweep(self):
        if not self._running or not self.root.winfo_viewable():
            return
        x1, y1, x2, y2 = self.panel_coords
        top = y1 + 30
        bot = y2 - 90

        self._scan_y += 7
        if self._scan_y > bot:
            self._scan_y = top

        self.c.coords(self.scanline, x1 + 20, self._scan_y, x2 - 20, self._scan_y + 60)
        self.root.after(SCAN_MS, self._scan_sweep)

    def _glitch_title(self):
        if not self._running or not self.root.winfo_viewable():
            return
        dx = random.choice([-2, -1, 1, 2]) if random.random() < 0.10 else 0
        dy = random.choice([-2, -1, 1, 2]) if random.random() < 0.10 else 0

        cx = int(self.w * 0.5)
        cy = int(self.h * 0.42)
        self.c.coords(self.text_main, cx, cy)
        self.c.coords(self.text_shadow, cx + 2 + dx, cy + 2 + dy)

        self.root.after(GLITCH_MS, self._glitch_title)

    def _lock_countdown(self, sec):
        if not self._running or not self.root.winfo_viewable():
            return
        if sec <= 0:
            self.c.itemconfig(self.lock_label, text="ACKNOWLEDGE TO CONTINUE.")
            self._set_button_enabled(True)
            return
        self.c.itemconfig(self.lock_label, text=f"ACKNOWLEDGEMENT LOCKED…  {sec}s")
        self.root.after(1000, lambda: self._lock_countdown(sec - 1))

    # ---------- LOGIC ----------
    def _delayed_show(self):
        time.sleep(DELAY_SECONDS)
        self.show()

    def show(self):
        self.root.deiconify()
        self.root.attributes("-topmost", True)
        self.root.lift()
        try:
            self.root.focus_force()
        except Exception:
            pass

        self._running = True
        self._set_button_enabled(False)
        self.c.itemconfig(self.lock_label, text="ACKNOWLEDGEMENT LOCKED…")

        self._flash_band()
        self._scan_sweep()
        self._glitch_title()
        self._lock_countdown(BUTTON_LOCK_SECONDS)

    def hide(self):
        self._running = False
        self.root.withdraw()
        self._set_button_enabled(False)
        self.c.itemconfig(self.lock_label, text="ACKNOWLEDGEMENT LOCKED…")

    def exit_app(self):
        self._running = False
        try:
            self.root.destroy()
        finally:
            sys.exit(0)


# ---------------------------
# Entrypoints
# ---------------------------
def _run_overlay_from_cli() -> None:
    reason = os.environ.get("FOCUS_OVERLAY_REASON", "")
    _log(f"[OVERLAY] Starting overlay. reason={reason!r}")
    FocusOverlay(reason_text=reason)


if __name__ == "__main__":
    # Run as: python src/focus_overlay_tool.py --overlay
    if "--overlay" in sys.argv:
        _run_overlay_from_cli()
    else:
        # Manual dev test: python src/focus_overlay_tool.py
        _log("[MAIN] Manual run (no args) -> showing overlay")
        _run_overlay_from_cli()
