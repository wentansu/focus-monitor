import argparse
import json
import socket
import time
from datetime import datetime


def now():
    return datetime.now().strftime("%H:%M:%S")


def last_value_rate(rate_list):
    # expected: [{"value": 46.8, ...}, ...]
    if isinstance(rate_list, list) and rate_list:
        last = rate_list[-1]
        if isinstance(last, dict):
            return last.get("value")
    return None


def any_event(event_list):
    return 1 if isinstance(event_list, list) and len(event_list) > 0 else 0


def normalize_presage(obj: dict) -> dict:
    md = obj.get("metadata", {}) if isinstance(obj.get("metadata", {}), dict) else {}
    data = obj.get("data", {}) if isinstance(obj.get("data", {}), dict) else {}

    ts = md.get("timestamp") or data.get("timestamp")
    session_id = obj.get("session_id") or md.get("session_id") or data.get("session_id")
    source = md.get("source", "unknown")
    msg_type = md.get("type") or data.get("dataType") or "unknown"

    pulse = data.get("pulse", {}) if isinstance(data.get("pulse", {}), dict) else {}
    breathing = data.get("breathing", {}) if isinstance(data.get("breathing", {}), dict) else {}
    face = data.get("face", {}) if isinstance(data.get("face", {}), dict) else {}

    hr_bpm = last_value_rate(pulse.get("rate"))
    breath_rpm = last_value_rate(breathing.get("rate"))

    blink_event = any_event(face.get("blinking"))
    talk_event = any_event(face.get("talking"))

    hr_stable = None
    if isinstance(pulse.get("trace"), list) and pulse["trace"]:
        tlast = pulse["trace"][-1]
        if isinstance(tlast, dict) and "stable" in tlast:
            hr_stable = bool(tlast["stable"])

    breath_stable = None
    if isinstance(breathing.get("upperTrace"), list) and breathing["upperTrace"]:
        ulast = breathing["upperTrace"][-1]
        if isinstance(ulast, dict) and "stable" in ulast:
            breath_stable = bool(ulast["stable"])

    return {
        "type": "presage_edge_metrics",
        "presage_type": msg_type,
        "ts": ts,
        "session_id": session_id,
        "source": source,
        "hr_bpm": hr_bpm,
        "hr_stable": hr_stable,
        "breath_rpm": breath_rpm,
        "breath_stable": breath_stable,
        "blink_event": blink_event,
        "talk_event": talk_event,
    }


def stream_key(norm: dict, addr: str) -> str:
    # prefer stable identity; fall back to addr if session_id missing
    sid = norm.get("session_id") or "no_session"
    src = norm.get("source") or "no_source"
    return f"{sid}|{src}|{addr}"


def run_udp(host: str, port: int, timeout_s: float, emit_every_s: float):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((host, port))
    sock.settimeout(timeout_s)

    latest_by_stream = {}   # key -> latest normalized dict
    last_emit_by_stream = {}  # key -> monotonic time

    print(f"[{now()}] UDP listening on {host}:{port} (timeout={timeout_s}s, emit_every={emit_every_s}s)")

    while True:
        # 1) Receive (non-blocking-ish due to timeout)
        try:
            data, addr = sock.recvfrom(65535)
            text = data.decode("utf-8", errors="replace").strip()
            if not text:
                continue
            try:
                obj = json.loads(text)
            except json.JSONDecodeError:
                continue

            norm = normalize_presage(obj)
            key = stream_key(norm, addr=str(addr))
            latest_by_stream[key] = norm

        except socket.timeout:
            pass
        except KeyboardInterrupt:
            print("\n[CTRL+C] Stopping UDP server.")
            break

        # 2) Emit at a fixed cadence (per stream)
        if emit_every_s <= 0:
            continue

        t = time.monotonic()
        # emit for each stream that has data
        for key, norm in list(latest_by_stream.items()):
            last_t = last_emit_by_stream.get(key, 0.0)
            if (t - last_t) >= emit_every_s:
                last_emit_by_stream[key] = t
                # Output one JSON object per line (easy for piping into Solace publisher)
                print(json.dumps(norm, ensure_ascii=False), flush=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=5005)
    ap.add_argument("--timeout", type=float, default=0.2)
    ap.add_argument("--emit-every", type=float, default=1.0, help="Emit latest normalized JSON every N seconds")
    args = ap.parse_args()

    run_udp(args.host, args.port, args.timeout, args.emit_every)


if __name__ == "__main__":
    main()
