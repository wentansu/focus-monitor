package com.presagetech.smartspectra.utils

import android.content.Context
import android.graphics.ImageFormat
import android.hardware.camera2.CameraAccessException
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraManager
import android.hardware.camera2.CaptureRequest
import android.os.Build
import android.util.Size
import android.view.WindowManager
import androidx.camera.camera2.interop.Camera2CameraControl
import androidx.camera.camera2.interop.CaptureRequestOptions
import androidx.camera.camera2.interop.ExperimentalCamera2Interop
import androidx.camera.core.Camera
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.presagetech.smartspectra.SmartSpectraSdkConfig
import timber.log.Timber
import java.util.concurrent.ExecutorService


@ExperimentalCamera2Interop
internal class MyCameraXPreviewHelper {
    fun interface OnCameraImageProxyListener {
        fun onImageProxy(image: ImageProxy)
    }

    var onCameraImageProxyListener: OnCameraImageProxyListener? = null
    private var preview: Preview? = null
    private var imageAnalyzer: ImageAnalysis? = null
    private var camera: Camera? = null
    private var cameraCharacteristics: CameraCharacteristics? = null
    private var cameraProvider: ProcessCameraProvider? = null
    private lateinit var backgroundExecutor: ExecutorService

    /**
     * Starts the camera preview and analysis pipelines.
     *
     * @param context The application context.
     * @param lifecycleOwner Lifecycle owner for binding use cases.
     * @param previewView The view that renders the camera preview.
     * @param backgroundExecutor Executor used for image analysis callbacks.
     */
    fun startCamera(
        context: Context,
        lifecycleOwner: LifecycleOwner,
        previewView: PreviewView,
        backgroundExecutor: ExecutorService
    ) {

        this.backgroundExecutor = backgroundExecutor
        cameraCharacteristics = getCameraCharacteristics(context, SmartSpectraSdkConfig.cameraPosition)

        val cameraProviderFuture = ProcessCameraProvider.getInstance(context)
        cameraProviderFuture.addListener(
            {
                cameraProvider = cameraProviderFuture.get()
                // Build and bind camera uses cases
                cameraProviderOpened(context, cameraProvider!!, lifecycleOwner, previewView)
            }, ContextCompat.getMainExecutor(context)
        )
        Timber.d("Started camera in the camera preview helper")
    }

    /**
     * Called once the [ProcessCameraProvider] is ready. Sets up the preview and
     * analysis use cases and binds them to the lifecycle.
     */
    private fun cameraProviderOpened(
        context: Context,
        cameraProvider: ProcessCameraProvider,
        lifecycleOwner: LifecycleOwner,
        previewView: PreviewView,
    ) {

        // Get display rotation
        val displayRotation = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            context.display!!.rotation
        } else {
            val windowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
            windowManager.defaultDisplay.rotation
        }

        val cameraSelector = CameraSelector.Builder()
            .requireLensFacing(SmartSpectraSdkConfig.cameraPosition)
            .build()

        // Preview. We are using 720p, so the aspect ratio is 16_9
        preview = Preview.Builder()
            .setTargetResolution(TARGET_SIZE)
            .setTargetRotation(displayRotation)
            .build()

        Timber.d("Display rotation: $displayRotation")

        imageAnalyzer = ImageAnalysis.Builder()
            .setTargetResolution(TARGET_SIZE)
            .setTargetRotation(displayRotation)
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_RGBA_8888)
            .setOutputImageRotationEnabled(true)
            .build().also {
                it.setAnalyzer(backgroundExecutor) { imageProxy ->
                    onCameraImageProxyListener?.onImageProxy(imageProxy)
                }
            }
        cameraProvider.unbindAll()
        try {
            camera = cameraProvider.bindToLifecycle(
                lifecycleOwner,
                cameraSelector,
                preview,
                imageAnalyzer
            )

            val cameraControl = camera?.cameraControl ?: return
            val camera2Control = Camera2CameraControl.from(cameraControl)
            // Initialize in auto mode
            val autoOptions = CaptureRequestOptions.Builder()
                .setCaptureRequestOption(CaptureRequest.CONTROL_MODE, CaptureRequest.CONTROL_MODE_AUTO)
                .setCaptureRequestOption(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON)
                .setCaptureRequestOption(CaptureRequest.CONTROL_AWB_MODE, CaptureRequest.CONTROL_AWB_MODE_AUTO)
                .setCaptureRequestOption(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                .build()

            camera2Control.captureRequestOptions = autoOptions

            //attach viewFinder's surface provide to preview use case
            preview?.surfaceProvider = previewView.surfaceProvider

        } catch (e: Exception) {
            Timber.e("Camera use case binding failed $e")
        }
    }

    /**
     * Unbinds all camera use cases and stops frame delivery.
     */
    fun stopCamera() {
        cameraProvider?.unbindAll()
    }

    /**
     * Toggles the camera's AE, AWB, and AF settings between locked and unlocked states.
     * @param locked If true, locks the settings; if false, unlocks them.
     */
    fun toggleCameraControl(locked: Boolean) {
        Timber.i("Camera settings locked: $locked")
        val cameraControl = camera?.cameraControl ?: return
        val camera2Control = Camera2CameraControl.from(cameraControl)

        val optionsBuilder = CaptureRequestOptions.Builder()
            .setCaptureRequestOption(CaptureRequest.CONTROL_MODE, CaptureRequest.CONTROL_MODE_AUTO)
            .setCaptureRequestOption(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON)
            .setCaptureRequestOption(CaptureRequest.CONTROL_AWB_MODE, CaptureRequest.CONTROL_AWB_MODE_AUTO)
            .setCaptureRequestOption(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE)

        if (locked) {
            // Lock AE and AWB
            optionsBuilder
                .setCaptureRequestOption(CaptureRequest.CONTROL_AE_LOCK, true)
                .setCaptureRequestOption(CaptureRequest.CONTROL_AWB_LOCK, true)
                .setCaptureRequestOption(CaptureRequest.BLACK_LEVEL_LOCK, true)
                // Trigger AF lock. Docs: https://developer.android.com/reference/android/hardware/camera2/CaptureRequest#CONTROL_AF_TRIGGER
                .setCaptureRequestOption(CaptureRequest.CONTROL_AF_TRIGGER, CaptureRequest.CONTROL_AF_TRIGGER_START)
        } else {
            // Unlock AE, AWB, and cancel AF lock
            optionsBuilder
                .setCaptureRequestOption(CaptureRequest.CONTROL_AE_LOCK, false)
                .setCaptureRequestOption(CaptureRequest.CONTROL_AWB_LOCK, false)
                .setCaptureRequestOption(CaptureRequest.BLACK_LEVEL_LOCK, false)
                .setCaptureRequestOption(CaptureRequest.CONTROL_AF_TRIGGER, CaptureRequest.CONTROL_AF_TRIGGER_CANCEL)
        }

        camera2Control.captureRequestOptions = optionsBuilder.build()
    }

    internal companion object {
        // Target frame and view resolution size in landscape.
        val TARGET_SIZE = Size(720,1280)

        /** Returns the [CameraCharacteristics] for the requested lens. */
        private fun getCameraCharacteristics(
            context: Context,
            lensFacing: Int
        ): CameraCharacteristics? {
            val cameraManager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
            try {
                val cameraList = listOf(*cameraManager.cameraIdList)
                for (availableCameraId in cameraList) {
                    val availableCameraCharacteristics =
                        cameraManager.getCameraCharacteristics(availableCameraId)
                    val availableLensFacing =
                        availableCameraCharacteristics.get(CameraCharacteristics.LENS_FACING)
                            ?: continue
                    if (availableLensFacing == lensFacing) {
                        // Check if the target resolution is supported
                        if (!isTargetResolutionSupported(availableCameraCharacteristics, TARGET_SIZE)) {
                            Timber.w("Target resolution ${TARGET_SIZE.width}x${TARGET_SIZE.height} not supported by camera $availableCameraId")
                        }
                        return availableCameraCharacteristics
                    }
                }
            } catch (e: CameraAccessException) {
                Timber.e("Accessing camera ID info got error: $e")
            }
            return null
        }

        /** Checks if the preview resolution is available for the device. */
        private fun isTargetResolutionSupported(
            cameraCharacteristics: CameraCharacteristics,
            targetSize: Size
        ): Boolean {
            val streamConfigurationMap =
                cameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
                    ?: return false
            val outputSizes = streamConfigurationMap.getOutputSizes(ImageFormat.JPEG)
            val rotatedSize = Size(targetSize.height, targetSize.width)
            return outputSizes.contains(targetSize) || outputSizes.contains(rotatedSize)
        }
    }
}
