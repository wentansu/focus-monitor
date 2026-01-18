package com.presagetech.smartspectra

import android.graphics.Bitmap
import org.opencv.android.Utils
import org.opencv.core.Mat
import org.opencv.videoio.VideoCapture
import org.opencv.videoio.Videoio
import java.util.concurrent.Executor
import java.util.concurrent.TimeUnit

class FrameCapture(
    private val videoFilePath: String,
    private val fps: Long = 30L,
    private val debugPrints: Boolean = false
) {

    var processFrames: ((Bitmap, Long) -> Unit)? = null
    var isRunning = false
    private var capture: VideoCapture? = null

    fun startCapturing(executor: Executor) {
        val frameInterval = 1000000L / fps // microseconds (30 fps)
        val capture = VideoCapture(videoFilePath)
        if (!capture.isOpened) throw IllegalStateException("Unable to open video file: $videoFilePath")

        this.capture = capture
        isRunning = true

        val frameWidth = capture.get(Videoio.CAP_PROP_FRAME_WIDTH).toInt()
        val frameHeight = capture.get(Videoio.CAP_PROP_FRAME_HEIGHT).toInt()
        val bitmap = Bitmap.createBitmap(frameWidth, frameHeight, Bitmap.Config.ARGB_8888)
        if (debugPrints) {
            println("Frame dimensions: $frameWidth x $frameHeight")
            println("Bitmap dimensions: ${bitmap.width} x ${bitmap.height}")
        }
        executor.execute {
            val frame = Mat()
            var timestamp = 0L
            val startTime = System.nanoTime()

            try {
                while (isRunning && capture.read(frame)) {
                    Utils.matToBitmap(frame, bitmap)
                    if (debugPrints) {
                        println("Mat size: ${frame.size()}")
                        println("Bitmap dimensions: ${bitmap.width} x ${bitmap.height}")
                    }

                    // Notify frame via callback
                    processFrames?.invoke(bitmap, timestamp)
                    timestamp += frameInterval
                    val elapsedTime = System.nanoTime() - startTime
                    val sleepTime = frameInterval - ((elapsedTime * 1000L) % frameInterval)
                    if (sleepTime > 0) {
                        Thread.sleep(TimeUnit.MICROSECONDS.toMillis(sleepTime))
                    }
                    // release the frame
                    frame.release()
                }
            } finally {
                stopCapturing()
            }
        }
    }

    fun stopCapturing() {
        isRunning = false
        capture?.release()
        capture = null
    }
}
