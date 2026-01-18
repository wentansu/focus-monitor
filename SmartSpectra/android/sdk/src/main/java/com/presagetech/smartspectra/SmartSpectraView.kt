package com.presagetech.smartspectra

import android.content.Context
import android.os.Build
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.Button
import android.widget.LinearLayout
import android.widget.Toast
import com.presagetech.smartspectra.SmartSpectraSdk.Companion.isSupportedAbi
import timber.log.Timber

/**
 * Composite view containing the measurement button and a simple result view.
 * This can be added directly to your layout for a quick integration.
 */
class SmartSpectraView (
    context: Context,
    attrs: AttributeSet?
) : LinearLayout(context, attrs) {

    private var smartSpectraButton: SmartSpectraButton
    private var resultView: SmartSpectraResultView

    init {
        // Set LinearLayout orientation to vertical
        orientation = VERTICAL

        // Inflate the view layout
        LayoutInflater.from(context).inflate(R.layout.view_smart_spectra, this, true)

        // Find child views
        smartSpectraButton = findViewById(R.id.smart_spectra_button)
        resultView = findViewById(R.id.result_view)

        // In case of unsupported devices
        if (!isSupportedAbi()) {
            val checkupButton = smartSpectraButton.findViewById<Button>(R.id.button_checkup)
            checkupButton.isEnabled = false
            Toast.makeText(context, "Unsupported device (ABI)", Toast.LENGTH_LONG).show()
            Timber.d("Unsupported device (ABI)")
            Timber.d("This device ABIs: ${Build.SUPPORTED_ABIS.contentToString()}")
        }
    }

}
