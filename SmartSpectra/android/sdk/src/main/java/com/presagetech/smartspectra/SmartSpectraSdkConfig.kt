package com.presagetech.smartspectra

import androidx.camera.core.CameraSelector
import timber.log.Timber

internal object SmartSpectraSdkConfig {

    internal val SHOW_OUTPUT_FPS =  false // set this to true if you want to observe rate of metrics from the server
    internal const val preprocessedDataBufferDuration: Double = 0.2
    var SHOW_FPS = false

    // Spot duration
    private const val SPOT_DURATION_DEFAULT: Double = 30.0
    private const val SPOT_DURATION_MIN: Double = 20.0
    private const val SPOT_DURATION_MAX: Double = 120.0

    private var _spotDuration: Double = SPOT_DURATION_DEFAULT

    var spotDuration: Double
        get() = _spotDuration
        set(value) {
            if (value !in SPOT_DURATION_MIN..SPOT_DURATION_MAX) {
                Timber.w("Spot duration must be between $SPOT_DURATION_MIN and $SPOT_DURATION_MAX \nCurrent Spot duration is set to: $_spotDuration")
                return
            }
            _spotDuration = value
        }

    internal var smartSpectraMode: SmartSpectraMode = SmartSpectraMode.CONTINUOUS
    internal var enableEdgeMetrics: Boolean = false
    internal var useFullRangeFaceDetection: Boolean = false
    internal var useFullLandmarks: Boolean = false
    internal var enableLandmarkSegmentation: Boolean = false

    const val ENABLE_BP = false
    const val MODEL_DIRECTORY = "graph/models"
    internal var recordingDelay = 3
    internal var cameraPosition = CameraSelector.LENS_FACING_FRONT
    internal var showControlsInScreeningView = true
    // debug flags
    internal val save_roi_image = false
}

/**
 * Measurement modes supported by [SmartSpectraSdk].
 */
public enum class SmartSpectraMode {
    /** Single spot measurement mode. */
    SPOT,

    /** Continuous measurement mode. */
    CONTINUOUS
}
