# Minimal REST Spot Example

This example demonstrates the simplest possible implementation of the SmartSpectra C++ SDK for spot measurements using the REST API integration.

## Overview

The minimal REST spot example shows how to:

- Configure basic spot measurement settings
- Set up REST API integration
- Perform a single 30-second physiological measurement
- Handle basic callbacks for metrics output

## Key Features

- **Minimal code footprint** - Demonstrates the absolute minimum required to get started
- **REST API integration** - Uses HTTP REST calls instead of gRPC
- **Spot measurement mode** - Single measurement rather than continuous monitoring
- **Basic error handling** - Simple logging and error reporting

## Usage

```bash
# Build the example (from smartspectra/cpp directory)
cmake --build build --target minimal_rest_spot_example

# Run with your API key
./build/samples/minimal_rest_spot_example/minimal_rest_spot_example
```

## Configuration

Before running, make sure to set your API key in the source code:

```cpp
settings.integration.api_key = "YOUR_API_KEY_HERE";
```

## Code Structure

The example consists of a single `main.cc` file that:

1. **Initializes logging** - Sets up Google logging for debug output
2. **Configures settings** - Sets up spot measurement and REST integration parameters  
3. **Creates container** - Instantiates the SmartSpectra processing container
4. **Starts measurement** - Begins the 30-second spot measurement
5. **Handles callbacks** - Processes physiological metrics as they're generated
6. **Cleanup** - Properly shuts down the measurement system

This example serves as the starting point for developers who want to integrate basic physiological measurements into their applications with minimal overhead.
