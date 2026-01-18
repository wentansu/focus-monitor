#!/usr/bin/env python3
"""
Test with actual heart rate data to verify the enhanced parsing extracts all content types.
"""

import requests
import json
from datetime import datetime
import logging

logging.basicConfig(level=logging.INFO)
log = logging.getLogger(__name__)

def test_heart_rate_integration():
    """Test with heart rate data similar to what main.py sends."""
    
    url = "http://localhost:8000/api/v1/message:stream"
    
    # Simulate actual heart rate data
    message_text = """Heart Rate Monitor Update:

I've collected 10 heart rate measurements:
- Average BPM: 71.4
- Range: 60.0 - 84.0 BPM
- Standard Deviation: 9.8
- Time Range: 2026-01-18 06:21:04 to 2026-01-18 06:21:09

Raw data points:
78.0, 78.0, 72.0, 84.0, 78.0, 84.0, 60.0, 60.0, 60.0, 60.0

Please analyze this heart rate data and provide health insights."""
    
    payload = {
        "jsonrpc": "2.0",
        "id": f"hr-test-{datetime.now().strftime('%Y%m%d%H%M%S')}",
        "method": "message/stream",
        "params": {
            "message": {
                "messageId": f"hr-test-{datetime.now().strftime('%Y%m%d%H%M%S')}",
                "role": "user",
                "kind": "message",
                "parts": [
                    {
                        "kind": "text",
                        "text": message_text
                    }
                ],
                "metadata": {
                    "agent_name": "OrchestratorAgent"
                }
            }
        }
    }
    
    print("Testing Heart Rate Integration with ENHANCED parsing...")
    print("=" * 70)
    
    # Step 1: Submit task
    response = requests.post(url, json=payload, headers={"Content-Type": "application/json"}, timeout=10)
    
    if response.status_code != 200:
        print(f"❌ Failed: {response.status_code}")
        return
    
    task_id = response.json().get("result", {}).get("id")
    print(f"✅ Task ID: {task_id}")
    
    # Step 2: Subscribe with ENHANCED parsing logic
    sse_url = f"http://localhost:8000/api/v1/sse/subscribe/{task_id}"
    print(f"\nSubscribing to: {sse_url}")
    print("=" * 70)
    print("🏥 ORCHESTRATOR ANALYSIS")
    print("=" * 70)
    
    sse_response = requests.get(sse_url, stream=True, timeout=60)
    
    if sse_response.status_code == 200:
        full_response = ""
        for line in sse_response.iter_lines():
            if line:
                try:
                    decoded_line = line.decode('utf-8')
                    if decoded_line.startswith("data: "):
                        data_str = decoded_line[6:]
                        data = json.loads(data_str)
                        
                        # ENHANCED: Extract text content from different event types
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
                                    # Direct text content
                                    if part.get('kind') == 'text':
                                        text = part['text']
                                        print(text, end="", flush=True)
                                        full_response += text
                                    
                                    # Data parts (status updates, tool invocations, LLM responses)
                                    elif part.get('kind') == 'data' and 'data' in part:
                                        data_content = part['data']
                                        
                                        # Status update text
                                        if data_content.get('type') == 'agent_progress_update':
                                            status_text = data_content.get('status_text', '')
                                            if status_text:
                                                print(f"\n[Status] {status_text}\n", flush=True)
                                                full_response += f"\n{status_text}\n"
                                        
                                        # LLM response content
                                        elif data_content.get('type') == 'llm_response':
                                            llm_data = data_content.get('data', {})
                                            content = llm_data.get('content', {})
                                            content_parts = content.get('parts', [])
                                            
                                            for content_part in content_parts:
                                                # Extract text from LLM response parts
                                                if 'text' in content_part:
                                                    text = content_part['text']
                                                    # Skip status_update embeds (already shown)
                                                    if not text.startswith('«status_update:'):
                                                        print(text, end="", flush=True)
                                                        full_response += text
                            
                            # Check for final task response
                            if result.get('kind') == 'task':
                                break
                except Exception as e:
                    log.debug(f"Error parsing SSE line: {e}")
        
        print("\n" + "=" * 70)
        print(f"\n✅ Received {len(full_response)} characters of response")
        if full_response:
            print(f"✅ SUCCESS - Analysis extracted and displayed!")
        else:
            print(f"⚠️  No content received")
    else:
        print(f"❌ SSE failed: {sse_response.status_code}")

if __name__ == "__main__":
    test_heart_rate_integration()
