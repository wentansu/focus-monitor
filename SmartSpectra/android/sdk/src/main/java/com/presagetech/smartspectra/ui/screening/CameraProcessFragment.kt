package com.presagetech.smartspectra.ui.screening

import android.app.Activity
import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.ImageButton
import android.widget.TextView
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.AppCompatTextView
import androidx.appcompat.widget.Toolbar
import androidx.camera.camera2.interop.ExperimentalCamera2Interop
import androidx.camera.core.CameraSelector
import androidx.camera.view.PreviewView
import androidx.core.view.isVisible
import androidx.fragment.app.Fragment
import com.google.android.material.button.MaterialButton

import com.google.mediapipe.components.PermissionHelper
import com.presagetech.smartspectra.MediapipeGraphViewModel
import com.presagetech.smartspectra.MediapipeGraphViewModel.Companion.ProcessingStatus
import com.presagetech.smartspectra.R
import com.presagetech.smartspectra.SmartSpectraMode
import com.presagetech.smartspectra.SmartSpectraSdk
import com.presagetech.smartspectra.SmartSpectraSdkConfig
import com.presagetech.smartspectra.ui.SmartSpectraActivity
import com.presagetech.smartspectra.utils.MyCameraXPreviewHelper
import timber.log.Timber
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit


@ExperimentalCamera2Interop
internal class CameraProcessFragment : Fragment() {


    private var cameraHelper: MyCameraXPreviewHelper = MyCameraXPreviewHelper()

    private lateinit var timerTextView: TextView
    private lateinit var hintText: TextView
    private lateinit var recordingButton: AppCompatTextView
    private lateinit var fpsTextView: TextView
    private lateinit var previewDisplayView: PreviewView  // frames processed by MediaPipe
    private lateinit var backgroundExecutor: ExecutorService
    private lateinit var screeningPlotView: ScreeningPlotView
    private lateinit var smartSpectraModeToggleButton: MaterialButton
    private lateinit var flipCameraButton: MaterialButton



    private lateinit var mediapipeViewModel: MediapipeGraphViewModel

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        val view = inflater.inflate(R.layout.fragment_camera_process_layout, container, false).also {
            previewDisplayView = it.findViewById(R.id.preview_view)
            timerTextView = it.findViewById(R.id.text_timer)
            hintText = it.findViewById(R.id.text_hint)
            recordingButton = it.findViewById(R.id.button_recording)
            fpsTextView = it.findViewById(R.id.fps_text_view)
            screeningPlotView = it.findViewById(R.id.screeningPlotView)
            smartSpectraModeToggleButton = it.findViewById<MaterialButton>(R.id.button_mode_toggle)
            flipCameraButton = it.findViewById<MaterialButton>(R.id.button_flip_camera)

            setupSmartSpectraModeViews()
        }

        val infoButton = view.findViewById<ImageButton>(R.id.info_button)
        infoButton.setOnClickListener {
            showInfoDialog()
        }

        screeningPlotView.bindLifecycleOwner(viewLifecycleOwner)
        // view controls in screening
        smartSpectraModeToggleButton.isVisible = SmartSpectraSdkConfig.showControlsInScreeningView
        flipCameraButton.isVisible = SmartSpectraSdkConfig.showControlsInScreeningView


        hintText.setText(R.string.loading_hint)

        previewDisplayView.visibility = View.GONE
        if (SmartSpectraSdkConfig.SHOW_FPS) {
            fpsTextView.visibility = View.VISIBLE
        }
        backgroundExecutor = Executors.newSingleThreadExecutor()

        view.findViewById<Toolbar>(R.id.toolbar).also {
            (requireActivity() as AppCompatActivity).setSupportActionBar(it)
            it.setNavigationIcon(R.drawable.ic_arrow_back)
            it.setNavigationOnClickListener { _ ->
                requireActivity().onBackPressedDispatcher.onBackPressed()
            }
        }

        mediapipeViewModel = MediapipeGraphViewModel.getInstance(requireContext())

        // Observe timeLeft LiveData
        mediapipeViewModel.timeLeft.observe(viewLifecycleOwner) { timeLeft ->
            timerTextView.text = timeLeft.toInt().toString()
        }

        // Observe hint text LiveData
        mediapipeViewModel.hintText.observe(viewLifecycleOwner) { hint ->
            hintText.text = hint
        }

        // Observe fps LiveData
        mediapipeViewModel.roundedFps.observe(viewLifecycleOwner) { roundedFps ->
            fpsTextView.text = context?.getString(R.string.fps_label, roundedFps)
        }

        // Observe countdown time
        mediapipeViewModel.countdownLeft.observe(viewLifecycleOwner) { countdownLeft ->
            recordingButton.text = countdownLeft.toString()
        }

        // Observe processing status
        mediapipeViewModel.processingState.observe(viewLifecycleOwner) { processingState ->
            onProcessingStateChanged(processingState)
        }

        recordingButton.setOnClickListener {
            mediapipeViewModel.recordButtonClickListener(it)
        }

        // setup toggle buttons on smartSpectra mode
        smartSpectraModeToggleButton.setOnClickListener {
            SmartSpectraSdkConfig.smartSpectraMode = if (SmartSpectraSdkConfig.smartSpectraMode == SmartSpectraMode.SPOT) SmartSpectraMode.CONTINUOUS else SmartSpectraMode.SPOT
            reset()
        }

        flipCameraButton.setOnClickListener {
            flipCamera()
        }

        return view
    }

    override fun onAttach(context: Context) {
        super.onAttach(context)
        toggleBrightMode(true)
    }

    override fun onDetach() {
        super.onDetach()
        toggleBrightMode(false)
    }

    /**
     * Adjusts screen brightness and keep-screen-on flags while recording.
     */
    private fun toggleBrightMode(on: Boolean) {
        requireActivity().window.also {
            val brightness: Float
            if (on) {
                it.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
                brightness = WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_FULL
            } else {
                it.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
                brightness = WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE
            }
            val params: WindowManager.LayoutParams = it.attributes
            params.screenBrightness = brightness
            it.attributes = params
        }
    }

    /**
     * Restarts camera capture and measurement when the user toggles modes.
     */
    private fun reset() {
        stopCamera()
        mediapipeViewModel.restart()
        setupSmartSpectraModeViews()
        startCamera()
    }

    /**
     * Shows or hides UI elements depending on the current
     * [SmartSpectraSdkConfig.smartSpectraMode].
     */
    private fun setupSmartSpectraModeViews() {
        timerTextView.isVisible = SmartSpectraSdkConfig.smartSpectraMode != SmartSpectraMode.CONTINUOUS
        screeningPlotView.isVisible = SmartSpectraSdkConfig.smartSpectraMode != SmartSpectraMode.SPOT
    }

    /**
     * Displays a short tip describing correct camera positioning for best
     * measurements.
     */
    private fun showInfoDialog() {
        val builder = AlertDialog.Builder(requireContext())
        builder.setTitle("Tip")
        builder.setMessage("Please ensure the subject’s face, shoulders, and upper chest are in view and remove any clothing that may impede visibility. Please refer to Instructions For Use for more information.")
        builder.setPositiveButton("OK") { dialog, _ ->
            dialog.dismiss()
        }
        builder.show()
    }

    override fun onResume() {
        super.onResume()

        if (!PermissionHelper.cameraPermissionsGranted(requireActivity())) {
            throw RuntimeException("Handle camera permission in host activity")
        }

        if (mediapipeViewModel.currentSmartSpectraMode != SmartSpectraSdkConfig.smartSpectraMode) {
            reset()
        }

        startCamera()
    }

    override fun onPause() {
        super.onPause()
        cameraHelper.stopCamera()
        mediapipeViewModel.stopRecording()
        // Hide preview display until we re-open the camera again.
        previewDisplayView.visibility = View.GONE
    }

    override fun onDestroy() {
        super.onDestroy()
        Timber.d("camera process fragment destroyed")
        //release the resources
        stopCamera()
        backgroundExecutor.shutdown()
        backgroundExecutor.awaitTermination(
            Long.MAX_VALUE,
            TimeUnit.NANOSECONDS
        )
    }

    /** Starts the camera preview and begins streaming frames to the graph. */
    private fun startCamera() {
        cameraHelper.startCamera(
            requireActivity(),
            viewLifecycleOwner,
            previewDisplayView,
            backgroundExecutor
        )
        // show preview
        previewDisplayView.visibility = View.VISIBLE
        if (cameraHelper.onCameraImageProxyListener == null) {
            cameraHelper.onCameraImageProxyListener = processImageFrames
        }
    }

    /** Stops the camera and removes any frame listener. */
    private fun stopCamera() {
        cameraHelper.stopCamera()
        cameraHelper.onCameraImageProxyListener = null
    }

    /** Swaps between front and back cameras and restarts preview. */
    private fun flipCamera() {
        cameraHelper.stopCamera()
        SmartSpectraSdkConfig.cameraPosition = if (SmartSpectraSdkConfig.cameraPosition == CameraSelector.LENS_FACING_FRONT) CameraSelector.LENS_FACING_BACK else CameraSelector.LENS_FACING_FRONT
        startCamera()
    }

    private val processImageFrames = MyCameraXPreviewHelper.OnCameraImageProxyListener { imageProxy ->
        // passing timestamp as micro-second
        imageProxy.use {
            mediapipeViewModel.addNewFrame(imageProxy)
        }
    }

    /**
     * Updates the UI to reflect the latest measurement [newState].
     */
    private fun onProcessingStateChanged(newState: ProcessingStatus) {
        when (newState) {
            ProcessingStatus.IDLE -> {
                smartSpectraModeToggleButton.isEnabled = true
                flipCameraButton.isEnabled = true
                recordingButton.text = getString(R.string.record)
                recordingButton.textSize = 20.0f
                recordingButton.setBackgroundResource(R.drawable.record_background)
                cameraHelper.toggleCameraControl(locked = false)
            }

            ProcessingStatus.COUNTDOWN -> {
                recordingButton.text = SmartSpectraSdkConfig.recordingDelay.toString()
                recordingButton.setBackgroundResource(R.drawable.record_background)
                recordingButton.textSize = 40.0f
                cameraHelper.toggleCameraControl(locked = true)
            }

            ProcessingStatus.RUNNING -> {
                smartSpectraModeToggleButton.isEnabled = false
                flipCameraButton.isEnabled = false
                recordingButton.text = getString(R.string.stop)
                recordingButton.textSize = 20.0f
                recordingButton.setBackgroundResource(R.drawable.record_background)
            }

            ProcessingStatus.PREPROCESSED -> {
                cameraHelper.toggleCameraControl(locked = false)
                Timber.d("Presage Processed")
                // TODO: Graph needs to move to a service for this to not hang until graph is done here
                (requireActivity() as SmartSpectraActivity).openUploadFragment()
                //stop camera when done preprocessing
                stopCamera()
            }

            ProcessingStatus.DONE -> {
                Timber.d("Got metrics buffer.")
                // clear error message from the sdk assuming DONE shouldn't happen otherwise
                SmartSpectraSdk.getInstance().setErrorMessage("")
                requireActivity().let {
                    it.setResult(Activity.RESULT_OK)
                    it.finish()
                }
                mediapipeViewModel.restart()
            }

            ProcessingStatus.DISABLE -> {
                recordingButton.text = getString(R.string.record)
                recordingButton.textSize = 20.0f
                recordingButton.setBackgroundResource(R.drawable.record_background_disabled)
            }

            ProcessingStatus.ERROR -> {
                Timber.e("Presage Processing error")
                requireActivity().let {
                    it.setResult(Activity.RESULT_CANCELED)
                    it.finish()
                }
                mediapipeViewModel.restart()
                SmartSpectraSdk.getInstance().setErrorMessage(requireContext().getString(R.string.graph_error_hint))
                // clear existng metrics buffer
                SmartSpectraSdk.getInstance().clearMetricsBuffer()
            }


        }
    }

}
