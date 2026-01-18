import SwiftUI
import AVFoundation
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared

    // Set the initial camera position. Can be set to .front or .back. Defaults to .front
    @State var cameraPosition: AVCaptureDevice.Position = .front
    // Set the initial smartSpectraMode. Can be set to .spot or .continuous. Defaults to .continuous
    @State var smartSpectraMode: SmartSpectraMode = .spot
    // Set the initial measurement duration. Valid range for measurement duration is between 20.0 and 120.0. Defaults to 30.0
    @State var measurementDuration: Double = 30.0

    // App display configurations
    let isCustomizationEnabled: Bool = false
    let isFaceMeshEnabled: Bool = false

    init() {
        // (Required) Authentication. Only need to use one of the two options: API Key or Oauth below
        // Authentication with Oauth currently only supported for apps in testflight/appstore
            // Option 1: (authentication with api key) set apiKey. API key from https://physiology.presagetech.com. Leave default or remove if you want to use oauth. Oauth overrides api key
            let apiKey = "IPydFbBExf7dVeWFc71cgvuoxUBPB3z7PT80EZAf"
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

        VStack {
            //add smartspectra view
            SmartSpectraView()

            if (isCustomizationEnabled) {
                // (Optional), example of how to switch camera at runtime
                Button(cameraPosition == .front ? "Switch to Back Camera": "Switch to Front Camera", systemImage: "camera.rotate") {
                    if cameraPosition == .front {
                        cameraPosition = .back
                    } else {
                        cameraPosition = .front
                    }
                    sdk.setCameraPosition(cameraPosition)
                }
                // (Optional), example of how to switch smartSpectraMode at runtime
                Button(smartSpectraMode == .spot ? "Switch to Continuous" : "Switch to Spot", systemImage: smartSpectraMode == .spot ? "waveform.path" : "chart.dots.scatter") {
                    if smartSpectraMode == .spot {
                        smartSpectraMode = .continuous
                    } else {
                        smartSpectraMode = .spot
                    }
                    sdk.setSmartSpectraMode(smartSpectraMode)
                }

                // (Optional), example of how to change measurementDuration at runtime
                Stepper(value: $measurementDuration, in: 20...120, step: 5) {
                    Text("Measurement Duration: \(measurementDuration.formatted(.number))")
                }
                .onChange(of: measurementDuration) {_ in
                    sdk.setMeasurementDuration(measurementDuration)
                }
            }


            // Scrolling view to view additional metrics from measurment
            ScrollView {
                VStack {
                 //  To show additional meta data of the analysis
                 //  Text("Metadata: \(String(describing: metricsBuffer.metadata))")

                    // Plotting example
                    if let metrics = sdk.metricsBuffer {
                        let pulse = metrics.pulse
                        let breathing = metrics.breathing
                        let bloodPressure = metrics.bloodPressure
                        let face = metrics.face

                        Section("Metadata") {
                            if metrics.hasMetadata {
                                VStack{
                                    Text("ID:\(metrics.metadata.id)")
                                    Text("Upload Timestamp:\(metrics.metadata.uploadTimestamp)")
                                }

                            }
                        }

                        Section ("Pulse") {
                            if !pulse.trace.isEmpty {
                                LineChartView(orderedPairs: pulse.trace.map { ($0.time, $0.value) }, title: "Pulse Pleth", xLabel: "Time", yLabel: "Value", showYTicks: false)
                            }

                            if !pulse.rate.isEmpty {
                                LineChartView(orderedPairs: pulse.rate.map { ($0.time, $0.value) }, title: "Pulse Rates", xLabel: "Time", yLabel: "Value", showYTicks: true)
                                LineChartView(orderedPairs: pulse.rate.map { ($0.time, $0.confidence) }, title: "Pulse Rate Confidence", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }

//                            if !pulse..isEmpty {
//                                //for hrv analysis this will only be producable with 60 second version of SDK
//                                LineChartView(orderedPairs: pulse..map { ($0.time, $0.value) }, title: "Pulse Rate Variability", xLabel: "Time", yLabel: "value", showYTicks: true)
//                            }
                        }

                        Section ("Breathing") {
                            if !breathing.upperTrace.isEmpty {
                                LineChartView(orderedPairs: breathing.upperTrace.map { ($0.time, $0.value) }, title: "Breathing Pleth", xLabel: "Time", yLabel: "Value", showYTicks: false)
                            }

                            if !breathing.rate.isEmpty {
                                LineChartView(orderedPairs: breathing.rate.map { ($0.time, $0.value) }, title: "Breathing Rates", xLabel: "Time", yLabel: "Value", showYTicks: true)
                                LineChartView(orderedPairs: breathing.rate.map { ($0.time, $0.confidence) }, title: "Breathing Rate Confidence", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }

                            if !breathing.amplitude.isEmpty {
                                LineChartView(orderedPairs: breathing.amplitude.map { ($0.time, $0.value) }, title: "Breathing Amplitude", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                            if !breathing.apnea.isEmpty {
                                LineChartView(orderedPairs: breathing.apnea.map { ($0.time, $0.detected ? 1.0 : 0.0) }, title: "Apnea Detection", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                            if !breathing.baseline.isEmpty {
                                LineChartView(orderedPairs: breathing.baseline.map { ($0.time, $0.value) }, title: "Breathing Baseline", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                            if !breathing.respiratoryLineLength.isEmpty {
                                LineChartView(orderedPairs: breathing.respiratoryLineLength.map { ($0.time, $0.value) }, title: "Respiratory Line Length", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }

                            if !breathing.inhaleExhaleRatio.isEmpty {
                                LineChartView(orderedPairs: breathing.inhaleExhaleRatio.map { ($0.time, $0.value) }, title: "Inhale-Exhale Ratio", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                        }

                        Section ("Blood Pressure") {
                            if !bloodPressure.phasic.isEmpty {
                                LineChartView(orderedPairs: bloodPressure.phasic.map { ($0.time, $0.value) }, title: "Phasic", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                        }

                        Section ("Face") {
                            if !face.blinking.isEmpty {
                                LineChartView(orderedPairs: face.blinking.map { ($0.time, $0.detected ? 1.0 : 0.0) }, title: "Blinking", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                            if !face.talking.isEmpty {
                                LineChartView(orderedPairs: face.talking.map { ($0.time, $0.detected ? 1.0 : 0.0) }, title: "Talking", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                        }
                    }
                }
            }
        }
        .padding()
    }
}

#Preview {
    ContentView()
}
