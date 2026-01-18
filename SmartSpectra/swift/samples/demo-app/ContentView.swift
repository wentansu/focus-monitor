import SwiftUI
import AVFoundation
import SmartSpectraSwiftSDK

struct ContentView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared
    @State private var isStreaming = false

    @State private var smoothedBlinkingData: [(Float, Float)] = []
    @State private var smoothedTalkingData: [(Float, Float)] = []
    
    // Accumulated detection data from edge metrics for real-time updates
    @State private var accumulatedBlinkingData: [Presage_Physiology_DetectionStatus] = []
    @State private var accumulatedTalkingData: [Presage_Physiology_DetectionStatus] = []
    
    // Processed data for display
    @State private var rawBlinkingData: [(Float, Float)] = []
    @State private var rawTalkingData: [(Float, Float)] = []
    
    // Detection state for hysteresis
    @State private var lastBlinkState: Bool = false
    @State private var lastTalkState: Bool = false
    @State private var blinkStateChangeTime: Float = 0
    @State private var talkStateChangeTime: Float = 0
    
    // Debug info
    @State private var debugBlinkCount: Int = 0
    @State private var debugTalkCount: Int = 0
    @State private var lastBlinkDetected: Bool = false
    @State private var lastTalkDetected: Bool = false
    
    // Maximum data points to keep (to prevent memory issues)
    private let maxDataPoints = 500

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
            // add smartspectra view
            SmartSpectraView()
            
            // Streaming controls and status
            VStack(spacing: 8) {
                HStack {
                    Button(isStreaming ? "Stop Streaming" : "Start Streaming") {
                        isStreaming.toggle()
                        if isStreaming {
                            vitalsProcessor.biometricStreamer.resetStats()
                            vitalsProcessor.startStreaming()
                        } else {
                            vitalsProcessor.stopStreaming()
                        }
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(isStreaming ? .red : .blue)
                    
                    Button("Reset Data") {
                        resetFaceData()
                    }
                    .buttonStyle(.bordered)
                }
                
                // Recording state indicator - IMPORTANT!
                HStack(spacing: 4) {
                    if vitalsProcessor.isRecording {
                        Circle()
                            .fill(Color.red)
                            .frame(width: 8, height: 8)
                        Text("Recording - Data flowing")
                            .font(.caption2)
                            .foregroundColor(.green)
                    } else {
                        Circle()
                            .fill(Color.orange)
                            .frame(width: 8, height: 8)
                        Text("Not recording - Press CHECKUP button above to start!")
                            .font(.caption2)
                            .foregroundColor(.orange)
                    }
                }
                
                // Connection status with live stats
                HStack(spacing: 12) {
                    // Connection indicator
                    HStack(spacing: 4) {
                        Circle()
                            .fill(vitalsProcessor.biometricStreamer.isConnected ? Color.green : Color.red)
                            .frame(width: 8, height: 8)
                        Text(vitalsProcessor.biometricStreamer.isConnected ? "WS Connected" : "WS Disconnected")
                            .font(.caption2)
                    }
                    
                    // Messages sent counter
                    if isStreaming || vitalsProcessor.biometricStreamer.messagesSentCount > 0 {
                        HStack(spacing: 2) {
                            Image(systemName: "arrow.up.circle.fill")
                                .foregroundColor(.blue)
                                .font(.caption2)
                            Text("\(vitalsProcessor.biometricStreamer.messagesSentCount)")
                                .font(.caption2.monospacedDigit())
                        }
                        
                        // Messages received counter
                        HStack(spacing: 2) {
                            Image(systemName: "arrow.down.circle.fill")
                                .foregroundColor(.green)
                                .font(.caption2)
                            Text("\(vitalsProcessor.biometricStreamer.messagesReceivedCount)")
                                .font(.caption2.monospacedDigit())
                        }
                    }
                }
                .foregroundColor(.gray)
                
                // Last sent summary
                if !vitalsProcessor.biometricStreamer.lastSentSummary.isEmpty {
                    Text("Last: \(vitalsProcessor.biometricStreamer.lastSentSummary)")
                        .font(.caption2)
                        .foregroundColor(.gray)
                }
                
                // Show error if any
                if let error = vitalsProcessor.biometricStreamer.connectionError {
                    Text("Error: \(error)")
                        .font(.caption)
                        .foregroundColor(.red)
                        .lineLimit(2)
                }
                
                // Instructions when streaming but not recording
                if isStreaming && !vitalsProcessor.isRecording && vitalsProcessor.biometricStreamer.messagesSentCount == 0 {
                    Text("👆 Tap the CHECKUP button above, then press the record button to start capturing vitals")
                        .font(.caption2)
                        .foregroundColor(.orange)
                        .multilineTextAlignment(.center)
                        .padding(.horizontal)
                }
            }
            .padding(.vertical, 4)


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


            // Scrolling view to view additional metrics from measurement
            ScrollView {
                VStack {

                    // Plotting example
                    if let metrics = sdk.metricsBuffer {
                        let pulse = metrics.pulse
                        let breathing = metrics.breathing
                        let bloodPressure = metrics.bloodPressure

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
                            // From metricsBuffer (processed)
                            if !metrics.face.blinking.isEmpty {
                                LineChartView(orderedPairs: metrics.face.blinking.map { ($0.time, $0.detected ? 1.0 : 0.0) }, title: "Blinking (from MetricsBuffer)", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                            if !metrics.face.talking.isEmpty {
                                LineChartView(orderedPairs: metrics.face.talking.map { ($0.time, $0.detected ? 1.0 : 0.0) }, title: "Talking (from MetricsBuffer)", xLabel: "Time", yLabel: "Value", showYTicks: true)
                            }
                        }
                    }
                    
                    // Face section from accumulated edge metrics (always show if data available)
                    Section("Face Detection (Real-time)") {
                        // Enhanced debug info with detection rates
                        VStack(alignment: .leading, spacing: 6) {
                            // Blinking stats
                            let blinkDetectedCount = accumulatedBlinkingData.filter { $0.detected }.count
                            let blinkStableCount = accumulatedBlinkingData.filter { $0.stable }.count
                            HStack {
                                Text("👁️ Blinks:")
                                    .font(.caption.bold())
                                Spacer()
                                Text("\(blinkDetectedCount) detected / \(debugBlinkCount) samples (\(blinkStableCount) stable)")
                                    .font(.caption.monospacedDigit())
                            }
                            .foregroundColor(blinkDetectedCount > 0 ? .green : .gray)
                            
                            if debugBlinkCount > 0 {
                                let blinkRate = Double(blinkDetectedCount) / Double(debugBlinkCount) * 100
                                ProgressView(value: blinkRate, total: 100)
                                    .tint(blinkRate > 5 ? .green : .orange)
                                Text("Detection rate: \(String(format: "%.1f%%", blinkRate))")
                                    .font(.caption2)
                                    .foregroundColor(.gray)
                            }
                            
                            Divider()
                            
                            // Talking stats
                            let talkDetectedCount = accumulatedTalkingData.filter { $0.detected }.count
                            let talkStableCount = accumulatedTalkingData.filter { $0.stable }.count
                            HStack {
                                Text("🗣️ Talking:")
                                    .font(.caption.bold())
                                Spacer()
                                Text("\(talkDetectedCount) detected / \(debugTalkCount) samples (\(talkStableCount) stable)")
                                    .font(.caption.monospacedDigit())
                            }
                            .foregroundColor(talkDetectedCount > 0 ? .green : .gray)
                            
                            if debugTalkCount > 0 {
                                let talkRate = Double(talkDetectedCount) / Double(debugTalkCount) * 100
                                ProgressView(value: talkRate, total: 100)
                                    .tint(talkRate > 5 ? .green : .orange)
                                Text("Detection rate: \(String(format: "%.1f%%", talkRate))")
                                    .font(.caption2)
                                    .foregroundColor(.gray)
                            }
                            
                            // Tips if no detections
                            if debugBlinkCount > 50 && blinkDetectedCount == 0 {
                                Text("⚠️ No blinks detected. Ensure your face is visible and try blinking.")
                                    .font(.caption2)
                                    .foregroundColor(.orange)
                                    .padding(.top, 4)
                            }
                            if debugTalkCount > 50 && talkDetectedCount == 0 {
                                Text("⚠️ No talking detected. Try speaking or moving your mouth.")
                                    .font(.caption2)
                                    .foregroundColor(.orange)
                                    .padding(.top, 4)
                            }
                        }
                        .padding(.vertical, 8)
                        .padding(.horizontal, 8)
                        .background(Color.gray.opacity(0.1))
                        .cornerRadius(8)
                        
                        // Smoothed data from edge metrics
                        if !smoothedBlinkingData.isEmpty {
                            LineChartView(orderedPairs: smoothedBlinkingData, title: "Blinking (Smoothed)", xLabel: "Time", yLabel: "Value", showYTicks: true)
                        } else {
                            Text("No blinking data yet...")
                                .font(.caption)
                                .foregroundColor(.gray)
                        }
                        if !smoothedTalkingData.isEmpty {
                            LineChartView(orderedPairs: smoothedTalkingData, title: "Talking (Smoothed)", xLabel: "Time", yLabel: "Value", showYTicks: true)
                        } else {
                            Text("No talking data yet...")
                                .font(.caption)
                                .foregroundColor(.gray)
                        }
                        
                        // Raw accumulated data
                        if !rawBlinkingData.isEmpty {
                            LineChartView(orderedPairs: rawBlinkingData, title: "Blinking (Raw Accumulated)", xLabel: "Time", yLabel: "Value", showYTicks: true)
                        }
                        if !rawTalkingData.isEmpty {
                            LineChartView(orderedPairs: rawTalkingData, title: "Talking (Raw Accumulated)", xLabel: "Time", yLabel: "Value", showYTicks: true)
                        }
                    }


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
                }
            }
        }
        .padding()
        // Subscribe to edgeMetrics for real-time updates - ACCUMULATE data over time
        .onChange(of: sdk.edgeMetrics) { newEdgeMetrics in
            if let metrics = newEdgeMetrics {
                // Diagnostic logging
                if metrics.hasFace {
                    let blinkArraySize = metrics.face.blinking.count
                    let talkArraySize = metrics.face.talking.count
                    let landmarkCount = metrics.face.landmarks.count
                    
                    // Accumulate blinking data
                    for detection in metrics.face.blinking {
                        // Only add if this is a new timestamp (avoid duplicates)
                        if accumulatedBlinkingData.isEmpty || detection.time > (accumulatedBlinkingData.last?.time ?? 0) {
                            accumulatedBlinkingData.append(detection)
                        }
                    }
                    
                    // Accumulate talking data
                    for detection in metrics.face.talking {
                        if accumulatedTalkingData.isEmpty || detection.time > (accumulatedTalkingData.last?.time ?? 0) {
                            accumulatedTalkingData.append(detection)
                        }
                    }
                    
                    // Trim to max data points to prevent memory issues
                    if accumulatedBlinkingData.count > maxDataPoints {
                        accumulatedBlinkingData.removeFirst(accumulatedBlinkingData.count - maxDataPoints)
                    }
                    if accumulatedTalkingData.count > maxDataPoints {
                        accumulatedTalkingData.removeFirst(accumulatedTalkingData.count - maxDataPoints)
                    }
                    
                    // Update debug info
                    debugBlinkCount = accumulatedBlinkingData.count
                    debugTalkCount = accumulatedTalkingData.count
                    lastBlinkDetected = metrics.face.blinking.last?.detected ?? false
                    lastTalkDetected = metrics.face.talking.last?.detected ?? false
                    
                    // Update raw display data (all accumulated data, no filtering)
                    rawBlinkingData = accumulatedBlinkingData.map { ($0.time, $0.detected ? 1.0 : 0.0) }
                    rawTalkingData = accumulatedTalkingData.map { ($0.time, $0.detected ? 1.0 : 0.0) }
                    
                    // Update smoothed data from accumulated detections
                    smoothedBlinkingData = generateImprovedSmoothedData(
                        rawDetections: accumulatedBlinkingData,
                        windowSize: 5,
                        trueThreshold: 3,
                        requireStable: false,  // Don't filter by stable for now to see all data
                        hysteresisTime: 0.1,
                        lastState: &lastBlinkState,
                        lastStateChangeTime: &blinkStateChangeTime
                    )
                    smoothedTalkingData = generateImprovedSmoothedData(
                        rawDetections: accumulatedTalkingData,
                        windowSize: 7,
                        trueThreshold: 4,
                        requireStable: false,
                        hysteresisTime: 0.15,
                        lastState: &lastTalkState,
                        lastStateChangeTime: &talkStateChangeTime
                    )
                }
            }
        }
    }

    /// Resets all accumulated face detection data
    private func resetFaceData() {
        accumulatedBlinkingData = []
        accumulatedTalkingData = []
        rawBlinkingData = []
        rawTalkingData = []
        smoothedBlinkingData = []
        smoothedTalkingData = []
        debugBlinkCount = 0
        debugTalkCount = 0
        lastBlinkDetected = false
        lastTalkDetected = false
        lastBlinkState = false
        lastTalkState = false
        blinkStateChangeTime = 0
        talkStateChangeTime = 0
    }

    /// Improved smoothing algorithm with stability filtering, larger window, and hysteresis
    /// - Parameters:
    ///   - rawDetections: Raw detection status array from SDK
    ///   - windowSize: Size of the sliding window for majority voting
    ///   - trueThreshold: Minimum count of true detections needed to output true
    ///   - requireStable: Whether to only consider stable detections
    ///   - hysteresisTime: Minimum time (seconds) before state can change again
    ///   - lastState: Binding to track the previous detection state
    ///   - lastStateChangeTime: Binding to track when state last changed
    private func generateImprovedSmoothedData(
        rawDetections: [Presage_Physiology_DetectionStatus],
        windowSize: Int,
        trueThreshold: Int,
        requireStable: Bool,
        hysteresisTime: Float,
        lastState: inout Bool,
        lastStateChangeTime: inout Float
    ) -> [(Float, Float)] {
        guard !rawDetections.isEmpty else {
            return []
        }

        var smoothedPoints: [(Float, Float)] = []
        var detectionHistory: [(time: Float, detected: Bool, stable: Bool)] = []
        var currentState = lastState
        var stateChangeTime = lastStateChangeTime

        for detectionStatus in rawDetections {
            // Add to history
            detectionHistory.append((
                time: detectionStatus.time,
                detected: detectionStatus.detected,
                stable: detectionStatus.stable
            ))
            
            // Maintain window size
            if detectionHistory.count > windowSize {
                detectionHistory.removeFirst()
            }

            // Count detections, optionally filtering by stability
            let relevantDetections: [(time: Float, detected: Bool, stable: Bool)]
            if requireStable {
                relevantDetections = detectionHistory.filter { $0.stable }
            } else {
                relevantDetections = detectionHistory
            }
            
            // If no stable detections, use last known state
            guard !relevantDetections.isEmpty else {
                smoothedPoints.append((detectionStatus.time, currentState ? 1.0 : 0.0))
                continue
            }

            let trueCount = relevantDetections.filter { $0.detected }.count
            let totalCount = relevantDetections.count
            
            // Determine target state based on majority voting
            let targetState = trueCount >= min(trueThreshold, (totalCount / 2) + 1)
            
            // Apply hysteresis: only change state if enough time has passed
            let timeSinceLastChange = detectionStatus.time - stateChangeTime
            if targetState != currentState && timeSinceLastChange >= hysteresisTime {
                currentState = targetState
                stateChangeTime = detectionStatus.time
            }

            smoothedPoints.append((detectionStatus.time, currentState ? 1.0 : 0.0))
        }
        
        // Update the state bindings for next call
        lastState = currentState
        lastStateChangeTime = stateChangeTime

        return smoothedPoints
    }

}

#Preview {
    ContentView()
}