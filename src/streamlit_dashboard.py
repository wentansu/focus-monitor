"""
Streamlit Dashboard for Heart Rate Monitor
Displays real-time analysis responses from the Solace Agent Mesh
"""

import streamlit as st
import json
import time
import os
import signal
import subprocess
from pathlib import Path
from datetime import datetime

# Configuration
RESPONSE_FILE = Path(__file__).parent / "latest_response.json"
REFRESH_INTERVAL = 1  # seconds

def shutdown_all_processes():
    """Shutdown all related processes: SAM, main.py, and this Streamlit app."""
    try:
        # Kill SAM process
        subprocess.run(["pkill", "-f", "sam run configs"], capture_output=True)
        # Kill main.py (heart rate monitor)
        subprocess.run(["pkill", "-f", "python3 src/main.py"], capture_output=True)
        subprocess.run(["pkill", "-f", "python src/main.py"], capture_output=True)
        # Kill transfer.py
        subprocess.run(["pkill", "-f", "python3 transfer.py"], capture_output=True)
        subprocess.run(["pkill", "-f", "python transfer.py"], capture_output=True)
        # Exit Streamlit
        os.kill(os.getpid(), signal.SIGTERM)
    except Exception as e:
        st.error(f"Error during shutdown: {e}")

# Page config
st.set_page_config(
    page_title="Heart Rate Monitor Dashboard",
    page_icon="🏥",
    layout="wide",
    initial_sidebar_state="collapsed"
)

# Custom CSS
st.markdown("""
<style>
    .main-header {
        font-size: 2.5rem;
        font-weight: bold;
        color: #FF4B4B;
        text-align: center;
        margin-bottom: 1rem;
    }
    .status-box {
        padding: 1rem;
        border-radius: 0.5rem;
        margin: 1rem 0;
    }
    .status-yes {
        background-color: #FFE5E5;
        border-left: 5px solid #FF4B4B;
    }
    .status-no {
        background-color: #E5F5E5;
        border-left: 5px solid #4BFF4B;
    }
    .timestamp {
        color: #666;
        font-size: 0.9rem;
        font-style: italic;
    }
</style>
""", unsafe_allow_html=True)

# Header
st.markdown('<div class="main-header">🏥 Heart Rate Monitor Dashboard</div>', unsafe_allow_html=True)

# Exit button at top right
col1, col2, col3 = st.columns([6, 1, 1])
with col3:
    if st.button("🛑 Exit", type="primary"):
        st.warning("Shutting down...")
        shutdown_all_processes()

# Create placeholder for dynamic content
status_placeholder = st.empty()
response_placeholder = st.empty()
metadata_placeholder = st.empty()

def load_latest_response():
    """Load the latest response from the JSON file."""
    if RESPONSE_FILE.exists():
        try:
            with open(RESPONSE_FILE, 'r') as f:
                return json.load(f)
        except (json.JSONDecodeError, FileNotFoundError):
            return None
    return None

def display_response(data):
    """Display the response data in the dashboard."""
    if not data:
        status_placeholder.info("⏳ Waiting for heart rate data...")
        return
    
    # Extract data
    timestamp = data.get('timestamp', 'Unknown')
    response_text = data.get('response', '')
    distracted = data.get('distracted', False)
    bpm_stats = data.get('bpm_stats', {})
    
    # Status indicator
    if distracted:
        status_placeholder.markdown(
            f'<div class="status-box status-yes"><h2>⚠️ DISTRACTION DETECTED</h2></div>',
            unsafe_allow_html=True
        )
    else:
        status_placeholder.markdown(
            f'<div class="status-box status-no"><h2>✅ FOCUSED</h2></div>',
            unsafe_allow_html=True
        )
    
    # Response content
    with response_placeholder.container():
        st.markdown("### 📊 Analysis")
        st.markdown(response_text)
    
    # Metadata
    with metadata_placeholder.container():
        col1, col2, col3, col4 = st.columns(4)
        
        with col1:
            st.metric("Average BPM", f"{bpm_stats.get('avg', 0):.1f}")
        
        with col2:
            st.metric("Min BPM", f"{bpm_stats.get('min', 0):.1f}")
        
        with col3:
            st.metric("Max BPM", f"{bpm_stats.get('max', 0):.1f}")
        
        with col4:
            st.metric("Std Dev", f"{bpm_stats.get('std', 0):.1f}")
        
        st.markdown(f'<p class="timestamp">Last updated: {timestamp}</p>', unsafe_allow_html=True)

# Auto-refresh only when file changes
if 'last_modified' not in st.session_state:
    st.session_state.last_modified = None

# Load and display
data = load_latest_response()
display_response(data)

# Check if file was modified
current_modified = None
if RESPONSE_FILE.exists():
    current_modified = RESPONSE_FILE.stat().st_mtime

# Only refresh if file changed or doesn't exist yet
if current_modified != st.session_state.last_modified:
    st.session_state.last_modified = current_modified
    time.sleep(REFRESH_INTERVAL)
    st.rerun()
else:
    # Wait longer before checking again when no changes
    time.sleep(10)
    st.rerun()
