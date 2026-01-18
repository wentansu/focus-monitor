"""
Webcam Heart Rate Monitor
Gilad Oved
December 2018

Modified to integrate with Solace Agent Mesh
"""

import numpy as np
import cv2
import sys
import requests
import json
import os
import subprocess
from datetime import datetime
from typing import List, Dict, Any
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
log = logging.getLogger(__name__)

# Helper Methods
def buildGauss(frame, levels):
    pyramid = [frame]
    for level in range(levels):
        frame = cv2.pyrDown(frame)
        pyramid.append(frame)
    return pyramid
def reconstructFrame(pyramid, index, levels):
    filteredFrame = pyramid[index]
    for level in range(levels):
        filteredFrame = cv2.pyrUp(filteredFrame)
    filteredFrame = filteredFrame[:videoHeight, :videoWidth]
    return filteredFrame

# Solace Integration Configuration
WEBUI_API_URL = os.getenv("WEBUI_API_URL", "http://localhost:8000/api/v1/message:stream")
BPM_COLLECTION_SIZE = 10  # Send data every 10 readings

def send_bpm_data_to_orchestrator(bpm_readings: List[Dict[str, Any]]) -> bool:
    """
    Send collected BPM data to the Solace orchestrator agent.
    
    Args:
        bpm_readings: List of BPM reading dictionaries with timestamp and value
        
    Returns:
        True if successful, False otherwise
    """
    log_id = "[HeartRateMonitor:send_bpm_data]"
    
    if not bpm_readings:
        log.warning(f"{log_id} No readings to send")
        return False
    
    # Calculate statistics
    bpm_values = [r['bpm'] for r in bpm_readings]
    avg_bpm = np.mean(bpm_values)
    min_bpm = np.min(bpm_values)
    max_bpm = np.max(bpm_values)
    std_bpm = np.std(bpm_values)
    
    # Create message for orchestrator
    message_text = f"""Heart Rate Monitor Update:

I've collected {len(bpm_readings)} heart rate measurements:
- Average BPM: {avg_bpm:.1f}
- Range: {min_bpm:.1f} - {max_bpm:.1f} BPM
- Standard Deviation: {std_bpm:.1f}
- Time Range: {bpm_readings[0]['timestamp']} to {bpm_readings[-1]['timestamp']}

Raw data points:
{', '.join([f"{r['bpm']:.1f}" for r in bpm_readings])}

Please analyze this heart rate data and provide health insights."""
    
    # Format request according to SAM WebUI API specification
    message_id = f"hr-monitor-{datetime.now().strftime('%Y%m%d%H%M%S%f')}"
    
    payload = {
        "jsonrpc": "2.0",
        "id": message_id,
        "method": "message/stream",
        "params": {
            "message": {
                "messageId": message_id,
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
    
    try:
        log.info(f"{log_id} Sending {len(bpm_readings)} readings to orchestrator")
        log.debug(f"{log_id} Avg BPM: {avg_bpm:.1f}, Range: {min_bpm:.1f}-{max_bpm:.1f}")
        
        # Step 1: Submit task (non-streaming)
        response = requests.post(
            WEBUI_API_URL,
            json=payload,
            headers={"Content-Type": "application/json"},
            timeout=30
        )
        
        if response.status_code == 200:
            response_json = response.json()
            task_id = response_json.get("result", {}).get("id")
            
            if not task_id:
                log.error(f"{log_id} No task ID returned in response")
                return False
                
            log.info(f"{log_id} Task submitted successfully. Task ID: {task_id}")
            
            # Step 2: Subscribe to SSE stream
            sse_url = WEBUI_API_URL.replace("/api/v1/message:stream", f"/api/v1/sse/subscribe/{task_id}")
            # Correct logic if the URL structure is different, but based on default this works.
            # Safer to construct from base if possible, but let's assume standard path for now or split string.
            # Actually, let's just make it robust.
            base_url = WEBUI_API_URL.split("/api/v1/")[0]
            sse_url = f"{base_url}/api/v1/sse/subscribe/{task_id}"
            
            log.info(f"{log_id} Subscribing to SSE stream at {sse_url}")
            
            sse_response = requests.get(sse_url, stream=True, timeout=30)
            
            if sse_response.status_code == 200:
                print("\n" + "="*50)
                print("🏥 ORCHESTRATOR ANALYSIS")
                print("="*50)
                
                full_response = ""
                for line in sse_response.iter_lines():
                    if line:
                        try:
                            decoded_line = line.decode('utf-8')
                            if decoded_line.startswith("data: "):
                                data_str = decoded_line[6:]
                                data = json.loads(data_str)
                                
                                # Extract text content from different event types
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
                            log.debug(f"{log_id} Error parsing SSE line: {e}")
                print("\n" + "="*50 + "\n")
                
                # Check if response indicates distraction (starts with YES)
                if full_response.strip().count("YES") > 0:
                    log.warning(f"{log_id} DISTRACTION DETECTED - Triggering focus overlay")
                    try:
                        overlay_script = os.path.join(os.path.dirname(__file__), "focus_overlay.py")
                        subprocess.Popen(
                            [sys.executable, overlay_script, "--overlay"],
                            close_fds=True,
                            env=os.environ.copy()
                        )
                        log.info(f"{log_id} Focus overlay triggered successfully")
                    except Exception as overlay_error:
                        log.error(f"{log_id} Failed to trigger overlay: {overlay_error}")
                
                return True
            else:
                log.error(f"{log_id} SSE Subscription failed: {sse_response.status_code}")
                return False
        else:
            log.error(f"{log_id} Failed with status {response.status_code}: {response.text}")
            return False
            
    except requests.exceptions.RequestException as e:
        log.error(f"{log_id} Request failed: {e}")
        return False
    except Exception as e:
        log.error(f"{log_id} Unexpected error: {e}", exc_info=True)
        return False


# Webcam Parameters
webcam = None
if len(sys.argv) == 2:
    webcam = cv2.VideoCapture(sys.argv[1])
else:
    webcam = cv2.VideoCapture(0)
realWidth = 320
realHeight = 240
videoWidth = 160
videoHeight = 120
videoChannels = 3
videoFrameRate = 15
webcam.set(3, realWidth)
webcam.set(4, realHeight)

# Color Magnification Parameters
levels = 3
alpha = 170
minFrequency = 1.0
maxFrequency = 2.0
bufferSize = 150
bufferIndex = 0

# Output Display Parameters
font = cv2.FONT_HERSHEY_SIMPLEX
loadingTextLocation = (20, 30)
bpmTextLocation = (videoWidth//2 + 5, 30)
fontScale = 1
fontColor = (255,255,255)
lineType = 2
boxColor = (0, 255, 0)
boxWeight = 3

# Initialize Gaussian Pyramid
firstFrame = np.zeros((videoHeight, videoWidth, videoChannels))
firstGauss = buildGauss(firstFrame, levels+1)[levels]
videoGauss = np.zeros((bufferSize, firstGauss.shape[0], firstGauss.shape[1], videoChannels))
fourierTransformAvg = np.zeros((bufferSize))

# Bandpass Filter for Specified Frequencies
frequencies = (1.0*videoFrameRate) * np.arange(bufferSize) / (1.0*bufferSize)
mask = (frequencies >= minFrequency) & (frequencies <= maxFrequency)

# Heart Rate Calculation Variables
bpmCalculationFrequency = 15
bpmBufferIndex = 0
bpmBufferSize = 10
bpmBuffer = np.zeros((bpmBufferSize))

# Solace integration - collect BPM readings to send to orchestrator
bpm_readings_for_orchestrator: List[Dict[str, Any]] = []


i = 0
while (True):
    ret, frame = webcam.read()
    if ret == False:
        break

    detectionFrame = frame[videoHeight//2:realHeight-videoHeight//2, videoWidth//2:realWidth-videoWidth//2, :]

    # Construct Gaussian Pyramid
    videoGauss[bufferIndex] = buildGauss(detectionFrame, levels+1)[levels]
    fourierTransform = np.fft.fft(videoGauss, axis=0)

    # Bandpass Filter
    fourierTransform[mask == False] = 0

    # Grab a Pulse
    if bufferIndex % bpmCalculationFrequency == 0:
        i = i + 1
        for buf in range(bufferSize):
            fourierTransformAvg[buf] = np.real(fourierTransform[buf]).mean()
        hz = frequencies[np.argmax(fourierTransformAvg)]
        bpm = 60.0 * hz
        bpmBuffer[bpmBufferIndex] = bpm
        bpmBufferIndex = (bpmBufferIndex + 1) % bpmBufferSize
        
        # Collect BPM reading for Solace orchestrator
        if i > bpmBufferSize:  # Only after initial buffer is filled
            current_reading = {
                'bpm': float(bpm),
                'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
                'reading_number': len(bpm_readings_for_orchestrator) + 1
            }
            bpm_readings_for_orchestrator.append(current_reading)
            
            log.info(f"[HeartRateMonitor] BPM reading #{current_reading['reading_number']}: {bpm:.1f}")
            
            # Send to orchestrator every 10 readings
            if len(bpm_readings_for_orchestrator) >= BPM_COLLECTION_SIZE:
                log.info(f"[HeartRateMonitor] Collected {len(bpm_readings_for_orchestrator)} readings, sending to orchestrator...")
                success = send_bpm_data_to_orchestrator(bpm_readings_for_orchestrator)
                
                if success:
                    # Clear the collected readings after successful send
                    bpm_readings_for_orchestrator.clear()
                    log.info("[HeartRateMonitor] Data sent successfully, cleared collection buffer")
                else:
                    log.warning("[HeartRateMonitor] Failed to send data, will retry with next batch")
                    # Keep last 5 readings and continue collecting
                    bpm_readings_for_orchestrator = bpm_readings_for_orchestrator[-5:]


    # Amplify
    filtered = np.real(np.fft.ifft(fourierTransform, axis=0))
    filtered = filtered * alpha

    # Reconstruct Resulting Frame
    filteredFrame = reconstructFrame(filtered, bufferIndex, levels)
    outputFrame = detectionFrame + filteredFrame
    outputFrame = cv2.convertScaleAbs(outputFrame)

    bufferIndex = (bufferIndex + 1) % bufferSize

    frame[videoHeight//2:realHeight-videoHeight//2, videoWidth//2:realWidth-videoWidth//2, :] = outputFrame
    cv2.rectangle(frame, (videoWidth//2 , videoHeight//2), (realWidth-videoWidth//2, realHeight-videoHeight//2), boxColor, boxWeight)
    if i > bpmBufferSize:
        cv2.putText(frame, "BPM: %d" % bpmBuffer.mean(), bpmTextLocation, font, fontScale, fontColor, lineType)
    else:
        cv2.putText(frame, "Calculating BPM...", loadingTextLocation, font, fontScale, fontColor, lineType)

    if len(sys.argv) != 2:
        cv2.imshow("Webcam Heart Rate Monitor", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

webcam.release()
cv2.destroyAllWindows()
