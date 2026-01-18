package com.presagetech.smartspectra

import android.content.Context
import android.graphics.Bitmap
import android.os.Looper
import android.os.SystemClock
import androidx.arch.core.executor.ArchTaskExecutor
import androidx.arch.core.executor.TaskExecutor
import androidx.lifecycle.Observer
import androidx.test.core.app.ApplicationProvider
import com.presagetech.smartspectra.MediapipeGraphViewModel.Companion.ProcessingStatus
import org.junit.*
import org.mockito.*

import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.ext.junit.runners.AndroidJUnit4

import org.junit.runner.RunWith
//import androidx.arch.core.executor.testing.InstantTaskExecutorRule
//import org.junit.Rule
import org.junit.Test

import org.junit.Assert.*
import org.junit.rules.TestRule
import org.junit.runners.model.Statement
import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.util.concurrent.Executors

/**
 * Instrumented test, which will execute on an Android device.
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
@RunWith(AndroidJUnit4::class)
class MediapipeGraphViewModelInstrumentedTest {

   @get:Rule
   val instantTaskExecutorRule = TestRule { base, _ ->
       object : Statement() {
           override fun evaluate() {
               ArchTaskExecutor.getInstance().setDelegate(object : TaskExecutor() {
                   override fun executeOnDiskIO(runnable: Runnable) = runnable.run()
                   override fun postToMainThread(runnable: Runnable) = runnable.run()
                   override fun isMainThread() = true
               })
               try {
                   base.evaluate()
               } finally {
                   ArchTaskExecutor.getInstance().setDelegate(null)
               }
           }
       }
   }

    @Mock
    private lateinit var context: Context

//    @Mock
//    private lateinit var imageProxy: ImageProxy

//    @Mock
//    private lateinit var metricsBuffer: MetricsProto.MetricsBuffer

//    @Mock
//    private lateinit var statusProto: StatusProto.StatusValue

    private lateinit var viewModel: MediapipeGraphViewModel
    private lateinit var smartSpectraSdk: SmartSpectraSdk

    @Before
    fun setUp() {
        MockitoAnnotations.openMocks(this)
        System.loadLibrary("mediapipe_jni")
        // Set smartSpectra mode to continuous
        // TODO: Configure a continuous mode and spot mode
        SmartSpectraSdkConfig.smartSpectraMode = SmartSpectraMode.CONTINUOUS
        context = ApplicationProvider.getApplicationContext()
        smartSpectraSdk = SmartSpectraSdk.getInstance()
        
        // Get API key from instrumentation arguments
        val apiKey = InstrumentationRegistry.getArguments().getString("physiologyApiKey")
        assertNotNull("PHYSIOLOGY_API_KEY must be set in environment", apiKey)
        assertFalse("PHYSIOLOGY_API_KEY cannot be empty", apiKey.isNullOrEmpty())
        smartSpectraSdk.setApiKey(apiKey!!)
        
        viewModel = MediapipeGraphViewModel.getInstance(context)
        assertNotNull(viewModel)
    }

    @Before
    fun setupLooper() {
        if (Looper.myLooper() == null) {
            Looper.prepare()
        }
    }

    @After
    fun tearDown() {
        //TODO: Do a better teardown implementation
//        viewModel.stop()
    }

    @Test
    fun testInitialization() {
        assertNotNull(viewModel)
    }

    @Test
    fun testStartRecording() {
        viewModel.startRecording()
        assertTrue(viewModel.isRecording)
        assertEquals(ProcessingStatus.RUNNING, viewModel.processingState.value)
    }

    @Test
    fun testStopRecording() {
        viewModel.stopRecording()
        assertFalse(viewModel.isRecording)
        assertEquals(ProcessingStatus.IDLE, viewModel.processingState.value)
    }

    @Test
    fun testHandleTimeLeftPacket() {
        viewModel.startRecording()
        val packet = viewModel.packetCreator.createFloat64(0.0)
        viewModel.handleTimeLeftPacket(packet)
        assertEquals(0.0, viewModel.timeLeft.value)
        if (SmartSpectraSdkConfig.smartSpectraMode == SmartSpectraMode.SPOT) {
            assertEquals(ProcessingStatus.PREPROCESSED, viewModel.processingState.value)
        }
    }

//    @Test
//    fun testHandleDenseMeshPacket() {
//        val denseMeshPoints = listOf<Short>(1, 2, 3)
//        Mockito.`when`(PacketGetter.getInt16Vector(packet)).thenReturn(denseMeshPoints)
//        viewModel.handleDenseMeshPacket(packet)
//        assertEquals(denseMeshPoints, viewModel.viewModel.getDenseMeshPoints())
//    }

//    @Test
//    fun testHandleMetricsBufferPacket() {
//        Mockito.`when`(PacketGetter.getProto(packet, MetricsProto.MetricsBuffer.parser())).thenReturn(metricsBuffer)
//        viewModel.handleMetricsBufferPacket(packet)
//        assertEquals(metricsBuffer, metricsViewModel.metricsBuffer)
//    }

//    @Test
//    fun testHandleStatusCodePacket() {
//        Mockito.`when`(PacketGetter.getProto(packet, StatusProto.StatusValue.parser())).thenReturn(statusProto)
//        Mockito.`when`(statusProto.value).thenReturn(StatusProto.StatusCode.OK)
//        viewModel.handleStatusCodePacket(packet)
//        assertEquals(StatusProto.StatusCode.OK, viewModel.statusCode)
//    }

//    @Test
//    fun testAddNewFrame() {
//        val bitmap = Mockito.mock(Bitmap::class.java)
//        Mockito.`when`(imageProxy.toBitmap()).thenReturn(bitmap)
//        Mockito.`when`(imageProxy.imageInfo.timestamp).thenReturn(1000L)
//        viewModel.addNewFrame(imageProxy)
//        Mockito.verify(frameProcessor).onNewFrame(bitmap, 1L)
//    }

    @Test
    fun testStartCountdown() {
        val observer = Mockito.mock(Observer::class.java) as Observer<Int>
        InstrumentationRegistry.getInstrumentation().runOnMainSync {
            viewModel.countdownLeft.observeForever(observer)
            viewModel.startCountdown { }
        }
        SystemClock.sleep(2000)
        Mockito.verify(observer, Mockito.atLeastOnce()).onChanged(Mockito.anyInt())
    }

    @Test
    fun testProcessVideoFramesAsyncCapture() {
        // make sure to execute run_android_tests.sh if the video file is not found
        // comment out the file cleanup if you are running from android studio but make sure to manually delete the file afterwards
        val videoFilePath = "/data/local/tmp/STLW_0261_econ_cropped_s66_d32_mjpeg.avi"
        val videoFile = File(videoFilePath)
        if (!videoFile.exists()) {
            throw FileNotFoundException("Video file not found at $videoFilePath. Make sure to execute run_android_tests.sh after connecting your phone to android studio")
        }

        val frameCapture = FrameCapture(videoFile.absolutePath, debugPrints = false)
        val executor = Executors.newSingleThreadExecutor()
        val save_roi_image = false

        viewModel.startRecording()
        assertEquals(ProcessingStatus.RUNNING, viewModel.processingState.value)

        frameCapture.processFrames = { bitmap, timestamp ->
            viewModel.addNewFrame(bitmap, timestamp)
            if (save_roi_image) {
                // use sparingly as it will cause noticeable slowdown
                // saved file would be located at /data/data/com.presagetech.smartspectra.test/files
                val filePath = "${context.filesDir}/test_image.png"
                saveBitmap(bitmap, filePath)
            }
        }

        frameCapture.startCapturing(executor)

        val startTime = SystemClock.elapsedRealtime()
        while (frameCapture.isRunning) {
            println("Rounded FPS: ${viewModel.roundedFps.value}")
            println("Hint Text: ${viewModel.hintText.value}")
            println("Processing State: ${viewModel.processingState.value}")
            println("Time Left: ${viewModel.timeLeft.value}")
            if (SmartSpectraSdkConfig.smartSpectraMode == SmartSpectraMode.SPOT && viewModel.timeLeft.value != 0.0) {
                frameCapture.stopCapturing()
            }

            if (SystemClock.elapsedRealtime() - startTime > 5000) {
                val hintText = viewModel.hintText.value
                if (hintText == null) {
                    throw AssertionError("Hint Text is null after a few seconds. Indicating an issue with the processing")
                } else if (hintText.contains("No face found", ignoreCase = true)) {
                    throw AssertionError("'No face found' after a few seconds. Indicating an issue with the processing")
                }
            }
        }

        frameCapture.stopCapturing()
        executor.shutdown()
        viewModel.stopRecording()
        assertEquals(ProcessingStatus.IDLE, viewModel.processingState.value)
    }

    private fun saveBitmap(bitmap: Bitmap, filePath: String) {
        val file = File(filePath)
        var out: FileOutputStream? = null
        try {
            out = FileOutputStream(file)
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out)
        } catch (e: IOException) {
            e.printStackTrace()
        } finally {
            out?.close()
        }
    }
}
