# ``SmartSpectraSwiftSDK``

A Swift SDK for measuring physiology metrics using mobile device's camera.

## Overview

The SmartSpectra SDK provides a comprehensive solution for capturing and analyzing physiological data including pulse rate, breathing patterns, and other vital signs using computer vision and signal processing techniques.

The SDK offers both a complete UI solution with ``SmartSpectraView`` and configurable image output for custom implementations, making it flexible for various app architectures and use cases.

## Key Features

- **Cardiac Waveform**
  Real-time pulse pleth waveform supporting calculation of pulse rate and heart rate variability.

- **Breathing Waveform**
  Real-time breathing waveform supporting biofeedback, breathing rate, inhale/exhale ratio, breathing amplitude, apnea detection, and respiratory line length.

- **Myofacial Analysis**
  Supporting face-point analysis, iris tracking, blinking detection, and talking detection.

- **Integrated Quality Control**
  Confidence and stability metrics providing insight into the confidence in the signal. User feedback on imaging conditions to support successful use.

- **Camera Selection**
  Front or rear facing camera selection.

- **Continuous or Spot Measurement**
  Options for continuous measurements or variable time window spot measurements to support varied use cases.

## Prerequisites

- iOS 15.0 or later
- Xcode 15.0 or later
- Physical iOS device (not compatible with emulators or Xcode simulator)
- Valid API key or OAuth configuration from [PresageTech Developer Portal](https://physiology.presagetech.com/)
- Camera permission configured in Info.plist (see setup instructions below)

## Quick Start

### 1. Add the Package Dependency

Add SmartSpectra SDK to your Xcode project using Swift Package Manager:

1. In Xcode, go to **File** → **Add Package Dependencies...**
2. Enter the repository URL: `https://github.com/Presage-Security/SmartSpectra`
3. Select **Branch: main** for the dependency rule
4. Add to your target

### 2. Configure Camera Permission

Add the camera usage description to your app's `Info.plist` file:

```xml
<key>NSCameraUsageDescription</key>
<string>This app needs camera access to measure vitals with SmartSpectra.</string>
```

### 3. Basic Integration

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    init() {
        // Set your API key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)
    }

    var body: some View {
        SmartSpectraView()
    }
}
```

If successful, you should be able to run the app and see something similar to this:

![iOS SmartSpectra SDK Demo](ios-quickstart.gif)

### 4. Advanced Configuration

```swift
init() {
    let apiKey = "YOUR_API_KEY_HERE"
    sdk.setApiKey(apiKey)

    // Configure measurement settings
    sdk.setSmartSpectraMode(.continuous)  // or .spot
    sdk.setMeasurementDuration(30.0)      // 20.0 - 120.0 seconds
    sdk.setCameraPosition(.front)         // or .back
    sdk.setRecordingDelay(3)              // countdown before recording
    sdk.showControlsInScreeningView(true) // show/hide UI controls
}
```

## SDK Modes

### Spot Mode

- Single measurement for specified duration
- Provides final results after measurement completion
- Best for quick health checks

### Continuous Mode

- Real-time monitoring with live updates
- Displays live pulse and breathing traces
- Ideal for extended monitoring sessions

## Custom UI and Performance Optimization

For custom UI implementations or performance optimization, you can control image output:

```swift
@ObservedObject var sdk = SmartSpectraSwiftSDK.shared
@ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared

init() {

    // Optional: Disable for optimal performance (no camera preview)
    // NOTE: setting this here will disable image output for the sdk being used anywhere else in the app. If other part of the app uses the imageOutput it will need to be re-enabled
    // sdk.setImageOutputEnabled(false)
}

func startMonitoring() {
    vitalsProcessor.startProcessing()
    vitalsProcessor.startRecording()
}

func stopMonitoring() {
    vitalsProcessor.stopProcessing()
    vitalsProcessor.stopRecording()
}

// Access camera frames when image output is enabled (default behavior)
if let image = vitalsProcessor.imageOutput {
    Image(uiImage: image)
        .resizable()
        .aspectRatio(contentMode: .fit)
}
```

## Data Access

Access measurement data through the metrics buffer:

```swift
@ObservedObject var sdk = SmartSpectraSwiftSDK.shared

if let metrics = sdk.metricsBuffer {
    // Access pulse data
    metrics.pulse.rate.forEach { measurement in
        print("Pulse: \(measurement.value) BPM at \(measurement.time)s")
    }

    // Access breathing data
    metrics.breathing.rate.forEach { rate in
        print("Breathing: \(rate.value) RPM at \(rate.time)s")
    }
}
```

## Authentication

The SDK supports two authentication methods:

### API Key Authentication

Set your API key during initialization:

```swift
sdk.setApiKey("YOUR_API_KEY_HERE")
```

### OAuth Authentication

For TestFlight/App Store releases, place the OAuth configuration plist file in your app's root directory. No additional code required.

## Face Mesh and Detection

Display face mesh points for debugging or custom UI:

```swift
if let edgeMetrics = sdk.edgeMetrics,
        edgeMetrics.hasFace && !edgeMetrics.face.landmarks.isEmpty && isFaceMeshEnabled {
        // Visual representation of mesh points from edge metrics
        if let latestLandmarks = edgeMetrics.face.landmarks.last {
            GeometryReader { geometry in
                ZStack {
                    ForEach(Array(latestLandmarks.value.enumerated()), id: \.offset) { index, landmark in
                        Circle()
                            .fill(Color.blue)
                            .frame(width: 3, height: 3)
                            .position(x: CGFloat(landmark.x) * geometry.size.width / 1280.0,
                                    y: CGFloat(landmark.y) * geometry.size.height / 1280.0)
                    }
                }
            }
            .frame(width: 400, height: 400) // Adjust the height as needed
        }
    }
```

## Data Serialization

Export and import measurement data:

```swift
// Serialize to binary format
do {
    let data = try metrics.serializedData()
    // Save or transmit data
} catch {
    print("Serialization failed: \(error)")
}

// Deserialize from binary data
do {
    let decodedMetrics = try MetricsBuffer(serializedBytes: data)
    // Use decoded metrics
} catch {
    print("Deserialization failed: \(error)")
}
```

## Best Practices

- **Device Orientation**: Recommend portrait mode only for optimal performance
- **Lighting Conditions**: Ensure adequate lighting for accurate measurements
- **Camera Position**: Front camera typically provides better face detection
- **Measurement Duration**: 30-60 seconds recommended for reliable results
- **Background Processing**: Disable image output (`setImageOutputEnabled(false)`) for background vital monitoring to optimize performance

## Troubleshooting

- Ensure physical device testing (simulators not supported)
- Verify API key validity and network connectivity
- Check camera permissions in device settings
- Ensure adequate lighting and face visibility
- Review measurement duration and mode settings

## Topics

### Classes

- ``SmartSpectraSwiftSDK``
- ``SmartSpectraVitalsProcessor``

### Views and UI Components

- ``SmartSpectraView``
- ``ContinuousVitalsPlotView``

### Protocols

- ``TimeStamped``

### Type Aliases

- ``Metrics``
- ``MetricsBuffer``

### Enumerations

- ``SmartSpectraMode``

### Main Data Containers

The primary data structures that contain all metrics and metadata.

- ``Presage_Physiology_MetricsBuffer``
- ``Presage_Physiology_Metrics``
- ``Presage_Physiology_Metadata``

### Physiological Metrics

Structures containing vital sign measurements and physiological data.

- ``Presage_Physiology_Pulse``
- ``Presage_Physiology_Breathing``
- ``Presage_Physiology_BloodPressure``

### Face Detection Data

Structures for face-related detection and landmark data.

- ``Presage_Physiology_Face``
- ``Presage_Physiology_Landmarks``

### Measurement Types

Different types of measurement structures with varying data precision.

- ``Presage_Physiology_Measurement``
- ``Presage_Physiology_MeasurementWithConfidence``
- ``Presage_Physiology_DetectionStatus``

### Geometric Data Types

Point structures for 2D and 3D coordinate data.

- ``Presage_Physiology_Point2dFloat``
- ``Presage_Physiology_Point2dInt32``
- ``Presage_Physiology_Point3dFloat``

### Supporting Data Structures

Helper structures that support the main physiological data types.

- ``Presage_Physiology_Strict``
- ``Presage_Physiology_Trace``
