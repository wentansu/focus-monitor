# Using Metrics with SmartSpectra C++ SDK {#metrics_usage}

The SmartSpectra C++ SDK provides access to physiological metrics through the protobuf-based metrics system defined in `@edge/modules/messages/metrics.proto`. This document explains how to use and access these metrics in your C++ applications.

## Table of Contents

- [Metrics Overview](#metrics-overview)
- [Accessing Core Metrics](#accessing-core-metrics)
- [Accessing Edge Metrics](#accessing-edge-metrics)
- [Metric Types and Structures](#metric-types-and-structures)
- [Dense Face Landmarks](#dense-face-landmarks)
- [Code Examples](#code-examples)
- [Protobuf Serialization](#protobuf-serialization)

## Metrics Overview

The SmartSpectra SDK provides two types of metrics:

1. **Core Metrics (`MetricsBuffer`)**: High-level physiological measurements computed via the Physiology REST API, including refined pulse rates, breathing rates, and face detection data.
2. **Edge Metrics (`Metrics`)**: Real-time measurements computed locally on the device, providing breathing traces, micromotion data, and face landmarks at frame rate.

**NOTE**: some metrics are not publicly available yet, please contact [Presage Support](mailto:support@presagetech.com) for any inquiries.

Both `MetricsBuffer` and `Metrics` classes are generated from the Protobuf messages. See the <a href="protobuf_reference.html">SmartSpectra Protocol Buffer Reference</a> for comprehensive descriptions of all their available fields.

## Accessing Core Metrics

Core metrics are obtained through the `OnCoreMetricsOutput` callback and contain refined physiological data from the cloud-based Physiology API.

### Setting Up the Core Metrics Callback

```cpp
#include <physiology/modules/messages/metrics.h>

MP_RETURN_IF_ERROR(container.SetOnCoreMetricsOutput([](const presage::physiology::MetricsBuffer& metrics, int64_t timestamp_microseconds) {
    LOG(INFO) << "Received core metrics: " << metrics;

    // Access pulse data
    if (metrics.has_pulse()) {
        for (const auto& rate : metrics.pulse().rate()) {
            std::cout << "Pulse rate: " << rate.value()
                      << " bpm, time: " << rate.time()
                      << "s, confidence: " << rate.confidence() << std::endl;
        }

        // Access pulse trace
        for (const auto& trace_point : metrics.pulse().trace()) {
            std::cout << "Pulse trace value: " << trace_point.value()
                      << " at time: " << trace_point.time() << "s" << std::endl;
        }
    }

    // Access breathing data
    if (metrics.has_breathing()) {
        for (const auto& rate : metrics.breathing().rate()) {
            std::cout << "Breathing rate: " << rate.value()
                      << " breaths/min, confidence: " << rate.confidence() << std::endl;
        }
    }

    return absl::OkStatus();
}));
```

## Accessing Edge Metrics

Edge metrics provide real-time data computed locally on the device. To enable edge metrics, set `enable_edge_metrics` to `true` in your settings.

### Setting Up the Edge Metrics Callback

```cpp
// Enable edge metrics in settings
settings.enable_edge_metrics = true;

MP_RETURN_IF_ERROR(container.SetOnEdgeMetricsOutput([](const presage::physiology::Metrics& metrics) {
    LOG(INFO) << "Computed edge metrics: " << metrics;

    // Access breathing traces
    if (metrics.has_breathing()) {
        if (!metrics.breathing().upper_trace().empty()) {
            const auto& latest_upper = *metrics.breathing().upper_trace().rbegin();
            std::cout << "Upper breathing trace: " << latest_upper.value()
                      << " at time: " << latest_upper.time() << "s" << std::endl;
        }

        if (!metrics.breathing().lower_trace().empty()) {
            const auto& latest_lower = *metrics.breathing().lower_trace().rbegin();
            std::cout << "Lower breathing trace: " << latest_lower.value()
                      << " at time: " << latest_lower.time() << "s" << std::endl;
        }
    }

    return absl::OkStatus();
}));
```

## Metric Types and Structures

### Measurement Types

The metrics system uses several fundamental measurement types:

**`Measurement`** - Basic measurement with time and value:

```cpp
struct Measurement {
    float time();     // Time in seconds
    float value();    // Measurement value
    bool stable();    // Whether measurement is stable
}
```

**`MeasurementWithConfidence`** - Measurement with confidence score:

```cpp
struct MeasurementWithConfidence {
    float time();        // Time in seconds
    float value();       // Measurement value
    bool stable();       // Whether measurement is stable
    float confidence();  // Confidence level (0.0 - 1.0)
}
```

**`DetectionStatus`** - Used for binary detection events:

```cpp
struct DetectionStatus {
    float time();       // Time in seconds
    bool detected();    // Whether feature is detected
    bool stable();      // Whether detection is stable
}
```

### Core Metric Structures

**`MetricsBuffer`** - Main container for core metrics:

```cpp
struct MetricsBuffer {
    Pulse pulse();                    // Pulse-related data
    Breathing breathing();            // Breathing-related data
    BloodPressure blood_pressure();   // Blood pressure data
    Face face();                      // Face detection data
    Metadata metadata();              // Measurement metadata
}
```

**`Pulse`** - Pulse rate and trace data:

```cpp
struct Pulse {
    repeated MeasurementWithConfidence rate();  // Pulse rate measurements
    repeated Measurement trace();               // Pulse pleth waveform
    repeated Measurement pulse_respiration_quotient();  // Pulse-respiration coupling
    Strict strict();                           // Strict pulse value
}
```

**`Breathing`** - Comprehensive breathing metrics:

```cpp
struct Breathing {
    repeated MeasurementWithConfidence rate();              // Breathing rate
    repeated Measurement upper_trace();                     // Upper chest trace
    repeated Measurement lower_trace();                     // Lower abdomen trace
    repeated Measurement amplitude();                       // Breathing amplitude
    repeated DetectionStatus apnea();                       // Apnea detection
    repeated Measurement respiratory_line_length();         // Respiratory line length
    repeated Measurement baseline();                        // Breathing baseline
    repeated Measurement inhale_exhale_ratio();            // I:E ratio
    Strict strict();                                       // Strict breathing value
}
```

**`Face`** - Face detection and landmark data:

```cpp
struct Face {
    repeated DetectionStatus blinking();  // Blink detection
    repeated DetectionStatus talking();   // Speech detection
    repeated Landmarks landmarks();       // Facial landmarks
}
```

### Edge Metric Structures

**`Metrics`** - Container for edge-computed metrics:

```cpp
struct Metrics {
    Breathing breathing();        // Real-time breathing data
    MicroMotion micromotion();   // Micromotion measurements
    Eda eda();                   // EDA measurements
    Face face();                 // Face landmarks
}
```

## Dense Face Landmarks

Dense face landmarks provide detailed facial feature points for advanced analysis. To access face landmarks:

### Enabling Dense Face Landmarks

Enable dense face landmarks in your application settings:

```cpp
// Enable dense facemesh points in settings
settings.enable_dense_facemesh_points = true;
```

### Accessing Face Landmarks from Edge Metrics

Face landmarks are available through the edge metrics system:

```cpp
MP_RETURN_IF_ERROR(container.SetOnEdgeMetricsOutput([](const presage::physiology::Metrics& metrics) {
    // Access face landmarks
    if (metrics.has_face() && !metrics.face().landmarks().empty()) {
        const auto& latest_landmarks = *metrics.face().landmarks().rbegin();

        std::cout << "Face landmarks at time: " << latest_landmarks.time() << "s" << std::endl;
        std::cout << "Landmark count: " << latest_landmarks.value_size() << std::endl;
        std::cout << "Stable: " << (latest_landmarks.stable() ? "yes" : "no") << std::endl;
        std::cout << "Reset: " << (latest_landmarks.reset() ? "yes" : "no") << std::endl;

        // Process individual landmark points
        for (int i = 0; i < latest_landmarks.value_size(); ++i) {
            const auto& point = latest_landmarks.value(i);
            std::cout << "Landmark " << i << ": (" << point.x() << ", " << point.y() << ")" << std::endl;
        }
    }

    return absl::OkStatus();
}));
```

### Face Landmarks Structure

```cpp
struct Landmarks {
    float time();                          // Timestamp in seconds
    repeated Point2dFloat value();         // Array of 2D landmark points
    bool stable();                         // Whether landmarks are stable
    bool reset();                          // Whether landmark set was reset
}

struct Point2dFloat {
    float x();  // X coordinate
    float y();  // Y coordinate
}
```

## Code Examples

### Complete Metrics Processing Example

See the [rest_continuous_example](../samples/rest_continuous_example/main.cc) for a comprehensive example of:

- Setting up both core and edge metrics callbacks
- Processing real-time breathing traces
- Handling micromotion data
- Plotting metrics in real-time

### Minimal Metrics Usage Example

See the [minimal_rest_spot_example](../samples/minimal_rest_spot_example/main.cc) for a simple example of:

- Basic core metrics processing
- JSON serialization of metrics
- Essential callback setup

## Protobuf Serialization

You can serialize metrics to binary format for storage or transmission:

### Serializing Metrics to Binary

```cpp
#include <google/protobuf/util/json_util.h>

// Serialize to binary data
std::string binary_data;
if (metrics.SerializeToString(&binary_data)) {
    // Save binary_data to file or send over network
    std::ofstream file("metrics.bin", std::ios::binary);
    file.write(binary_data.data(), binary_data.size());
    file.close();
}
```

### Converting Metrics to JSON

```cpp
// Convert to JSON string
std::string json_string;
google::protobuf::util::JsonPrintOptions options;
options.add_whitespace = true;
options.preserve_proto_field_names = true;

auto status = google::protobuf::util::MessageToJsonString(metrics, &json_string, options);
if (status.ok()) {
    std::cout << "Metrics JSON: " << json_string << std::endl;
}
```

### Deserializing Metrics from Binary

```cpp
// Load from binary data
std::ifstream file("metrics.bin", std::ios::binary);
std::string binary_data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

presage::physiology::MetricsBuffer loaded_metrics;
if (loaded_metrics.ParseFromString(binary_data)) {
    // Use loaded_metrics
    std::cout << "Loaded metrics successfully" << std::endl;
}
```

## Command Line Options

The following command line flags control metrics behavior:

- `--enable_edge_metrics`: Enable real-time edge metrics computation
- `--enable_dense_facemesh_points`: Enable dense facial landmark detection
- `--enable_micromotion`: Enable micromotion analysis (requires thighs/knees in view)
- `--enable_phasic_bp`: Enable phasic blood pressure computation
- `--verbosity`: Set verbosity level for metrics logging (1-4)

## Best Practices

1. **Performance**: Edge metrics are computed at frame rate, so keep processing lightweight in the callback
2. **Stability**: Check the `stable()` flag before using measurement values for critical applications
3. **Face Landmarks**: Monitor the `reset()` flag to detect when landmark tracking has been reinitialized
4. **Memory Management**: Process metrics data promptly in callbacks to avoid memory buildup
5. **Error Handling**: Always check `absl::Status` return values from SDK methods

## Troubleshooting

- **No Edge Metrics**: Ensure `enable_edge_metrics = true` in your settings
- **No Face Landmarks**: Enable `enable_dense_facemesh_points = true`
- **Empty Measurements**: Check that face is properly detected and in camera view
- Some metrics are not publicly available yet, please contact [Presage Support](mailto:support@presagetech.com) for any inquiries

For additional support, contact [support@presagetech.com](mailto:support@presagetech.com) or [submit a GitHub issue](https://github.com/Presage-Security/SmartSpectra/issues).
