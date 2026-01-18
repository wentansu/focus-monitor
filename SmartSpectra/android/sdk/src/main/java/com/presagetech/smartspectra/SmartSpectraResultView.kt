package com.presagetech.smartspectra

import android.content.Context
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.LinearLayout
import android.widget.TextView
import com.presage.physiology.proto.MetricsProto.MetricsBuffer
import timber.log.Timber
import kotlin.math.roundToInt

/**
 * View that displays measurement results from [SmartSpectraSdk].
 */
class SmartSpectraResultView(
    context: Context,
    attrs: AttributeSet?
) : LinearLayout(context, attrs) {
    private var resultTextView: TextView
    private var resultErrorTextView: TextView
    private val viewModel: SmartSpectraSdk by lazy {
        SmartSpectraSdk.getInstance()
    }

    init {
        orientation = VERTICAL
        LayoutInflater.from(context).inflate(R.layout.view_result, this, true)
        resultTextView = findViewById(R.id.result_text)
        resultErrorTextView = findViewById(R.id.result_error_text)

        viewModel.setMetricsBufferObserver { metricsBuffer ->
            if (SmartSpectraSdkConfig.smartSpectraMode == SmartSpectraMode.SPOT) {
                updateResultText(metricsBuffer)
            }
        }

        viewModel.errorMessage.observeForever { message ->
            updateErrorText(message)
        }
    }

    /**
     * Renders the pulse and breathing rate from the provided [metricsBuffer].
     *
     * If strict metrics are unavailable a hint is shown to the user.
     */
    private fun updateResultText(metricsBuffer: MetricsBuffer) {
        val strictPulseRate = metricsBuffer.pulse.strict.value.roundToInt()
        val strictBreathingRate = metricsBuffer.breathing.strict.value.roundToInt()
        val breathingRateText = if (strictBreathingRate == 0) "N/A" else "$strictBreathingRate BPM"
        val pulseRateText = if (strictPulseRate == 0) "N/A" else "$strictPulseRate BPM"

        resultTextView.text = context.getString(R.string.result_label, breathingRateText, pulseRateText)

        if (strictPulseRate == 0 || strictBreathingRate == 0) {
            Timber.w("Measurement insufficent for strict. Strict Pulse rate: ${strictPulseRate}, Strict Breathing rate: ${strictBreathingRate}")
            resultErrorTextView.setText(R.string.measurement_failed_hint)
            resultErrorTextView.visibility = VISIBLE
        } else {
            resultErrorTextView.visibility = GONE
        }
    }

    /**
     * Displays an error message below the results view on the UI thread.
     */
    private fun updateErrorText(errorMessage: String) {
        post {
            if (errorMessage.isEmpty()) {
            resultErrorTextView.visibility = GONE
            resultErrorTextView.text = ""
            } else {
            resultErrorTextView.text = "Error: $errorMessage"
            resultErrorTextView.visibility = VISIBLE
            }
        }
    }
}
