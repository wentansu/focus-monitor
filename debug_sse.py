#!/usr/bin/env python3
"""
Debug script to see exactly what SSE events are being received from Solace.
"""

import requests
import json
from datetime import datetime

def debug_sse_stream():
    """Send a test message and print ALL SSE events received."""
    
    url = "http://localhost:8000/api/v1/message:stream"
    
    payload = {
        "jsonrpc": "2.0",
        "id": f"debug-{datetime.now().strftime('%Y%m%d%H%M%S')}",
        "method": "message/stream",
        "params": {
            "message": {
                "messageId": f"debug-{datetime.now().strftime('%Y%m%d%H%M%S')}",
                "role": "user",
                "kind": "message",
                "parts": [
                    {
                        "kind": "text",
                        "text": "Hello! Please respond with a simple greeting."
                    }
                ],
                "metadata": {
                    "agent_name": "OrchestratorAgent"
                }
            }
        }
    }
    
    print("=" * 60)
    print("DEBUG: Submitting task...")
    print("=" * 60)
    
    # Step 1: Submit task
    response = requests.post(url, json=payload, headers={"Content-Type": "application/json"}, timeout=10)
    
    if response.status_code != 200:
        print(f"❌ Task submission failed: {response.status_code}")
        print(response.text)
        return
    
    response_json = response.json()
    task_id = response_json.get("result", {}).get("id")
    
    if not task_id:
        print("❌ No task ID in response")
        print(json.dumps(response_json, indent=2))
        return
    
    print(f"✅ Task ID: {task_id}")
    
    # Step 2: Subscribe to SSE
    sse_url = f"http://localhost:8000/api/v1/sse/subscribe/{task_id}"
    print(f"\n{'=' * 60}")
    print(f"DEBUG: Subscribing to SSE stream...")
    print(f"URL: {sse_url}")
    print("=" * 60)
    
    sse_response = requests.get(sse_url, stream=True, timeout=30)
    
    if sse_response.status_code != 200:
        print(f"❌ SSE subscription failed: {sse_response.status_code}")
        return
    
    print("✅ SSE Connection established\n")
    print("=" * 60)
    print("RAW SSE EVENTS (everything received):")
    print("=" * 60)
    
    event_count = 0
    for line in sse_response.iter_lines():
        if line:
            decoded_line = line.decode('utf-8')
            print(f"[Event {event_count}] {decoded_line}")
            
            # Try to parse and pretty-print JSON data
            if decoded_line.startswith("data: "):
                data_str = decoded_line[6:]
                try:
                    data = json.loads(data_str)
                    print(f"  → Parsed JSON:")
                    print(json.dumps(data, indent=4))
                    
                    # Check what kind of event this is
                    if 'result' in data:
                        result = data['result']
                        event_kind = result.get('kind', 'unknown')
                        print(f"  → Event Kind: {event_kind}")
                        
                        if event_kind == 'message' and 'message' in result:
                            msg = result['message']
                            if 'parts' in msg:
                                for part in msg['parts']:
                                    if part.get('kind') == 'text':
                                        print(f"  → TEXT CONTENT: {part['text']}")
                        
                        if event_kind == 'task':
                            print("  → FINAL TASK EVENT (stream ending)")
                            break
                    
                except json.JSONDecodeError as e:
                    print(f"  → JSON Parse Error: {e}")
            
            event_count += 1
            print()
    
    print("=" * 60)
    print(f"Total events received: {event_count}")
    print("=" * 60)

if __name__ == "__main__":
    debug_sse_stream()
