//
//  HeadlessSDKExample.swift
//  demo-app
//
//  Created by Ashraful Islam on 3/5/25.
//


import SwiftUI
import SmartSpectraSwiftSDK

@available(iOS 16.0, *)
struct HeadlessSDKExample: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared
    @State private var isVitalMonitoringEnabled: Bool = false
    @State private var showCameraFeed: Bool = false

    init() {
        sdk.setSmartSpectraMode(.continuous)
        sdk.setCameraPosition(.front)

        // (Required), If you want to use Oauth, copy the Oauth config from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your app's root.
        // (Required), Deprecated. set apiKey. API key from https://physiology.presagetech.com. Leave default if you want to use oauth. Oauth overrides api key
        /*
         * assuming you already set it in ContentView.swift, you don't need to set it here again
         */
//        let apiKey = "YOUR_API_KEY_HERE"
//        sdk.setApiKey(apiKey)
    }

    var body: some View {
        VStack(spacing: 20) {
            // Vitals (always at top)
            GroupBox(label: Text("Vitals")) {
                ContinuousVitalsPlotView()
                HStack {
                    Text("Status: \(vitalsProcessor.statusHint)")
                        .font(.caption)
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
                .padding(.horizontal, 10)
            }

            // Camera Preview Toggle
            HStack {
                Text("Camera Preview")
                Spacer()
                Toggle("", isOn: $showCameraFeed)
                    .onChange(of: showCameraFeed) { newValue in
                        toggleCameraFeedDisplay(enabled: newValue)
                    }
            }
            .padding(.horizontal)

            // Camera Feed (only shows when enabled)
            if showCameraFeed {
                Group {
                    if let image = vitalsProcessor.imageOutput {
                        Image(uiImage: image)
                            .resizable()
                            .aspectRatio(contentMode: .fit)
                    } else {
                        if #available(iOS 17.0, *) {
                            ContentUnavailableView {
                                Label("Camera Feed", systemImage: "camera.fill")
                            } description: {
                                if !isVitalMonitoringEnabled {
                                    Text("Start monitoring to see live frames")
                                } else {
                                    Text("Starting camera feed...")
                                }
                            }
                        } else {
                            VStack(spacing: 8) {
                                Image(systemName: "camera.fill")
                                    .font(.largeTitle)
                                    .foregroundColor(.secondary)
                                Text("Camera Feed")
                                    .font(.headline)
                                    .foregroundColor(.secondary)
                                if !isVitalMonitoringEnabled {
                                    Text("Start monitoring to see live frames")
                                        .font(.caption)
                                        .foregroundColor(.secondary)
                                } else {
                                    Text("Starting camera feed...")
                                        .font(.caption)
                                        .foregroundColor(.secondary)
                                }
                            }
                        }
                    }
                }
                .frame(height: 200)
                .cornerRadius(8)
            }

            Spacer()
        }
        .padding()
        .onDisappear {
            stopVitalsMonitoring()
        }
    }

    func startVitalsMonitoring() {
        vitalsProcessor.startProcessing()
        vitalsProcessor.startRecording()
    }

    func stopVitalsMonitoring() {
        vitalsProcessor.stopRecording()
        vitalsProcessor.stopProcessing()
    }

    /// Toggles camera feed display and starts processing if needed
    /// - Parameter enabled: When true, enables camera feed preview; when false, hides the camera feed
    private func toggleCameraFeedDisplay(enabled: Bool) {
        // Enable image output if not enabled already
        if enabled {
            // this sets it for the shared instance of the sdk and will affect other parts of the app using the sdk
            sdk.setImageOutputEnabled(enabled)
        }

    }
}
