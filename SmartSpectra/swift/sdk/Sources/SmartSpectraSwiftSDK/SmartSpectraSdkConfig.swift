import Foundation
import AVFoundation
import PresagePreprocessing

/// Operating modes for the SDK.
public enum SmartSpectraMode {
    /// Perform a single measurement of fixed duration.
    case spot
    /// Continuously measure until stopped.
    case continuous

    // Internal helper to map to PresageMode
    internal var presageMode: PresageMode {
        switch self {
        case .spot:
            return .spot
        case .continuous:
            return .continuous
        }
    }
}

/// Shared configuration used by ``SmartSpectraSwiftSDK`` and
/// ``SmartSpectraVitalsProcessor``.
internal class SmartSpectraSdkConfig: ObservableObject {
    internal static let shared = SmartSpectraSdkConfig()
    /// Operating mode for new measurements.
    @Published internal var smartSpectraMode: SmartSpectraMode
    /// Duration for spot measurements in seconds.
    @Published internal var measurementDuration: Double
    /// API key used when OAuth is not configured.
    internal var apiKey: String?
    /// Display FPS overlay when true.
    internal var showFps: Bool = false
    /// Seconds to wait before capturing begins.
    internal var recordingDelay: Int = 3
    /// Camera position used for capture.
    internal var cameraPosition: AVCaptureDevice.Position = .front
    /// Whether to display built-in UI controls in ``SmartSpectraView``.
    internal var showControlsInScreeningView: Bool = true
    /// Enable generation of UI preview images from camera frames.
    internal var imageOutputEnabled: Bool = true

    // defaults to 30 second spot if configuration is not supplied
    internal init(smartSpectraMode: SmartSpectraMode = .continuous, duration: Double =  30.0) {
        self.smartSpectraMode = smartSpectraMode
        self.measurementDuration = clipValue(duration, minValue: 20.0, maxValue: 120.0)
    }
}

fileprivate func clipValue(_ value: Double, minValue: Double, maxValue: Double) -> Double {
    if value < minValue {
        Logger.log("Warning: duration \(value) is below the minimum value. Clipping to \(minValue).")
        return minValue
    } else if value > maxValue {
        Logger.log("Warning: duration \(value) is above the maximum value. Clipping to \(maxValue).")
        return maxValue
    } else {
        return value
    }
}
