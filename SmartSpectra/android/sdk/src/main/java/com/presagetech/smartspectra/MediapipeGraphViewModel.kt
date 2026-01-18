package com.presagetech.smartspectra

import android.content.Context
import android.graphics.Bitmap
import android.os.CountDownTimer
import android.os.SystemClock
import android.view.View
import androidx.camera.core.ImageProxy
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import com.google.mediapipe.components.FrameProcessor
import com.google.mediapipe.framework.AndroidAssetUtil
import com.google.mediapipe.framework.AndroidPacketCreator
import com.google.mediapipe.framework.Packet
import com.google.mediapipe.framework.PacketGetter
import com.google.mediapipe.glutil.EglManager
import com.presage.physiology.Messages
import com.presage.physiology.proto.MetricsProto.MetricsBuffer
import com.presage.physiology.proto.MetricsProto.Metrics
import com.presage.physiology.proto.StatusProto
import timber.log.Timber
import java.lang.ref.WeakReference

internal class MediapipeGraphViewModel private constructor(context: Context) : ViewModel() {
    lateinit var currentSmartSpectraMode: SmartSpectraMode
    private val BINARY_GRAPH_NAME: String
        get () {
            return if (currentSmartSpectraMode == SmartSpectraMode.SPOT ) {
                "metrics_cpu_spot_rest.binarypb"
            } else {
                "metrics_cpu_continuous_rest.binarypb"
            }
        }

    // == input streams
    private val INPUT_VIDEO_STREAM_NAME = "input_video"
    private val RECORDING_STREAM_NAME = "recording"
    // == output streams
    private val STATUS_CODE_STREAM_NAME = "status_code"
    private val TIME_LEFT_STREAM_NAME = "time_left_s"
    private val METRICS_BUFFER_STREAM_NAME = "metrics_buffer"
    private val EDGE_METRICS_STREAM_NAME = "edge_metrics"
    // == input side packets
    private val SPOT_DURATION_SIDE_PACKET_NAME = "spot_duration_s"
    private val ENABLE_BP_SIDE_PACKET_NAME = "enable_phasic_bp"
    private val ENABLE_EDGE_METRICS_PACKET_NAME = "enable_edge_metrics"
    private val ENABLE_DENSE_FACEMESH_POINTS_PACKET_NAME = "enable_dense_facemesh_points"
    private val MODEL_DIRECTORY_SIDE_PACKET_NAME = "model_directory"
    private val USE_FULL_RANGE_FACE_DETECTION_SIDE_PACKET_NAME = "use_full_range_face_detection"
    private val USE_FULL_POSE_LANDMARKS_SIDE_PACKET_NAME = "use_full_pose_landmarks"
    private val ENABLE_POSE_LANDMARK_SEGMENTATION_SIDE_PACKET_NAME = "enable_pose_landmark_segmentation"
    private val API_KEY_SIDE_PACKET_NAME = "api_key"
    private val PREPROCESSED_DATA_BUFFER_DURATION_PACKET_NAME = "preprocessed_data_buffer_duration"
    private val SAVE_ROI_IMAGE = "save_roi_image"

    // Initializes the mediapipe graph and provides access to the input/output streams
    private var eglManager: EglManager? = null
    private var processor: FrameProcessor? = null
    internal lateinit var packetCreator: AndroidPacketCreator

    private val fpsTimestamps: ArrayDeque<Long> = ArrayDeque()
    @Volatile var statusCode: StatusProto.StatusCode = StatusProto.StatusCode.PROCESSING_NOT_STARTED

    private var cameraLockTimeout: Long = 0L

    internal var isRecording: Boolean = false
    private var countdownTimer: CountDownTimer? = null


    private val _processingState = MutableLiveData<ProcessingStatus>().apply { value = ProcessingStatus.DISABLE }
    val processingState: LiveData<ProcessingStatus> get() = _processingState

    // live data for updating the UI
    private val _timeLeft = MutableLiveData<Double>()
    val timeLeft: LiveData<Double> get() = _timeLeft

    private val _hintText = MutableLiveData<String>()
    val hintText: LiveData<String> get() = _hintText

    private val _roundedFps = MutableLiveData<Int>()
    val roundedFps: LiveData<Int> get() = _roundedFps

    private val _countdownLeft = MutableLiveData<Int>()
    val countdownLeft: LiveData<Int> get() = _countdownLeft


    private val viewModel: SmartSpectraSdk by lazy {
        SmartSpectraSdk.getInstance()
    }

    private val contextRef: WeakReference<Context> = WeakReference(context)


    init {
        currentSmartSpectraMode = SmartSpectraSdkConfig.smartSpectraMode
        AndroidAssetUtil.initializeNativeAssetManager(context)
        loadGraphAndResources(context)
    }

    /**
     * Loads the MediaPipe graph and configures all input/output streams.
     */
    private fun loadGraphAndResources(context: Context) {
        val apiKey = try {
            viewModel.getApiKey()
        } catch (e: IllegalStateException) {
            Timber.e("ERROR: API key missing - cannot proceed with processing: ${e.localizedMessage}")
            _processingState.postValue(ProcessingStatus.ERROR)
            return
        }

        eglManager = EglManager(null)
        processor = FrameProcessor(
            context,
            eglManager!!.nativeContext,
            BINARY_GRAPH_NAME,
            null,
            null,
        ).also {
            it.setVideoInputStreamCpu(INPUT_VIDEO_STREAM_NAME)
            it.setInputSidePackets(mapOf(
                SPOT_DURATION_SIDE_PACKET_NAME to it.packetCreator.createFloat64(SmartSpectraSdkConfig.spotDuration),
                ENABLE_BP_SIDE_PACKET_NAME to it.packetCreator.createBool(SmartSpectraSdkConfig.ENABLE_BP),
                ENABLE_DENSE_FACEMESH_POINTS_PACKET_NAME to it.packetCreator.createBool(true),
                ENABLE_EDGE_METRICS_PACKET_NAME to it.packetCreator.createBool(SmartSpectraSdkConfig.enableEdgeMetrics),
                MODEL_DIRECTORY_SIDE_PACKET_NAME to it.packetCreator.createString(SmartSpectraSdkConfig.MODEL_DIRECTORY),
                USE_FULL_RANGE_FACE_DETECTION_SIDE_PACKET_NAME to it.packetCreator.createBool(SmartSpectraSdkConfig.useFullRangeFaceDetection),
                USE_FULL_POSE_LANDMARKS_SIDE_PACKET_NAME to it.packetCreator.createBool(SmartSpectraSdkConfig.useFullLandmarks),
                ENABLE_POSE_LANDMARK_SEGMENTATION_SIDE_PACKET_NAME to it.packetCreator.createBool(SmartSpectraSdkConfig.enableLandmarkSegmentation),
                // TODO: currently always need to set because of some graph changes. only set it for api key based auth once the graph is fixed
                API_KEY_SIDE_PACKET_NAME to it.packetCreator.createString(apiKey),
                PREPROCESSED_DATA_BUFFER_DURATION_PACKET_NAME to it.packetCreator.createFloat64(SmartSpectraSdkConfig.preprocessedDataBufferDuration),
                SAVE_ROI_IMAGE to it.packetCreator.createBool(SmartSpectraSdkConfig.save_roi_image),
            ))
            it.setOnWillAddFrameListener(::handleOnWillAddFrame)
            if (currentSmartSpectraMode == SmartSpectraMode.SPOT ) {
                it.addPacketCallback(TIME_LEFT_STREAM_NAME, ::handleTimeLeftPacket)
            } else if (currentSmartSpectraMode == SmartSpectraMode.CONTINUOUS) {
                it.addPacketCallback(EDGE_METRICS_STREAM_NAME, ::handleEdgeMetricsPacket)
            }
            it.addPacketCallback(STATUS_CODE_STREAM_NAME, ::handleStatusCodePacket)
            it.addPacketCallback(METRICS_BUFFER_STREAM_NAME, ::handleMetricsBufferPacket)
            it.setAsynchronousErrorListener(::handleGraphError)
            it.preheat()
            packetCreator = it.packetCreator
        }
    }

    /**
     * Called when the MediaPipe graph encounters an unrecoverable error.
     * Transitions the processing state to [ProcessingStatus.ERROR].
     */
    internal fun handleGraphError(runtimeException: RuntimeException?) {
        Timber.e("Runtime exception occured in graph: ${runtimeException}")
        _processingState.postValue(ProcessingStatus.ERROR)
    }

    /**
     * Inserts a recording flag packet into the graph for each incoming frame.
     */
    private fun handleOnWillAddFrame(timestamp: Long) {
        val processor = processor ?: throw IllegalStateException()
        val value = (processingState.value == ProcessingStatus.RUNNING) && SystemClock.elapsedRealtime() > cameraLockTimeout
        processor
            .graph
            .addPacketToInputStream(
                RECORDING_STREAM_NAME, processor.packetCreator.createBool(value), timestamp
            )
    }

    /** Updates [_timeLeft] with the latest countdown value from the graph. */
    internal fun handleTimeLeftPacket(packet: Packet?) {
        if (packet == null) return
        _timeLeft.postValue(PacketGetter.getFloat64(packet))

        if (timeLeft.value == 0.0 && processingState.value == ProcessingStatus.RUNNING) {
            _processingState.postValue(ProcessingStatus.PREPROCESSED)
        }
    }


    /**
     * Receives a [MetricsBuffer] proto from the graph and updates observers.
     */
    internal fun handleMetricsBufferPacket(packet: Packet?) {
        if (packet == null) return
        val metricsBuffer = PacketGetter.getProto(packet, MetricsBuffer.parser())

        if (currentSmartSpectraMode == SmartSpectraMode.SPOT) {
            _processingState.postValue(ProcessingStatus.DONE)
            Timber.d("Received spot metrics protobuf")
            Timber.d(metricsBuffer.metadata.toString())
        }

        // forward the metrics buffer to view model
        viewModel.setMetricsBuffer(metricsBuffer)

        // Calculate and set FPS based on statusCode packet for continuous mode
        if (SmartSpectraSdkConfig.SHOW_FPS && (currentSmartSpectraMode == SmartSpectraMode.CONTINUOUS && SmartSpectraSdkConfig.SHOW_OUTPUT_FPS)) {
            calculateAndSetFPS(packet.timestamp)
        }
    }

    /**
     * Receives edge [Metrics] proto from the graph for continuous mode and updates observers.
     */
    internal fun handleEdgeMetricsPacket(packet: Packet?) {
        if (packet == null) return
        val edgeMetrics = PacketGetter.getProto(packet, Metrics.parser())

        // forward the edge metrics to view model
        viewModel.setEdgeMetrics(edgeMetrics)

        // Calculate and set FPS based on edge metrics packet for continuous mode
        if (SmartSpectraSdkConfig.SHOW_FPS && (currentSmartSpectraMode == SmartSpectraMode.CONTINUOUS && SmartSpectraSdkConfig.SHOW_OUTPUT_FPS)) {
            calculateAndSetFPS(packet.timestamp)
        }
    }

    /** Processes graph status updates and adjusts UI state accordingly. */
    internal fun handleStatusCodePacket(packet: Packet?) {
        if (packet == null) return
        val newStatusCodeMessage: StatusProto.StatusValue = PacketGetter.getProto(packet, StatusProto.StatusValue.parser())
        val newStatusCode = newStatusCodeMessage.value

        // Calculate and set FPS based on statusCode packet for spot mode
        if (SmartSpectraSdkConfig.SHOW_FPS && (currentSmartSpectraMode == SmartSpectraMode.SPOT || !SmartSpectraSdkConfig.SHOW_OUTPUT_FPS)) {
            calculateAndSetFPS(packet.timestamp)
        }

        if (newStatusCode != statusCode) {
            statusCode = newStatusCode

            _hintText.postValue(Messages.getStatusHint(statusCode))

            val newProcessingState = if (statusCode == StatusProto.StatusCode.OK) {
                if(isRecording) {
                    ProcessingStatus.RUNNING
                } else {
                    ProcessingStatus.IDLE
                }
            } else {
                ProcessingStatus.DISABLE
            }

            _processingState.postValue(newProcessingState)
        }
    }

    /**
     * Computes and posts a rolling average frames-per-second value based on
     * packet timestamps.
     */
    private fun calculateAndSetFPS(timestamp: Long) {
        fpsTimestamps.addLast(timestamp)
        if (fpsTimestamps.size > 60) {
            fpsTimestamps.removeFirst()
        }

        if (fpsTimestamps.size > 1) {
            val duration = timestamp - fpsTimestamps.first()
            val fps = (1_000_000.0 * (fpsTimestamps.size - 1)) / duration  // Convert interval to FPS, since timestamp is in microseconds
            _roundedFps.postValue(kotlin.math.round(fps).toInt())
        }
    }
    /**
     * Releases graph resources and resets internal state.
     */
    private fun stop() {
        countdownTimer?.cancel()
        stopRecording()
        try {
            processor?.waitUntilIdle()
            processor?.close()
            processor = null
            eglManager?.release()
        } catch (e: Exception) {
            Timber.e("Error while stopping the processor: ${e}")
        }
    }

    /** Starts measurement capture and unlocks the graph. */
    fun startRecording() {
        cameraLockTimeout = SystemClock.elapsedRealtime()
        isRecording = true
        _processingState.postValue(ProcessingStatus.RUNNING)
    }

    /** Stops measurement capture and resets UI state. */
    fun stopRecording() {
        cameraLockTimeout = 0L
        isRecording = false
        _processingState.postValue(ProcessingStatus.IDLE)
    }

    /**
     * Queues an [ImageProxy] for processing. Timestamp conversion to
     * microseconds is handled automatically.
     */
    internal fun addNewFrame(imageProxy: ImageProxy) {
        processor?.onNewFrame(imageProxy.toBitmap(), imageProxy.imageInfo.timestamp / 1000L)
    }

    // add bitmap directly with timestamp
    /** Adds a raw [Bitmap] frame with a specified capture [timestamp]. */
    internal fun addNewFrame(bitmap: Bitmap, timestamp: Long) {
        processor?.onNewFrame(bitmap, timestamp)
    }

    /** Handles record button presses based on the current [processingState]. */
    internal fun recordButtonClickListener(view: View) {
        when (processingState.value) {
            ProcessingStatus.IDLE -> {
                // start auth flow in case we need to refresh token
                AuthHandler.getInstance().startAuthWorkflow()
                startCountdown {
                    startRecording()
                }
            }
            ProcessingStatus.RUNNING -> {
                stopRecording()
            }
            ProcessingStatus.PREPROCESSED -> {
                stopRecording()
                _processingState.postValue(ProcessingStatus.IDLE)
            }
            ProcessingStatus.DISABLE -> {
                Timber.d("Processing is disabled: status code: $statusCode")
            }

            ProcessingStatus.DONE -> {
                Timber.d("Processing is done")
            }
            ProcessingStatus.ERROR -> {
                Timber.e("Processing error")
            }
            ProcessingStatus.COUNTDOWN -> {
                Timber.d("Countdown cancelled. Count down time left: ${countdownLeft.value}s")
                countdownTimer?.cancel()
                _processingState.postValue(ProcessingStatus.IDLE)
            }
            else -> {
                Timber.e("Processing status is null or unrecognized")
            }
        }
    }

    /**
     * Displays a countdown before recording begins. [onCountdownFinish] is
     * invoked when the timer completes.
     */
    internal fun startCountdown(onCountdownFinish: () -> Unit) {
        _processingState.postValue(ProcessingStatus.COUNTDOWN)
        var secondsLeft = SmartSpectraSdkConfig.recordingDelay
        countdownTimer = object : CountDownTimer(secondsLeft * 1000L, 1000) {
            override fun onTick(millisUntilFinished: Long) {
                if (processingState.value == ProcessingStatus.COUNTDOWN) {
                    _countdownLeft.postValue(secondsLeft)
                    secondsLeft -= 1
                } else {
                    countdownTimer?.cancel()
                }
            }

            override fun onFinish() {
                if (processingState.value == ProcessingStatus.COUNTDOWN) {
                    // call the closure if the button state is still countdown
                    onCountdownFinish()
                }
            }
        }.start()
    }

    /** Restarts the MediaPipe graph using the latest configuration. */
    internal fun restart() {
        Timber.d("Restarting mediapipe graph")
        stop()
        currentSmartSpectraMode = SmartSpectraSdkConfig.smartSpectraMode
        this.contextRef.get()?.let {loadGraphAndResources(it) }
        _processingState.postValue(ProcessingStatus.IDLE)
    }

    internal companion object {
        @Volatile
        private var INSTANCE: MediapipeGraphViewModel? = null
        enum class ProcessingStatus {
            IDLE,
            COUNTDOWN,
            RUNNING,
            PREPROCESSED,
            DONE,
            DISABLE,
            ERROR
        }

      fun getInstance(context: Context): MediapipeGraphViewModel {
          return INSTANCE ?: synchronized(this) {
              INSTANCE ?: MediapipeGraphViewModel(context).also { INSTANCE = it }
          }
      }
  }
}
