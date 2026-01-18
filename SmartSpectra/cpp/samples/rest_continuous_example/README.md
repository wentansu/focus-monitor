# REST Continuous Example

This example demonstrates continuous physiological monitoring with real-time visualization using the SmartSpectra C++ SDK and REST API integration.

## Overview

The REST continuous example shows how to:

- Set up continuous monitoring from a camera or video file
- Use REST API integration for physiological processing
- Display real-time vitals data overlaid on the video stream
- Handle keyboard controls for interactive operation
- Plot physiological data in real-time using OpenCV

## Key Features

- **Continuous monitoring** - Real-time measurement rather than single spot readings
- **REST API integration** - Uses HTTP REST calls to the Physiology API
- **Real-time visualization** - Live HUD display with vitals data
- **Interactive controls** - Keyboard shortcuts for recording control
- **Data plotting** - Real-time graphs of heart rate and breathing patterns

## Usage

```bash
# Build the example (from smartspectra/cpp directory)
cmake --build build --target rest_continuous_example

# Run with camera input
./build/samples/rest_continuous_example/rest_continuous_example --also_log_to_stderr --camera_device_index=0 --auto_lock=false --api_key=YOUR_API_KEY_HERE

# Run with video file input
./build/samples/rest_continuous_example/rest_continuous_example --also_log_to_stderr --input_video_path=/path/to/video.mp4 --api_key=YOUR_API_KEY_HERE
```

## Configuration

Before running, make sure to:

1. Set your API key via the `--api_key` parameter
2. Configure camera or video file input
3. Optionally adjust visualization and processing parameters

## Keyboard Controls

During execution, use these keyboard shortcuts:

- `q` or `ESC`: Exit the application
- `s`: Start/stop recording data (streaming mode only)  
- `e`: Lock/unlock camera exposure (streaming mode only)
- `-` and `=`: Decrease/increase exposure (when locked)

## Code Structure

The example demonstrates:

1. **Settings Configuration** - Sets up continuous measurement and REST integration
2. **Container Setup** - Creates and configures the SmartSpectra processing container
3. **Callback Registration** - Sets up handlers for metrics and video output
4. **Real-time Processing** - Continuous measurement with live feedback
5. **Visualization** - HUD overlay with physiological data and trends
6. **Interactive Control** - Keyboard input handling for user interaction

This example is ideal for applications requiring continuous monitoring with immediate visual feedback, such as fitness applications, health monitoring systems, or research tools.
