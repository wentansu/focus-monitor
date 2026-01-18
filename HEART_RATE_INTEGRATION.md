# Heart Rate Monitor - Solace Agent Mesh Integration

This integration connects the webcam heart rate monitor (`src/main.py`) with the Solace Agent Mesh orchestrator to enable AI-powered health insights based on real-time heart rate data.

## How It Works

The heart rate monitor:
1. Captures video from your webcam
2. Uses signal processing to detect your heart rate (BPM)
3. **Collects measurements every 10 data points**
4. **Sends aggregated data to the Solace orchestrator** for AI analysis
5. Continues monitoring and sending updates automatically

## Architecture

```
Webcam → Heart Rate Monitor → Data Collection → WebUI API → Orchestrator Agent → Health Insights
         (src/main.py)          (every 10 BPM)   (REST API)   (AI Analysis)
```

## Prerequisites

1. **Solace Agent Mesh** running with WebUI on port 8000
2. **Python dependencies** for the heart rate monitor:
   ```bash
   uv pip install opencv-python numpy requests
   ```

## Running the Integration

### Step 1: Start the Solace Agent Mesh

```bash
# Make sure .env is configured (see docs/llm-setup.md)
uv sync
uv run sam run configs/
```

The WebUI should be accessible at `http://localhost:8000`

### Step 2: Run the Heart Rate Monitor

In a separate terminal:

```bash
cd /Users/stevensu/solace-agent-mesh-hackathon-quickstart
uv run python src/main.py
```

### Step 3: Monitor the Integration

**In the heart rate monitor terminal**, you'll see:
```
INFO:__main__:[HeartRateMonitor] BPM reading #1: 72.3
INFO:__main__:[HeartRateMonitor] BPM reading #2: 73.1
...
INFO:__main__:[HeartRateMonitor] BPM reading #10: 71.8
INFO:__main__:[HeartRateMonitor] Collected 10 readings, sending to orchestrator...
INFO:__main__:[HeartRateMonitor:send_bpm_data] Sending 10 readings to orchestrator
INFO:__main__:[HeartRateMonitor:send_bpm_data] Successfully sent data to orchestrator
INFO:__main__:[HeartRateMonitor] Data sent successfully, cleared collection buffer
```

**In the Solace WebUI** (`http://localhost:8000`), you'll see messages like:

> **Heart Rate Monitor Update:**
> 
> I've collected 10 heart rate measurements:
> - Average BPM: 72.5
> - Range: 68.2 - 76.3 BPM
> - Standard Deviation: 2.4
> - Time Range: 2026-01-18 04:30:15 to 2026-01-18 04:32:30
> 
> Raw data points: 72.3, 73.1, 71.5, 68.2, 74.8, 75.2, 76.3, 71.2, 70.8, 71.8
> 
> Please analyze this heart rate data and provide health insights.

The orchestrator will then analyze the data and provide AI-powered insights.

## Configuration

### WebUI API Endpoint

By default, the monitor connects to `http://localhost:8000/api/v1/message:stream`. To change this:

```bash
export WEBUI_API_URL="http://your-server:port/api/v1/message:stream"
uv run python src/main.py
```

### Data Collection Frequency

The monitor sends data every **10 BPM readings**. To change this, edit `src/main.py`:

```python
BPM_COLLECTION_SIZE = 10  # Change to desired number
```

## What Data Is Sent

Each message to the orchestrator includes:

| Field | Description | Example |
|-------|-------------|---------|
| **Number of readings** | How many data points collected | 10 |
| **Average BPM** | Mean heart rate | 72.5 |
| **Range** | Min and max BPM values | 68.2 - 76.3 |
| **Standard Deviation** | Variability measure | 2.4 |
| **Time Range** | Start and end timestamps | 04:30:15 to 04:32:30 |
| **Raw data** | All individual BPM values | 72.3, 73.1, ... |

## Troubleshooting

### "Connection refused" errors

**Problem:** Heart rate monitor can't reach the WebUI API

**Solution:** Make sure SAM is running and accessible:
```bash
curl http://localhost:8000/api/v1/health
# Should return: {"status":"ok"}
```

### No webcam detected

**Problem:** `cv2.VideoCapture(0)` fails

**Solution:** 
- Check webcam permissions in System Settings
- Try specifying a different camera: `uv run python src/main.py 1`
- List available cameras: `ls /dev/video*` (Linux) or check System Information (macOS)

### BPM readings seem inaccurate

**Problem:** Wildly fluctuating or unrealistic BPM values

**Solution:**
- Ensure good lighting on your face
- Stay still while measuring
- The algorithm needs ~10 seconds to stabilize (initial readings are less accurate)
- Adjust camera angle to capture more of your forehead/face

### Messages not appearing in WebUI

**Problem:** Data sent successfully but not visible in chat

**Solution:**
- Check the SAM logs for routing errors
- Verify orchestrator agent is running: check `configs/orchestrator.yaml`
- Open WebUI in browser and check the conversation thread

## Implementation Details

### Message Format

The heart rate monitor sends JSON-RPC 2.0 formatted requests:

```json
{
  "jsonrpc": "2.0",
  "id": "hr-monitor-20260118043015",
  "method": "message/stream",
  "params": {
    "message": {
      "role": "user",
      "kind": "message",
      "parts": [
        {
          "kind": "text",
          "text": "Heart Rate Monitor Update: ..."
        }
      ],
      "metadata": {}
    }
  }
}
```

### Error Handling

- **Network failures**: Logged and retried with the next batch
- **Partial failures**: Keeps last 5 readings to avoid data loss
- **API errors**: Logs full response for debugging

### Logging

Set log level for more detailed output:

```python
# In src/main.py
logging.basicConfig(level=logging.DEBUG)  # Change INFO to DEBUG
```

## Future Enhancements

Potential improvements to this integration:

- [ ] Create a dedicated Health Analysis agent for specialized medical insights
- [ ] Add configurable alerting (e.g., BPM > 100 for extended period)
- [ ] Store historical data in artifacts for trend analysis
- [ ] Support for multiple biometric data sources (e.g., SpO2, temperature)
- [ ] Real-time visualization dashboard
- [ ] Integration with wearable devices (Fitbit, Apple Watch)

## Related Files

- **Heart rate monitor**: [`src/main.py`](file:///Users/stevensu/solace-agent-mesh-hackathon-quickstart/src/main.py)
- **WebUI configuration**: [`configs/webui.yaml`](file:///Users/stevensu/solace-agent-mesh-hackathon-quickstart/configs/webui.yaml)
- **Orchestrator configuration**: [`configs/orchestrator.yaml`](file:///Users/stevensu/solace-agent-mesh-hackathon-quickstart/configs/orchestrator.yaml)
