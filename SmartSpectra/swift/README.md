# SmartSpectra SDK for iOS

## Integration Guide Overview

This guide provides instructions for integrating and utilizing the Presage SmartSpectra SDK for Swift Package Manager (SPM) publicly hosted at [SmartSpectra SDK](https://github.com/Presage-Security/SmartSpectra/tree/main/swift/sdk) to measure physiology metrics using an Apple mobile device's camera.

The app contained in this repo is an example of using the SmartSpectra SDK and should run out of the box after adding [SmartSpectra Swift SDK](https://github.com/Presage-Security/SmartSpectra/tree/main/swift/sdk) and adding an API key.

## Table of Contents

- [Quick Start - Hello Vitals](#quick-start---hello-vitals)
- [Prerequisites](#prerequisites)
- [Demo App Guide](#demo-app-guide)
- [Advanced Integration](#advanced-integration)
- [Usage](#usage)
- [Generating Documentation](#generating-documentation)
- [Bugs & Troubleshooting](#bugs--troubleshooting)

## Quick Start - Hello Vitals

Get measuring vitals in under 5 minutes! This minimal example gets you up and running with heart rate and breathing measurements.

### Prerequisites

- **iOS 15.0 or later**
- **Xcode 15.0 or later**
- **API Key** from [physiology.presagetech.com](https://physiology.presagetech.com)
- **Physical device** (not usable with emulators or the Xcode simulator)

### Hello Vitals Example

Create a new iOS project in Xcode and add the SmartSpectra SDK dependency.

**Add Package Dependency:**

1. In Xcode, go to **File** → **Add Package Dependencies...**
2. Enter the URL: `https://github.com/Presage-Security/SmartSpectra`
3. Select **Branch** → **main**
4. Add to your target

**Info.plist:**

Add camera permission to your `Info.plist`:

```xml
<key>NSCameraUsageDescription</key>
<string>This app needs camera access to measure vitals with SmartSpectra.</string>
```

**ContentView.swift:**

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    init() {
        // Replace with your API key from https://physiology.presagetech.com
        sdk.setApiKey("YOUR_API_KEY")
    }

    var body: some View {
        SmartSpectraView()
    }
}
```

### Run and Test

1. **Build and run** your app on a physical device (camera required)
2. **Allow camera permission** when prompted
3. **Tap the measurement button** and follow on-screen instructions
4. **View your vitals** - heart rate, breathing rate, and more!

The SDK automatically handles camera permissions, measurement UI, and result display.

![iOS Quickstart Demo](docs/ios-quickstart.gif)

### Next Steps

- **Customize measurements**: See [Advanced Integration](#advanced-integration) for configuration options
- **Access raw metrics**: Learn about [metrics extraction](#extracting-and-using-metrics-data)
- **Try the demo app**: Follow the [Demo App Guide](#demo-app-guide) for a full-featured example

## Demo App Guide

1. Clone the repository and open the SmartSpectra workspace in Xcode:

    ```bash
    git clone https://github.com/Presage-Security/SmartSpectra/
    open smartspectra/swift/SmartSpectra.xcworkspace
    ```

2. Select the demo app target in Xcode.
3. Navigate to [ContentView.swift](samples/demo-app/ContentView.swift) and replace the placeholder API key with your actual API key obtained from PresageTech's developer portal (<https://physiology.presagetech.com/>) [See authentication](#authentication)
4. Setup the signing and capabilities for the demo app target in Xcode. Make sure to select your development team and set a unique bundle identifier.
5. Connect your iOS device to your computer.
6. Select your device as the target in Xcode.
7. Click the "Run" button in Xcode to build and run the demo app on your device.
8. Follow the on-screen instructions in the app to conduct a measurement and view the results.
9. If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.

## Advanced Integration

To integrate the SmartSpectra SDK into your iOS project, follow these steps:

### Swift Package Manager

The Swift Package Manager (SPM) is a tool for managing the distribution of Swift code. It automates the process of downloading, compiling, and linking dependencies.

To add SmartSpectra Swift SDK as a dependency to your Xcode project using SPM, follow either of these two sets of steps within Xcode:

- Method 1: Go to File -> "Add Package Dependencies..."
In the "Search or Enter Package URL" field, enter the URL "<https://github.com/Presage-Security/SmartSpectra>"
For the "Dependency Rule," select "Branch" and then "main."
For "Add to Target," select your project.

- Method 2: Open your project in Xcode.  Select your project in the Project Navigator, then click on the project in the Project/Targets Pane. Go to the Package Dependencies Tab, then click the "+" button.

  - **Note**: Some Version of Xcode Navigate to File > Swift Packages > Add Package Dependency
    Paste the repository URL for SmartSpectra SDK in the search bar and press Enter. The URL is <https://github.com/Presage-Security/SmartSpectra>.
    Select Add Package

### Camera Permission Setup

The SmartSpectra SDK requires camera access to measure vitals. You must add the camera usage description to your app's `Info.plist` file:

```xml
<key>NSCameraUsageDescription</key>
<string>This app needs camera access to measure vitals with SmartSpectra.</string>
```

### Authentication

You'll need either an API key or Oauth config to use the SmartSpectra Swift SDK. You can find instructions on how to do that [here](../docs/authentication.md)

- **API Key Option**: Add your API key to the [ContentView.swift](samples/demo-app/ContentView.swift) file by replacing the placeholder `"YOUR_API_KEY_HERE"`.
- **Oauth Config Option**: With the downloaded Oauth plist config from obtained during [Authentication](../docs/authentication.md) and place into your app's root directory.
![plist file location in repo](../docs/images/plist_location_in_repo.png)

> **Note**
> Oauth Option is currently only supported for TestFlight/App Store releases.

## Swift SDK API Overview

The library exposes a few key classes:

- **SmartSpectraSwiftSDK**: shared instance for configuring and receiving measurement data.
- **SmartSpectraView**: composite view containing the measurement button and result display.
- **SmartSpectraVitalsProcessor**: processor for headless vital monitoring without UI.

## Usage

### Example Code

Please refer to [ContentView.swift](samples/demo-app/ContentView.swift) for example usage and plotting of a pulse pleth waveform and breathing waveform.

- **Note**: to use this example repo make sure to under "Signing and Capabilities" of Targets "demo-app" to set:
- Team: Your desired developer profile
- Bundle Identifier: Your desired bundle identifier such as: `com.johnsmith.smartspectratest`
- If you are not a registered developer for the App Store follow the prompt to navigate to Settings > General > VPN & Device Management, then select your developer App certificate to trust it on your iOS device.

### Integrate the SmartSpectra View

You need to integrate the `SmartSpectraView` into your app which is composed of

- A button that allows the user to conduct a measurement and compute physiology metrics
- A resultview that shows the strict breathing rate and pulse rate after the measurement

Here's a minimal example using SwiftUI:

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    init() {
        // (Required) Authentication. Only need to use one of the two options: API Key or Oauth below
        // Authentication with Oauth currently only supported for apps in testflight/appstore
        // Option 1: (authentication with api key) set apiKey. API key from https://physiology.presagetech.com. Leave default or remove if you want to use oauth. Oauth overrides api key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)

        // Option 2: (Oauth) If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // No additional code needed for Oauth

    }

    var body: some View {
        // add smartspectra view
        SmartSpectraView()
   }
}
```

If you want a lot more customization you can do so in the init function of your view as shown below. See [ContentView.swift](samples/demo-app/ContentView.swift) for implementation example.

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    // Set the initial camera position. Can be set to .front or .back. Defaults to .front
    @State var cameraPosition: AVCaptureDevice.Position = .front
    // Set the initial smartSpectraMode. Can be set to .spot or .continuous. Defaults to .continuous
    @State var smartSpectraMode: SmartSpectraMode = .continuous
    // Set the initial measurement duration. Valid range for measurement duration is between 20.0 and 120.0. Defaults to 30.0
    @State var measurementDuration: Double = 30.0

    // App display configurations
    let isCustomizationEnabled: Bool = true
    let isFaceMeshEnabled: Bool = true

    init() {
        // (Required) Authentication. Only need to use one of the two options: API Key or Oauth below
        // Authentication with Oauth currently only supported for apps in testflight/appstore
        // Option 1: (authentication with api key) set apiKey. API key from https://physiology.presagetech.com. Leave default or remove if you want to use oauth. Oauth overrides api key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)

        // Option 2: (Oauth) If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // No additional code needed for Oauth

        // (optional) toggle display of camera and smartspectra mode controls in screening view
        sdk.showControlsInScreeningView(isCustomizationEnabled)
        // (Optional), set smartSpectraMode. Can be set to .spot or .continuous. Defaults to .continuous
        sdk.setSmartSpectraMode(smartSpectraMode)
        // (Optional), set measurementDuration. Valid range for measurement duration is between 20.0 and 120.0
        sdk.setMeasurementDuration(measurementDuration)
        // (Optional), set showFPS. To show fps in the screening view
        sdk.setShowFps(false)
        // (Optional), set recordingDelay. To set a initial countdown timer before recording starts. Defaults to 3
        sdk.setRecordingDelay(3)
        // (Optional), set cameraPosition. To set the camera position. Can be set to .front or .back. Defaults to .front
        sdk.setCameraPosition(cameraPosition)
    }

    var body: some View {
        // add smartspectra view
        SmartSpectraView()
    }
}
```

## SmartSpectra Mode

The SmartSpectra SDK supports two modes:

- Spot Mode (`SmartSpectraMode.spot`): In this mode, the SDK will take a single measurement for the specified duration.
- Continuous Mode (`SmartSpectraMode.continuous`): In this mode, the SDK will continuously take measurements for the specified duration. Currently defaults to infinite duration and manual stop. During continuous mode, live pulse and breathing rate is displayed; along with live plots of the pulse and breathing trace.

## Switching SmartSpectra Mode, Measurement Duration, and Camera Position at Runtime

See examples of how you can implement UI elements or programmatically change the SmartSpectra mode, measurement duration, and camera position at runtime in [ContentView.swift](samples/demo-app/ContentView.swift).

> [!IMPORTANT]
> You need to enter your API key string at `"YOUR_API_KEY_HERE"`. Optionally, you can also configure measurement type, duration, whether to show frame per second (fps) during screening.

## Using the SmartSpectra SDK headless

SmartSpectrSDK can be used headless without the SmartSpectraView. This is useful if you want to use the SDK to collect data in the background without displaying the camera view. It is also useful, if you want to display any other content while monitoring the vitals in the background. To get started with the headless mode, you will need to use `SmartSpectraVitalsProcessor` to control the vitals processing, while using the `SmartSpectraSwiftSDK` to configure and monitor the vitals.

Here's a minimal example to get started with the headless mode (see [HeadlessSDKExample.swift](samples/demo-app/HeadlessSDKExample.swift)):

```swift
import SwiftUI
import SmartSpectraSwiftSDK

struct HeadlessSDKExample: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared
    @State private var isVitalMonitoringEnabled: Bool = false

    init() {
        // (Required) Authentication. Only need to use one of the two options: API Key or Oauth below
        // Authentication with Oauth currently only supported for apps in testflight/appstore
        // Option 1: (authentication with api key) set apiKey. API key from https://physiology.presagetech.com. Leave default or remove if you want to use oauth. Oauth overrides api key
        let apiKey = "YOUR_API_KEY_HERE"
        sdk.setApiKey(apiKey)

        // Option 2: (Oauth) If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // No additional code needed for Oauth
    }

    var body: some View {
        VStack {
            GroupBox(label: Text("Vitals")) {
                ContinuousVitalsPlotView()
                Grid {
                    GridRow {
                        Text("Status: \(vitalsProcessor.statusHint)")
                    }
                    GridRow {
                        HStack {
                            Text("Vitals Monitoring")
                            Spacer()
                            Button(isVitalMonitoringEnabled ? "Stop": "Start") {
                                isVitalMonitoringEnabled.toggle()
                                if(isVitalMonitoringEnabled) {
                                    startVitalsMonitoring()
                                } else {
                                    stopVitalsMonitoring()
                                }
                            }
                        }
                    }
                }
                .padding(.horizontal, 10)
            }
        }
    }

    func startVitalsMonitoring() {
        vitalsProcessor.startProcessing()
        vitalsProcessor.startRecording()
    }

    func stopVitalsMonitoring() {
        vitalsProcessor.stopProcessing()
        vitalsProcessor.stopRecording()

    }
}
```

### Extracting and Using Metrics Data

The `MetricsBuffer` is the main struct generated using [swift-protobuf](https://github.com/apple/swift-protobuf) that contains all metrics data. You can access it through a `@ObservedObject` instance of `SmartSpectraSwiftSDK.shared`. This way any update to the metrics data will automatically trigger a UI update.

**Usage Example:**

```swift
@ObservedObject var shared = SmartSpectraSwiftSDK.shared

if let metrics = sdk.metricsBuffer {
  // Use the metrics

  // Access pulse data
  metrics.pulse.rate.forEach { measurement in
      print("Pulse rate value: \(measurement.value), time: \(measurement.time), confidence: \(measurement.confidence)")
  }

  // Access breathing data
  metrics.breathing.rate.forEach { rate in
      print("Breathing rate: \(rate.value), time: \(rate.time), confidence: \(rate.confidence)")
  }
}
```

### Detailed `MetricsBuffer` Struct Descriptions

> [!TIP]
> If you need to use the types directly, the MetricsBuffer and corresponding structs are under the `Presage_Physiology` namespace. You can type alias it from the `Presage_Physiology_MetricsBuffer` to `MetricsBuffer` for easier usage:
>
> ```swift
> typealias MetricsBuffer = Presage_Physiology_MetricsBuffer
> ```

> **TIP**
> `MetricsBuffer` is generated from a Protobuf message. The full descriptions of `MetricsBuffer` and `Metrics` messages, as well as the messages composing them, can currently be found in [SmartSpectra Protocol Buffer Reference](https://docs.physiology.presagetech.com/cpp/protobuf_reference.html).

Metrics Buffer contains the following parent structs:

```swift
struct MetricsBuffer {
    var pulse: Pulse
    var breathing: Breathing
    var bloodPressure: BloodPressure
    var face: Face
    var metadata: Metadata
}
```

### Measurement Types

- **`Measurement` Struct** : Represents a measurement with time and value:

```swift
struct Measurement {
    var time: Float
    var value: Float
    var stable: Bool
}
```

- **`MeasurementWithConfidence` Struct** : Includes confidence with the measurement:

```swift
struct MeasurementWithConfidence {
    var time: Float
    var value: Float
    var stable: Bool
    var confidence: Float
}
```

- **`DetectionStatus` Struct** :Used for events like apnea or face detection (blinking/talking):

```swift
struct DetectionStatus {
    var time: Float
    var detected: Bool
    var stable: Bool
}
```

#### Metric Types

- **`Pulse` Struct** : Contains pulse-related measurements, including rate, trace, and strict values:

```swift
struct Pulse {
    var rate: [MeasurementWithConfidence]
    var trace: [Measurement]
    var strict: Strict
}
```

- **`Breathing` Struct** : Handles breathing-related data with upper and lower traces, amplitude, apnea status, and other metrics:

```swift
struct Breathing {
    var rate: [MeasurementWithConfidence]
    var upperTrace: [Measurement]
    var lowerTrace: [Measurement]
    var amplitude: [Measurement]
    var apnea: [DetectionStatus]
    var respiratoryLineLength: [Measurement]
    var inhaleExhaleRatio: [Measurement]
    var strict: Strict
}
```

- **`BloodPressure` Struct** : Handles blood pressure measurements:

> [!CAUTION]
> Currently not available publicly, currently returned results are a duplicate of pulse pleth

```swift
struct BloodPressure {
    var phasic: [MeasurementWithConfidence]
}
```

- **`Face` Struct** : Includes detection statuses for blinking and talking:

```swift
struct Face {
    var blinking: [DetectionStatus]
    var talking: [DetectionStatus]
}
```

- **`Metadata` Struct** : Includes metadata information:

```swift
struct Metadata {
    var id: String
    var uploadTimestamp: String
    var apiVersion: String
}
```

#### Encoding and Decoding Protobuf Messages

To serialize `MetricsBuffer` into binary format:

```swift
do {
    let data = try metrics.serializedData()
    // Send `data` to your backend or save it
} catch {
    print("Failed to serialize metrics: \(error)")
}
```

To decode binary protobufdata into `MetricsBuffer`:

```swift
do {
    let decodedMetrics = try MetricsBuffer(serializedBytes: data)
    // Use `decodedMetrics` as needed
} catch {
    print("Failed to decode metrics: \(error)")
}
```

### Displaying face mesh points

You can display the face mesh points by accessing them from the edge metrics. The dense face landmarks are now available through the edge metrics system for continuous mode processing. Here's an example from [ContentView.swift](samples/demo-app/ContentView.swift):

```Swift
if let edgeMetrics = sdk.edgeMetrics, 
   edgeMetrics.hasFace && !edgeMetrics.face.landmarks.isEmpty {
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

Since the edge metrics are published you can also use `combine` to subscribe to the edge metrics to add a custom callback to further process the face landmarks.

## Device Orientation

We do not recommend landscape support. We recommend removing the "Landscape Left," "Landscape Right," and "Portrait Upside Down" modes from your supported interface orientations.

## Generating Documentation

Documentation is available online at [https://docs.physiology.presagetech.com/swift/documentation/smartspectraswiftsdk/](https://docs.physiology.presagetech.com/swift/documentation/smartspectraswiftsdk/).

You can also build and browse documentation locally within Xcode using the built-in documentation library. To generate and view API documentation:

1. Open your project in Xcode
2. Navigate to **Product** → **Build Documentation**
3. Once built, the documentation will be available in Xcode's documentation viewer for easy browsing

This provides a convenient way to access SDK documentation directly within your development environment.

## Bugs & Troubleshooting

For additional support, contact <[support@presagetech.com](mailto:support@presagetech.com)> or submit a [Github Issue](https://github.com/Presage-Security/SmartSpectra/issues)
