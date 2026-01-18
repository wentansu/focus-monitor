//
//  SmartSpectraProcessor.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 3/3/25.
//

import Foundation
import PresagePreprocessing
import CoreImage
import UIKit
import SwiftUI
import SwiftProtobuf
import Combine

/// Indicates the current state of the preprocessing pipeline.
public enum PresageProcessingStatus {
    case idle
    case processing
    case processed
    case error
}

/// Utility for converting ``CVPixelBuffer`` images to ``UIImage`` asynchronously.
final class ImageConverter {
    private let sharedContext = SharedCIContext.shared
    private let queue = DispatchQueue(label: "image.convert.queue", qos: .userInteractive)
    private var isProcessing = false
    private let lock = NSLock()

    /// Converts a pixel buffer to a `UIImage` off the main thread.
    /// - Parameters:
    ///   - pixelBuffer: The buffer to convert.
    ///   - completion: Closure invoked with the resulting image or `nil` on failure.
    func convertAsync(pixelBuffer: CVPixelBuffer, completion: @escaping (UIImage?) -> Void) {
        // Skip frame if already processing to prevent backlog
        lock.lock()
        if isProcessing {
            lock.unlock()
            return // Drop frame to maintain smooth UI
        }
        isProcessing = true
        lock.unlock()

        // Extract dimensions before async - these are fast, synchronous operations
        let width = CVPixelBufferGetWidth(pixelBuffer)
        let height = CVPixelBufferGetHeight(pixelBuffer)

        queue.async { [weak self] in
            guard let self = self else { return }

            defer {
                self.lock.lock()
                self.isProcessing = false
                self.lock.unlock()
            }

            autoreleasepool {
                let ciImage = CIImage(cvPixelBuffer: pixelBuffer)
                let rect = CGRect(x: 0, y: 0, width: width, height: height)

                self.sharedContext.createCGImage(ciImage, from: rect) { cgImage in
                    guard let cgImage = cgImage else {
                        completion(nil)
                        return
                    }
                    completion(UIImage(cgImage: cgImage))
                }
            }
        }
    }
}

/// Lower level API for controlling real‑time processing.
///
/// Most apps can rely on ``SmartSpectraView``, but this class provides
/// granular control for headless operation or custom UIs.
public class SmartSpectraVitalsProcessor: NSObject, ObservableObject {
    public static let shared = SmartSpectraVitalsProcessor()
    /// Live camera preview image updated in real-time during processing.
    ///
    /// This property provides access to the processed camera frames as `UIImage` objects.
    /// Images are only available when image output is enabled via ``SmartSpectraSwiftSDK/setImageOutputEnabled(_:)``.
    ///
    /// ## Usage Examples
    ///
    /// ### Display Camera Feed
    /// ```swift
    /// if let image = vitalsProcessor.imageOutput {
    ///     Image(uiImage: image)
    ///         .resizable()
    ///         .aspectRatio(contentMode: .fit)
    /// }
    /// ```
    ///
    /// ### Observe Frame Updates
    /// ```swift
    /// @State private var frameCount = 0
    ///
    /// // Observe frame updates
    /// .onReceive(vitalsProcessor.$imageOutput) { image in
    ///     if let frame = image {
    ///         frameCount += 1
    ///         // Custom frame processing
    ///         analyzeFrame(frame)
    ///     }
    /// }
    /// ```
    ///
    /// - Note: This property is `nil` when image output is disabled for performance optimization.
    /// - Tip: Use `onReceive` to react to frame updates
    @Published public var imageOutput: UIImage?
    /// Current status of the underlying `PresagePreprocessing` engine.
    @Published public var processingStatus: PresageProcessingStatus = .idle
    /// Countdown timer value for spot measurements.
    @Published public var counter: Double = 0
    /// Estimated frames per second of incoming video.
    @Published public var fps: Int = 0
    /// Latest status code emitted by the preprocessing engine.
    @Published public var lastStatusCode: StatusCode = .processingNotStarted
    /// Human readable hint string for the current status code.
    @Published public var statusHint: String = ""
    /// Indicates whether recording is currently active.
    @Published public var isRecording: Bool = false
    var presageProcessing: PresagePreprocessing! // Implicitly unwrapped optional, initialized in init
    var lastTimestamp: Int?
    var fpsValues: [Int] = []
    let movingAveragePeriod = 10
    private var sdk: SmartSpectraSwiftSDK
    private var avCaptureDeviceManager: AVCaptureDeviceManager = AVCaptureDeviceManager.shared
    @State private var coreIsRunning: Bool = false // TODO: perhaps add a processing status for not ready
    private let imageConverter = ImageConverter()
    private weak var authHandler: AuthHandler?
    public let biometricStreamer = BiometricStreamer()
    private var cancellables = Set<AnyCancellable>()

    private override init(){
        sdk = SmartSpectraSwiftSDK.shared
        authHandler = AuthHandler.shared
        // Initialize PresagePreprocessing AFTER SDK is initialized (which calls useTestServer)
        presageProcessing = PresagePreprocessing()
        super.init()
        presageProcessing.delegate = self
    }

    internal func setRecordingState(_ state: Bool) {
        presageProcessing.buttonStateChanged(inFramework: state)
        DispatchQueue.main.async {
            self.isRecording = state
        }
    }

    internal func changeProcessingMode(_ mode: SmartSpectraMode) {
        guard presageProcessing.mode != mode.presageMode else { return }
        presageProcessing.mode = mode.presageMode
        sdk.config.smartSpectraMode = mode
    }

    internal func setProcessingCameraPosition(_ position: AVCaptureDevice.Position) {
        guard presageProcessing.cameraPosition != position else { return }
        presageProcessing.cameraPosition = position
        sdk.config.cameraPosition = position
    }

    internal func setApiKey(_ apiKey: String) {
        presageProcessing.apiKey = apiKey
        sdk.config.apiKey = apiKey
    }

    internal func setSpotDuration(_ duration: Double) {
        presageProcessing.spotDuration = duration
        sdk.config.measurementDuration = duration
    }

    private func setupProcessing() {
        let oauth_enabled  = authHandler?.isOauthEnabled ?? false

        if !oauth_enabled {
            guard let apiKey = sdk.config.apiKey, !apiKey.isEmpty else {
                print("ERROR: API key missing - cannot proceed with processing")
                DispatchQueue.main.async {
                    self.processingStatus = .error
                }
                return
            }
            setApiKey(apiKey)
        }

        changeProcessingMode(sdk.config.smartSpectraMode)
        setProcessingCameraPosition(sdk.config.cameraPosition)
        if(sdk.config.smartSpectraMode == .spot) {
            setSpotDuration(sdk.config.measurementDuration)
        }
    }

    /// Start streaming biometric data to the SAM gateway
    /// Uses the default URL: udp://10.194.124.2:5005
    /// - Parameter autoStartRecording: If true, automatically starts recording to generate metrics
    public func startStreaming(autoStartRecording: Bool = false) {
        startStreaming(urlString: "udp://10.194.124.2:5005", autoStartRecording: autoStartRecording)
    }
    
    /// Start streaming biometric data to a custom WebSocket URL
    /// - Parameters:
    ///   - urlString: The WebSocket URL to connect to
    ///   - autoStartRecording: If true, automatically starts processing and recording
    public func startStreaming(urlString: String, autoStartRecording: Bool = false) {
        // Generate a new session ID for this streaming session
        biometricStreamer.sessionId = UUID().uuidString
        biometricStreamer.connect(urlString: urlString)
        
        print("[VitalsProcessor] 🚀 Started streaming session: \(biometricStreamer.sessionId)")
        print("[VitalsProcessor] 📡 Connecting to: \(urlString)")
        print("[VitalsProcessor] ℹ️ Note: Metrics will only stream when RECORDING is active in SmartSpectraView")
        
        // Track if we've received any data
        var edgeMetricsCount = 0
        var metricsBufferCount = 0
        
        // Subscribe to edge metrics (real-time, high frequency)
        sdk.$edgeMetrics.sink { [weak self] metrics in
            guard let self = self else { return }
            
            if let metrics = metrics {
                edgeMetricsCount += 1
                if edgeMetricsCount == 1 {
                    print("[VitalsProcessor] ✅ First edgeMetrics received! Data is flowing.")
                }
                self.sendEdgeMetrics(metrics)
            } else if edgeMetricsCount == 0 {
                // Only log once when nil
                print("[VitalsProcessor] ⏳ Waiting for edgeMetrics... (start recording in SmartSpectraView)")
            }
        }.store(in: &cancellables)
        
        // Subscribe to metricsBuffer (processed data, includes more details)
        sdk.$metricsBuffer.sink { [weak self] metricsBuffer in
            guard let self = self else { return }
            
            if let metricsBuffer = metricsBuffer {
                metricsBufferCount += 1
                if metricsBufferCount == 1 {
                    print("[VitalsProcessor] ✅ First metricsBuffer received! Processed data is flowing.")
                }
                self.sendMetricsBuffer(metricsBuffer)
            }
        }.store(in: &cancellables)
        
        // Optionally auto-start recording for headless operation
        if autoStartRecording {
            print("[VitalsProcessor] 🎬 Auto-starting processing and recording...")
            startProcessing()
            // Delay recording start to allow processing to initialize
            DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { [weak self] in
                self?.startRecording()
                print("[VitalsProcessor] 🔴 Recording started - metrics should now flow")
            }
        }
    }
    
    /// Send edge metrics (real-time data)
    private func sendEdgeMetrics(_ metrics: Metrics) {
        do {
            let jsonOptions = JSONEncodingOptions()
            // jsonOptions.alwaysPrintPrimitiveFields = true // Not available in this version
            
            // Encode the entire Metrics protobuf object to a JSON string
            let jsonString = try metrics.jsonString(options: jsonOptions)
            
            // Convert back to dictionary to embed in our payload structure
            if let jsonData = jsonString.data(using: .utf8),
               var jsonDict = try JSONSerialization.jsonObject(with: jsonData) as? [String: Any] {
                
                // Add dataType and timestamp
                jsonDict["dataType"] = "edge_metrics"
                jsonDict["timestamp"] = ISO8601DateFormatter().string(from: Date())
                
                // --- Inject Pulse (Missing in EdgeMetrics) ---
                if let buffer = sdk.metricsBuffer, buffer.hasPulse, let lastRate = buffer.pulse.rate.last {
                    // Create a snippet with just the latest rate
                    var pulseSnippet = Presage_Physiology_Pulse()
                    pulseSnippet.rate = [lastRate]
                    // We can also include the last trace point if available, but rate is most important
                    if let lastTrace = buffer.pulse.trace.last {
                        pulseSnippet.trace = [lastTrace]
                    }
                    
                    if let pulseJson = try? pulseSnippet.jsonString(options: jsonOptions),
                       let pulseData = pulseJson.data(using: .utf8),
                       let pulseDict = try? JSONSerialization.jsonObject(with: pulseData) as? [String: Any] {
                        jsonDict["pulse"] = pulseDict
                    }
                }
                
                // --- Inject Face (If missing in EdgeMetrics) ---
                if !metrics.hasFace, let buffer = sdk.metricsBuffer, buffer.hasFace {
                    // Inject latest face data from buffer if we lost it in edge (or if it's sparse)
                    var faceSnippet = Presage_Physiology_Face()
                    
                    if let lastBlink = buffer.face.blinking.last { faceSnippet.blinking = [lastBlink] }
                    if let lastTalk = buffer.face.talking.last { faceSnippet.talking = [lastTalk] }
                    // Landmarks are heavy, maybe skip for fallback? Or just take last.
                    // if let lastLandmarks = buffer.face.landmarks.last { faceSnippet.landmarks = [lastLandmarks] }
                    
                    if let faceJson = try? faceSnippet.jsonString(options: jsonOptions),
                       let faceData = faceJson.data(using: .utf8),
                       let faceDict = try? JSONSerialization.jsonObject(with: faceData) as? [String: Any] {
                        jsonDict["face"] = faceDict
                    }
                }
                
                // --- Inject Breathing Rate (If missing in EdgeMetrics) ---
                // EdgeMetrics usually has upperTrace, but might miss rate updates.
                if let buffer = sdk.metricsBuffer, buffer.hasBreathing, let lastRate = buffer.breathing.rate.last {
                    // Check if current jsonDict["breathing"] exists and has rate
                    var breathingDict = jsonDict["breathing"] as? [String: Any] ?? [:]
                    
                    if breathingDict["rate"] == nil {
                        // Inject rate from buffer
                        var breathingSnippet = Presage_Physiology_Breathing()
                        breathingSnippet.rate = [lastRate]
                        
                        if let snippetJson = try? breathingSnippet.jsonString(options: jsonOptions),
                           let snippetData = snippetJson.data(using: .utf8),
                           let snippetDict = try? JSONSerialization.jsonObject(with: snippetData) as? [String: Any],
                           let rateList = snippetDict["rate"] {
                            
                            breathingDict["rate"] = rateList
                            jsonDict["breathing"] = breathingDict
                        }
                    }
                }
                
                // Send to gateway
                biometricStreamer.send(data: [
                    "id": UUID().uuidString,
                    "content": "Edge metrics update",
                    "session_id": biometricStreamer.sessionId,
                    "data": jsonDict,
                    "metadata": [
                        "source": "iOS",
                        "type": "edge_metrics",
                        "mode": sdk.config.smartSpectraMode == .continuous ? "continuous" : "spot",
                        "timestamp": ISO8601DateFormatter().string(from: Date())
                    ]
                ])
            }
        } catch {
            print("[VitalsProcessor] Failed to serialize Metrics for streaming: \(error)")
        }
    }
    
    /// Send metricsBuffer (processed/aggregated data) with full pulse, breathing, and face metrics
    private func sendMetricsBuffer(_ metricsBuffer: MetricsBuffer) {
        let timestamp = ISO8601DateFormatter().string(from: Date())
        
        // Build the complete data payload with all metrics
        var dataPayload: [String: Any] = [
            "dataType": "metrics_buffer",
            "timestamp": timestamp
        ]
        
        // -- Pulse data --
        var pulseDict: [String: Any] = [:]
        
        // pulse.rate: [{time, value, stable, confidence}]
        let pulseRateArray = metricsBuffer.pulse.rate.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "confidence": measurement.confidence,
                "timestamp": String(measurement.timestamp)
            ]
        }
        pulseDict["rate"] = pulseRateArray
        
        // pulse.trace: [{time, value, stable}]
        let pulseTraceArray = metricsBuffer.pulse.trace.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "timestamp": String(measurement.timestamp)
            ]
        }
        pulseDict["trace"] = pulseTraceArray
        
        // pulse.strict
        if metricsBuffer.pulse.hasStrict {
            pulseDict["strict"] = ["value": metricsBuffer.pulse.strict.value]
        }
        
        dataPayload["pulse"] = pulseDict
        
        // -- Breathing data --
        var breathingDict: [String: Any] = [:]
        
        // breathing.rate: [{time, value, stable, confidence}]
        let breathingRateArray = metricsBuffer.breathing.rate.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "confidence": measurement.confidence,
                "timestamp": String(measurement.timestamp)
            ]
        }
        breathingDict["rate"] = breathingRateArray
        
        // breathing.upperTrace: [{time, value, stable}]
        let upperTraceArray = metricsBuffer.breathing.upperTrace.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "timestamp": String(measurement.timestamp)
            ]
        }
        breathingDict["upperTrace"] = upperTraceArray
        
        // breathing.lowerTrace: [{time, value, stable}]
        let lowerTraceArray = metricsBuffer.breathing.lowerTrace.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "timestamp": String(measurement.timestamp)
            ]
        }
        breathingDict["lowerTrace"] = lowerTraceArray
        
        // breathing.amplitude: [{time, value, stable}]
        let amplitudeArray = metricsBuffer.breathing.amplitude.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "timestamp": String(measurement.timestamp)
            ]
        }
        breathingDict["amplitude"] = amplitudeArray
        
        // breathing.apnea: [{time, detected, stable}]
        let apneaArray = metricsBuffer.breathing.apnea.map { detection -> [String: Any] in
            return [
                "time": detection.time,
                "detected": detection.detected,
                "stable": detection.stable,
                "timestamp": String(detection.timestamp)
            ]
        }
        breathingDict["apnea"] = apneaArray
        
        // breathing.respiratoryLineLength: [{time, value, stable}]
        let respiratoryLineLengthArray = metricsBuffer.breathing.respiratoryLineLength.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "timestamp": String(measurement.timestamp)
            ]
        }
        breathingDict["respiratoryLineLength"] = respiratoryLineLengthArray
        
        // breathing.inhaleExhaleRatio: [{time, value, stable}]
        let inhaleExhaleRatioArray = metricsBuffer.breathing.inhaleExhaleRatio.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "timestamp": String(measurement.timestamp)
            ]
        }
        breathingDict["inhaleExhaleRatio"] = inhaleExhaleRatioArray
        
        // breathing.baseline: [{time, value, stable}]
        let baselineArray = metricsBuffer.breathing.baseline.map { measurement -> [String: Any] in
            return [
                "time": measurement.time,
                "value": measurement.value,
                "stable": measurement.stable,
                "timestamp": String(measurement.timestamp)
            ]
        }
        breathingDict["baseline"] = baselineArray
        
        // breathing.strict
        if metricsBuffer.breathing.hasStrict {
            breathingDict["strict"] = ["value": metricsBuffer.breathing.strict.value]
        }
        
        dataPayload["breathing"] = breathingDict
        
        // -- Face data --
        var faceDict: [String: Any] = [:]
        
        // face.blinking: [{time, detected, stable}]
        let blinkingArray = metricsBuffer.face.blinking.map { detection -> [String: Any] in
            return [
                "time": detection.time,
                "detected": detection.detected,
                "stable": detection.stable,
                "timestamp": String(detection.timestamp)
            ]
        }
        faceDict["blinking"] = blinkingArray
        
        // face.talking: [{time, detected, stable}]
        let talkingArray = metricsBuffer.face.talking.map { detection -> [String: Any] in
            return [
                "time": detection.time,
                "detected": detection.detected,
                "stable": detection.stable,
                "timestamp": String(detection.timestamp)
            ]
        }
        faceDict["talking"] = talkingArray
        
        dataPayload["face"] = faceDict
        
        // -- Blood Pressure data (optional, if needed) --
        if metricsBuffer.hasBloodPressure && !metricsBuffer.bloodPressure.phasic.isEmpty {
            let phasicArray = metricsBuffer.bloodPressure.phasic.map { measurement -> [String: Any] in
                return [
                    "time": measurement.time,
                    "value": measurement.value,
                    "stable": measurement.stable,
                    "confidence": measurement.confidence,
                    "timestamp": String(measurement.timestamp)
                ]
            }
            dataPayload["bloodPressure"] = ["phasic": phasicArray]
        }
        
        // Send to gateway
        biometricStreamer.send(data: [
            "id": UUID().uuidString,
            "content": "Metrics buffer update",
            "session_id": biometricStreamer.sessionId,
            "data": dataPayload,
            "metadata": [
                "source": "iOS",
                "type": "metrics_buffer",
                "mode": sdk.config.smartSpectraMode == .continuous ? "continuous" : "spot",
                "timestamp": timestamp
            ]
        ])
    }

    /// Stop streaming biometric data
    public func stopStreaming() {
        cancellables.forEach { $0.cancel() }
        cancellables.removeAll()
        biometricStreamer.disconnect()
        print("[VitalsProcessor] 🛑 Stopped streaming session: \(biometricStreamer.sessionId)")
    }

    /// Initializes processing pipelines and starts streaming frames.
    public func startProcessing() {
        guard let authHandler = authHandler else {
            print("AuthHandler is not available.")
            return
        }

        authHandler.startAuthWorkflow { [weak self] error in
            guard let self = self else { return }

            if let error = error {
                print("Authentication failed with error: \(error)")
                DispatchQueue.main.async {
                    self.processingStatus = .error
                }
                return
            }

            if self.coreIsRunning {
                self.stopProcessing()
            }
            
            DispatchQueue.main.async {
                self.processingStatus = .idle
            }
            DispatchQueue.global(qos: .userInitiated).async {
                self.setupProcessing()
                self.presageProcessing.start()
                DispatchQueue.main.async {
                    self.coreIsRunning = true
                }
            }
        }
    }

    /// Stops processing and cleans up resources.
    public func stopProcessing() {
        DispatchQueue.global(qos: .userInitiated).async {
            self.presageProcessing.stop()
            DispatchQueue.main.async {
                self.processingStatus = .idle
                self.imageOutput = nil
                self.coreIsRunning = false
            }
        }
    }

    /// Begins recording vitals. Call after ``startProcessing()``.
    public func startRecording() {
        UIApplication.shared.isIdleTimerDisabled = true
        avCaptureDeviceManager.lockCameraSettings()
        setRecordingState(true)
    }

    /// Stops recording vitals and unlocks camera settings.
    public func stopRecording() {
        UIApplication.shared.isIdleTimerDisabled = false
        avCaptureDeviceManager.unlockCameraSettings()
        setRecordingState(false)
    }

}

extension SmartSpectraVitalsProcessor: PresagePreprocessingDelegate {

    /// Called when a new camera frame is available for processing.
    ///
    /// When image output is disabled, this method skips image conversion to optimize performance and reduce memory pressure.
    /// When image output is enabled, frames are converted to UIImage and published via ``imageOutput`` for UI display.
    ///
    /// - Parameters:
    ///   - tracker: The preprocessing instance that captured the frame
    ///   - pixelBuffer: Raw camera frame data as CVPixelBuffer
    ///   - timestamp: Frame timestamp in milliseconds
    public func frameWillUpdate(_ tracker: PresagePreprocessing!, didOutputPixelBuffer pixelBuffer: CVPixelBuffer!, timestamp: Int) {
        // Skip UI image conversion when image output is disabled to optimize performance and reduce memory usage
        guard sdk.config.imageOutputEnabled else { return }

        // Convert the pixel buffer to UIImage asynchronously and publish it for UI display

        //TODO: Need better approach here: Conversion to UIImage is not very efficient, and causes a lot of memory pressure.
        imageConverter.convertAsync(pixelBuffer: pixelBuffer) { image in
            if let image = image {
                DispatchQueue.main.async {
                    self.imageOutput = image
                }
            }
        }

    }

    public func frameDidUpdate(_ tracker: PresagePreprocessing!, didOutputPixelBuffer pixelBuffer: CVPixelBuffer!) {
        // TODO: figure out if we need to keep this or remove it
    }

    // Status delivered as serialized StatusValue protobuf for Swift consumers.
    public func statusBufferChanged(_ tracker: PresagePreprocessing!, serializedBytes: Data) {
        do {
            let status = try Presage_Physiology_StatusValue(serializedBytes: serializedBytes)
            let code: StatusCode = status.value
            if code != lastStatusCode {
                DispatchQueue.main.async {
                    self.lastStatusCode = code
                    self.statusHint = tracker.getStatusHint(fromCodeValue: Int(code.rawValue))
                }
            }
            if sdk.config.smartSpectraMode == .spot && sdk.config.showFps {
                // update fps based on status code in spot mode
                updateFps()
            }
        } catch {
            print("Failed to deserialize StatusValue: \(error.localizedDescription)")
        }
    }

    /// Delegate callback providing processed metrics.
    public func metricsBufferChanged(_ tracker: PresagePreprocessing!, serializedBytes: Data) {
        do {
            // Deserialize the data directly into the Swift Protobuf object
            let metricsBuffer = try MetricsBuffer(serializedBytes: serializedBytes)
            
            // update metrics buffer
            if sdk.config.smartSpectraMode == .spot {
                DispatchQueue.main.async {
                    self.processingStatus = .processed
                }
            }

            DispatchQueue.main.async {
                self.sdk.metricsBuffer = metricsBuffer
            }

//            if sdk.config.smartSpectraMode == .continuous && sdk.config.showFps {
//                //update fps based on metricsBuffer in continuous mode
//                updateFps()
//            }

        } catch {
            print("Failed to deserialize MetricsBuffer: \(error.localizedDescription)")
        }
    }


    /// Delegate callback providing real-time edge metrics for continuous mode.
    public func edgeMetricsChanged(_ tracker: PresagePreprocessing!, serializedBytes: Data) {
        do {
            // Deserialize the data directly into the Swift Protobuf object
            let edgeMetrics = try Metrics(serializedBytes: serializedBytes)
            
            // update metrics buffer
            if sdk.config.smartSpectraMode == .spot {
                DispatchQueue.main.async {
                    self.processingStatus = .processed
                }
            }

            DispatchQueue.main.async {
                self.sdk.edgeMetrics = edgeMetrics
            }

            if sdk.config.smartSpectraMode == .continuous && sdk.config.showFps {
                //update fps based on edgeMetrics in continuous mode
                updateFps()
            }

        } catch {
            print("Failed to deserialize Metrics: \(error.localizedDescription)")
        }
    }

    /// Delegate callback providing countdown updates in seconds.
    public func timerChanged(_ timerValue: Double) {
        if counter != timerValue {
            DispatchQueue.main.async {
                self.counter = timerValue
                if self.counter == 0.0 && self.processingStatus == .idle {
                    self.processingStatus = .processing
                }
            }
        }
    }


    /// Called when the preprocessing graph reports an unrecoverable error.
    public func handleGraphError(_ error: Error) {
        print("Error in vital processing: \(error)")
        self.sdk.updateErrorText("Internal error occurred. Check your internet connection and retry. If it happens repeatedly contact customer support.")
        //clear metrics buffer
        DispatchQueue.main.async {
            self.processingStatus = .error
            self.sdk.metricsBuffer = nil
        }
    }

    /// Calculates a moving-average FPS value based on delegate callbacks.
    fileprivate func updateFps() {
        let currentTime = Int(Date().timeIntervalSince1970 * 1000)

        if let lastTimestamp = lastTimestamp {
            let deltaTime = currentTime - lastTimestamp

            fpsValues.append(deltaTime)
            if fpsValues.count > movingAveragePeriod {
                fpsValues.removeFirst()
            }
            // TODO: 10/28/24: Fix this further upstream so this isn't necessary
            let averageDeltaTime = max(Double(fpsValues.reduce(0, +)) / Double(max(fpsValues.count, 1)), 0.0001)

            DispatchQueue.main.async {
                self.fps = Int(round(1000 / averageDeltaTime))
            }
        }
        lastTimestamp = currentTime
    }
}
