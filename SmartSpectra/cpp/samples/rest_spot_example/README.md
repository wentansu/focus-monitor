# REST Spot Example

This example demonstrates spot physiological measurements with file output using the SmartSpectra C++ SDK and REST API integration.

## Overview

The REST spot example shows how to:

- Perform single spot measurements (30 seconds by default)
- Use REST API integration for physiological processing
- Process video from camera or file input
- Output results to both console and files
- Handle comprehensive command-line configuration

## Key Features

- **Spot measurement mode** - Single measurement session rather than continuous monitoring
- **REST API integration** - Uses HTTP REST calls to the Physiology API
- **File output** - Saves results to disk for analysis
- **Flexible input** - Supports both camera and video file sources
- **Comprehensive CLI** - Extensive command-line options for configuration

## Usage

```bash
# Build the example (from smartspectra/cpp directory)
cmake --build build --target rest_spot_example

# Run with camera input (30-second measurement)
./build/samples/rest_spot_example/rest_spot_example --also_log_to_stderr --camera_device_index=0 --auto_lock=false --api_key=YOUR_API_KEY_HERE

# Run with video file input
./build/samples/rest_spot_example/rest_spot_example --also_log_to_stderr --input_video_path=/path/to/video.mp4 --api_key=YOUR_API_KEY_HERE

# Run with custom measurement duration (60 seconds)
./build/samples/rest_spot_example/rest_spot_example --also_log_to_stderr --camera_device_index=0 --spot_duration_s=60 --api_key=YOUR_API_KEY_HERE
```

## Configuration

Key configuration options include:

- `--api_key`: Your Physiology REST API key (required)
- `--spot_duration_s`: Duration of measurement in seconds (default: 30)
- `--camera_device_index`: Camera to use for input (default: 0)
- `--input_video_path`: Path to video file for processing
- `--output_file_path`: Path for output files

For complete options, run:

```bash
./rest_spot_example --help=main
```

## Output

The example generates:

1. **Console output** - Real-time logging and final results
2. **JSON files** - Structured physiological data
3. **Log files** - Detailed processing information
4. **Status files** - Processing status indicators

## Code Structure

The example demonstrates:

1. **Command-line Processing** - Comprehensive argument parsing with Abseil flags
2. **Settings Configuration** - Spot measurement and REST integration setup
3. **Container Management** - SmartSpectra processing container lifecycle
4. **File I/O Operations** - Reading input and writing output files
5. **Error Handling** - Robust error checking and status reporting
6. **Data Processing** - Physiological metrics extraction and formatting

This example is ideal for batch processing applications, research workflows, or scenarios where you need to analyze pre-recorded video content for physiological data.
