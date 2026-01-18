#!/usr/bin/env python3
"""
Test the FIXED SSE parsing logic with the updated code.
"""

import requests
import json
from datetime import datetime
import logging

logging.basicConfig(level=logging.INFO)
log = logging.getLogger(__name__)

def test_fixed_parsing():
    """Test with the fixed SSE parsing logic."""
    
    url = "http://localhost:8000/api/v1/message:stream"
    
    payload = {
        "jsonrpc": "2.0",
        "id": f"test-fixed-{datetime.now().strftime('%Y%m%d%H%M%S')}",
        "method": "message/stream",
        "params": {
            "message": {
                "messageId": f"test-fixed-{datetime.now().strftime('%Y%m%d%H%M%S')}",
                "role": "user",
                "kind": "message",
                "parts": [
                    {
                        "kind": "text",
                        "text": "Hello! Please respond with a simple message."
                    }
                ],
                "metadata": {
                    "agent_name": "OrchestratorAgent"
                }
            }
        }
    }
    
    print("Testing FIXED SSE parsing logic...")
    print("=" * 60)
    
    # Step 1: Submit task
    response = requests.post(url, json=payload, headers={"Content-Type": "application/json"}, timeout=10)
    
    if response.status_code != 200:
        print(f"❌ Failed: {response.status_code}")
        return
    
    task_id = response.json().get("result", {}).get("id")
    print(f"✅ Task ID: {task_id}")
    
    # Step 2: Subscribe with FIXED parsing logic
    sse_url = f"http://localhost:8000/api/v1/sse/subscribe/{task_id}"
    print(f"\nSubscribing to: {sse_url}")
    print("=" * 60)
    print("ORCHESTRATOR RESPONSE:")
    print("=" * 60)
    
    sse_response = requests.get(sse_url, stream=True, timeout=30)
    
    if sse_response.status_code == 200:
        full_response = ""
        for line in sse_response.iter_lines():
            if line:
                try:
                    decoded_line = line.decode('utf-8')
                    if decoded_line.startswith("data: "):
                        data_str = decoded_line[6:]
                        data = json.loads(data_str)
                        
                        # FIXED: Extract text content from different event types
                        if 'result' in data:
                            result = data['result']
                            
                            # Try to find message in different locations
                            msg = None
                            
                            # For status-update events: result.status.message
                            if 'status' in result and 'message' in result['status']:
                                msg = result['status']['message']
                            # For direct message events: result.message
                            elif 'message' in result:
                                msg = result['message']
                            
                            # Extract text from message parts
                            if msg and 'parts' in msg:
                                for part in msg['parts']:
                                    if part.get('kind') == 'text':
                                        text = part['text']
                                        print(text, end="", flush=True)
                                        full_response += text
                            
                            # Check for final task response
                            if result.get('kind') == 'task':
                                break
                except Exception as e:
                    log.debug(f"Error parsing SSE line: {e}")
        
        print("\n" + "=" * 60)
        print(f"\n✅ Received {len(full_response)} characters of response")
        if full_response:
            print(f"✅ SUCCESS - Text was extracted and displayed!")
        else:
            print(f"⚠️  No text content received (likely due to LLM errors)")
    else:
        print(f"❌ SSE failed: {sse_response.status_code}")

if __name__ == "__main__":
    test_fixed_parsing()
