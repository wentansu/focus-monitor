#!/usr/bin/env python3
"""
Test script to verify the heart rate monitor can communicate with the Solace WebUI API.
This sends a test message without requiring a webcam.
"""

import requests
import json
from datetime import datetime

def test_webui_connection():
    """Test connection to the WebUI API endpoint."""
    
    url = "http://localhost:8000/api/v1/message:stream"
    
    # Create a test message similar to what the heart rate monitor sends
    payload = {
        "jsonrpc": "2.0",
        "id": f"test-{datetime.now().strftime('%Y%m%d%H%M%S')}",
        "method": "message/stream",
        "params": {
            "message": {
                "role": "user",
                "kind": "message",
                "parts": [
                    {
                        "kind": "text",
                        "text": """Heart Rate Monitor Integration Test

This is a test message from the heart rate monitor integration.

Test data:
- Average BPM: 75.0
- Range: 72.0 - 78.0 BPM
- Standard Deviation: 2.1

If you're seeing this message, the integration is working correctly!"""
                    }
                ],
                "metadata": {}
            }
        }
    }
    
    print("Testing connection to Solace WebUI API...")
    print(f"URL: {url}")
    print(f"Payload: {json.dumps(payload, indent=2)}")
    print("\nSending request...")
    
    try:
        response = requests.post(
            url,
            json=payload,
            headers={"Content-Type": "application/json"},
            timeout=10
        )
        
        print(f"\nResponse Status: {response.status_code}")
        print(f"Response Headers: {dict(response.headers)}")
        
        if response.status_code == 200:
            print("\n✅ SUCCESS! The WebUI API is reachable and accepting messages.")
            print("\nCheck the WebUI at http://localhost:8000 to see your test message.")
            return True
        else:
            print(f"\n❌ FAILED with status code {response.status_code}")
            print(f"Response body: {response.text}")
            return False
            
    except requests.exceptions.ConnectionError:
        print("\n❌ CONNECTION FAILED - Cannot reach the WebUI API")
        print("\nPossible causes:")
        print("1. Solace Agent Mesh is not running")
        print("2. WebUI is not running on port 8000")
        print("\nTo fix:")
        print("  cd /Users/stevensu/solace-agent-mesh-hackathon-quickstart")
        print("  uv sync")
        print("  uv run sam run configs/")
        return False
        
    except Exception as e:
        print(f"\n❌ UNEXPECTED ERROR: {e}")
        return False

if __name__ == "__main__":
    success = test_webui_connection()
    exit(0 if success else 1)
