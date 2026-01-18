package com.presagetech.smartspectra_example


import android.graphics.Color
import android.graphics.Typeface
import android.os.Bundle
import android.view.Gravity
import android.widget.LinearLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector

// Plotting imports
import androidx.core.view.isVisible
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.charts.ScatterChart
import com.github.mikephil.charting.components.XAxis
import com.github.mikephil.charting.components.YAxis
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.github.mikephil.charting.data.ScatterData
import com.github.mikephil.charting.data.ScatterDataSet
import com.google.android.material.button.MaterialButton

// SmartSpectra SDK Specific Imports
import com.presagetech.smartspectra.SmartSpectraView
import com.presagetech.smartspectra.SmartSpectraMode
import com.presagetech.smartspectra.SmartSpectraSdk
import com.presage.physiology.proto.MetricsProto.MetricsBuffer
import com.presage.physiology.proto.MetricsProto.Metrics


class MainActivity : AppCompatActivity() {
    private lateinit var smartSpectraView: SmartSpectraView
    private lateinit var buttonContainer: LinearLayout
    private lateinit var chartContainer: LinearLayout
    private lateinit var faceMeshContainer: ScatterChart

    // App display configurations
    private val isCustomizationEnabled: Boolean = true
    private val isFaceMeshEnabled: Boolean = true


    // SmartSpectra SDK settings
    // define smartSpectra mode to SPOT or CONTINUOUS. Defaults to CONTINUOUS when not set
    private var smartSpectraMode = SmartSpectraMode.CONTINUOUS
    // define front or back camera to use
    private var cameraPosition = CameraSelector.LENS_FACING_FRONT
    // measurement duration (valid ranges are between 20.0 and 120.0) Defaults to 30.0 when not set
    // For continuous SmartSpectra mode currently defaults to infinite
    private var measurementDuration = 30.0

    // (Required) Authentication. Only need to use one of the two options: API Key or OAuth below
    // Authentication with OAuth is currently only supported for apps in the Play Store
    // Option 1: (Authentication with API Key) Set the API key. Obtain the API key from https://physiology.presagetech.com. Leave default or remove if you want to use OAuth. OAuth overrides the API key.
    private var apiKey = "YOUR_API_KEY"

    // Option 2: (OAuth) If you want to use OAuth, copy the OAuth config (`presage_services.xml`) from PresageTech's developer portal (<https://physiology.presagetech.com/>) to your src/main/res/xml/ directory.
    // No additional code is needed for OAuth.

    // get instance of SmartSpectraSdk and apply optional configurations
    private val smartSpectraSdk: SmartSpectraSdk = SmartSpectraSdk.getInstance().apply {
        //Required configurations: Authentication
        setApiKey(apiKey) // Use this if you are authenticating with an API key
        // If OAuth is configured, it will automatically override the API key

        // Optional configurations
        // Valid range for spot time is between 20.0 and 120.0
        setMeasurementDuration(measurementDuration)
        setShowFps(false)
        //Recording delay defaults to 3 if not provided
        setRecordingDelay(3)

        // smartSpectra mode (SPOT or CONTINUOUS. Defaults to CONTINUOUS when not set)
        setSmartSpectraMode(smartSpectraMode)

        // select camera (front or back, defaults to front when not set)
        setCameraPosition(cameraPosition)

        // Optional: Only need to set it if you want to access metrics to do any processing
        setMetricsBufferObserver { metricsBuffer ->
            handleMetricsBuffer(metricsBuffer)
        }

        // Optional: Only need to set it if you want to access edge metrics and dense face landmarks
        if (isFaceMeshEnabled || smartSpectraMode == SmartSpectraMode.CONTINUOUS) {
            setEdgeMetricsObserver { edgeMetrics ->
                handleEdgeMetrics(edgeMetrics)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Setting up SmartSpectra Results/Views
        smartSpectraView = findViewById(R.id.smart_spectra_view)
        // button container for extra buttons
        buttonContainer = findViewById(R.id.button_container)
        // setup views for plots
        chartContainer = findViewById(R.id.chart_container)
        faceMeshContainer = findViewById(R.id.mesh_container)

        // (optional) toggle display of camera and smartspectra mode controls in screening view
        smartSpectraSdk.showControlsInScreeningView(isCustomizationEnabled)

        if (isCustomizationEnabled) {
            // Example button to switch SmartSpectraMode
            val smartSpectraModeButton = MaterialButton(
                this,
                null,
                com.google.android.material.R.attr.materialIconButtonStyle
            ).apply {
                text =
                    "Switch SmartSpectra Mode to ${if (smartSpectraMode == SmartSpectraMode.SPOT) "CONTINUOUS" else "SPOT"}"
                setIconResource(if (smartSpectraMode == SmartSpectraMode.SPOT) R.drawable.ic_line_chart else R.drawable.ic_scatter_plot)
                iconGravity = MaterialButton.ICON_GRAVITY_TEXT_START
                iconPadding = 10
            }
            smartSpectraModeButton.setOnClickListener {
                if (smartSpectraMode == SmartSpectraMode.CONTINUOUS) {
                    smartSpectraMode = SmartSpectraMode.SPOT
                    smartSpectraModeButton.text = "Switch SmartSpectra Mode to CONTINUOUS"
                    smartSpectraModeButton.setIconResource(R.drawable.ic_line_chart)
                } else {
                    smartSpectraMode = SmartSpectraMode.CONTINUOUS
                    smartSpectraModeButton.text = "Switch SmartSpectra Mode to SPOT"
                    smartSpectraModeButton.setIconResource(R.drawable.ic_scatter_plot)
                }
                smartSpectraSdk.setSmartSpectraMode(smartSpectraMode)
            }

            // Add the button to the layout
            buttonContainer.addView(smartSpectraModeButton)

            // Example button to switch camera position
            val cameraPositionButton = MaterialButton(
                this,
                null,
                com.google.android.material.R.attr.materialIconButtonStyle
            ).apply {
                text =
                    "Switch Camera to ${if (cameraPosition == CameraSelector.LENS_FACING_FRONT) "BACK" else "FRONT"}"
                setIconResource(R.drawable.ic_flip_camera)
                iconGravity = MaterialButton.ICON_GRAVITY_TEXT_START
                iconPadding = 10
            }
            cameraPositionButton.setOnClickListener {
                if (cameraPosition == CameraSelector.LENS_FACING_FRONT) {
                    cameraPosition = CameraSelector.LENS_FACING_BACK
                    cameraPositionButton.text = "Switch Camera to FRONT"
                } else {
                    cameraPosition = CameraSelector.LENS_FACING_FRONT
                    cameraPositionButton.text = "Switch Camera to BACK"
                }
                smartSpectraSdk.setCameraPosition(cameraPosition)
            }

            // Add the button to the layout
            buttonContainer.addView(cameraPositionButton)

            // Example buttons to change measurement duration
            val measurementDurationButtonRow = createMeasurementButtonRow { newDuration ->
                smartSpectraSdk.setMeasurementDuration(newDuration)
            }
            buttonContainer.addView(measurementDurationButtonRow)
        }

    }

    private fun handleEdgeMetrics(edgeMetrics: Metrics) {
        // Handle dense face landmarks from edge metrics if face mesh is enabled
        if (isFaceMeshEnabled && edgeMetrics.hasFace() && edgeMetrics.face.landmarksCount > 0) {
            // Get the latest landmarks from edge metrics
            val latestLandmarks = edgeMetrics.face.landmarksList.lastOrNull()
            latestLandmarks?.let { landmarks ->
                val meshPoints = landmarks.valueList.map { landmark ->
                    Pair(landmark.x.toInt(), landmark.y.toInt())
                }
                handleMeshPoints(meshPoints)
            }
        }
    }

    private fun handleMeshPoints(meshPoints: List<Pair<Int, Int>>) {
        // TODO: Update UI or handle the points as needed

        // Reference the ScatterChart from the layout
        val chart = faceMeshContainer
        chart.isVisible = true


        // Scale the points and sort by x
        // Sorting is important here for scatter plot as unsorted points cause negative array size exception in scatter chart
        // See https://github.com/PhilJay/MPAndroidChart/issues/2074#issuecomment-239936758
        // --- Important --- we are subtracting the y points for plotting since (0,0) is top-left on the screen but bottom-left on the chart
        // --- Important --- we are subtracting the x points to mirror horizontally
        val scaledPoints = meshPoints.map { Entry(1f - it.first / 720f, 1f - it.second / 720f) }
            .sortedBy { it.x }

        // Create a dataset and add the scaled points
        val dataSet = ScatterDataSet(scaledPoints, "Mesh Points").apply {
            setDrawValues(false)
            scatterShapeSize = 15f
            setScatterShape(ScatterChart.ScatterShape.CIRCLE)
        }

        // Create ScatterData with the dataset
        val scatterData = ScatterData(dataSet)

        // Customize the chart
        chart.apply {
            data = scatterData
            axisLeft.isEnabled = false
            axisRight.isEnabled = false
            xAxis.isEnabled = false
            setTouchEnabled(false)
            description.isEnabled = false
            legend.isEnabled = false

            // Set visible range to make x and y axis have the same range

            setVisibleXRange(0f, 1f)
            setVisibleYRange(0f, 1f, YAxis.AxisDependency.LEFT)

            // Move view to the data
            moveViewTo(0f, 0f, YAxis.AxisDependency.LEFT)
        }

        // Refresh the chart
        chart.invalidate()
    }

    private fun handleMetricsBuffer(metrics: MetricsBuffer) {
        // Clear the chart container before plotting new results
        chartContainer.removeAllViews()

        if (metrics.hasMetadata()) {
            // Create TextView for metadata ID
            val idTextView = TextView(chartContainer.context).apply {
                text = "ID: ${metrics.metadata.id}"
                textSize = 16f
                setPadding(16, 8, 16, 8)
            }

            // Create TextView for metadata upload timestamp
            val timestampTextView = TextView(chartContainer.context).apply {
                text = "Upload Timestamp: ${metrics.metadata.uploadTimestamp}"
                textSize = 16f
                setPadding(16, 8, 16, 8)
            }

            // Add the TextViews to the chart container
            chartContainer.addView(idTextView)
            chartContainer.addView(timestampTextView)
        }

        // get the relevant metrics
        val pulse = metrics.pulse
        val breathing = metrics.breathing
        val bloodPressure = metrics.bloodPressure
        val face = metrics.face

        // Plot the results

        // Pulse plots
        if (pulse.traceCount > 0) {
            addChart(pulse.traceList.map { Entry(it.time, it.value) },  "Pulse Pleth", false)
        }
        if (pulse.rateCount > 0) {
            addChart( pulse.rateList.map { Entry(it.time, it.value) }, "Pulse Rates", true)
            addChart( pulse.rateList.map { Entry(it.time, it.confidence) }, "Pulse Rate Confidence", true)

        }
        //TODO: 9/30/24: add this chart when hrv is added to protobuf
//        addChart( pulse..hrv.map { Entry(it.time, it.value) }, "Pulse Rate Variability", true)

        // Breathing plots
        if (breathing.upperTraceCount > 0) {
            addChart(breathing.upperTraceList.map { Entry(it.time, it.value) }, "Breathing Pleth", false)
        }
        if (breathing.rateCount > 0) {
            addChart(breathing.rateList.map { Entry(it.time, it.value) }, "Breathing Rates", true)
            addChart(breathing.rateList.map { Entry(it.time, it.confidence) }, "Breathing Rate Confidence", true)
        }
        if (breathing.amplitudeCount > 0) {
            addChart(breathing.amplitudeList.map { Entry(it.time, it.value) }, "Breathing Amplitude", true)
        }
        if (breathing.apneaCount > 0) {
            addChart(breathing.apneaList.map { Entry(it.time, if (it.detected) 1f else 0f) }, "Apnea", true)
        }
        if (breathing.baselineCount > 0) {
            addChart(breathing.baselineList.map { Entry(it.time, it.value) }, "Breathing Baseline", true)
        }
        if (breathing.respiratoryLineLengthCount > 0) {
            addChart(breathing.respiratoryLineLengthList.map { Entry(it.time, it.value) }, "Respiratory Line Length", true)
        }
        if (breathing.inhaleExhaleRatioCount > 0) {
            addChart(
                breathing.inhaleExhaleRatioList.map { Entry(it.time, it.value) },
                "Inhale-Exhale Ratio",
                true
            )
        }

        // Blood pressure plots
        if (bloodPressure.phasicCount > 0) {
            addChart(bloodPressure.phasicList.map { Entry(it.time, it.value) }, "Phasic", true)
        }

        // Face plots
        if (face.blinkingCount > 0) {
            addChart(face.blinkingList.map { Entry(it.time, if (it.detected) 1f else 0f) }, "Blinking", true)
        }
        if (face.talkingCount > 0) {
            addChart(face.talkingList.map { Entry(it.time, if (it.detected) 1f else 0f) }, "Talking", true)
        }

    }

    private fun addChart(entries: List<Entry>, title: String, showYTicks: Boolean) {
        val chart = LineChart(this)

        val density = resources.displayMetrics.density
        val heightInPx = (200 * density).toInt()

        chart.layoutParams = LinearLayout.LayoutParams (
            LinearLayout.LayoutParams.MATCH_PARENT,
            heightInPx
        )


        val titleView = TextView(this)
        titleView.text = title
        titleView.textSize = 18f
        titleView.gravity = Gravity.CENTER
        titleView.setTypeface(null, Typeface.BOLD)

        val xLabelView = TextView(this)
        xLabelView.setText(R.string.api_xLabel)
        xLabelView.gravity = Gravity.CENTER
        xLabelView.setPadding(0, 0, 0, 20)

        chartContainer.addView(titleView)
        chartContainer.addView(chart)
        chartContainer.addView(xLabelView)

        dataPlotting(chart, entries, showYTicks)
    }

    /**
     * Configures and displays a line chart with the provided data entries.
     * This function sets up the line chart to show a simplified and clean visualization,
     * removing unnecessary visual elements like grid lines, axis lines, labels, and legends.
     * It sets the line color to red and ensures that no markers or value texts are shown.
     *
     * @param chart The LineChart object to configure and display data on.
     * @param entries The list of Entry objects representing the data points to be plotted.
     * @param showYTicks Whether to show the Y axis ticks
     */
    private fun dataPlotting(chart: LineChart, entries: List<Entry>, showYTicks: Boolean) {
        val dataSet = LineDataSet(entries, "Data")

        // Clean up line
        dataSet.setDrawValues(false)
        dataSet.setDrawCircles(false)
        dataSet.color = Color.RED

        chart.data = LineData(dataSet)

        // x axis setup
        chart.xAxis.position = XAxis.XAxisPosition.BOTTOM
        chart.xAxis.setDrawGridLines(false)
        chart.xAxis.setDrawAxisLine(true)
        chart.xAxis.granularity = 1.0f


        // y axis setup
        chart.axisLeft.setPosition(YAxis.YAxisLabelPosition.OUTSIDE_CHART)
        chart.axisLeft.setDrawZeroLine(false)
        chart.axisLeft.setDrawGridLines(false)
        chart.axisLeft.setDrawAxisLine(true)
        chart.axisLeft.setDrawLabels(showYTicks)

        // chart specific setup
        chart.axisRight.isEnabled = false
        chart.legend.isEnabled = false
        chart.description.isEnabled = false
        chart.onTouchListener = null
        chart.invalidate()

    }

    private fun createMeasurementButtonRow(
        onMeasurementDurationChanged: (Double) -> Unit
    ): LinearLayout {
        val buttonRow = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                setMargins(16, 0, 16, 0) // left, top, right, bottom margins in pixels
            }
        }

        val measurementDurationTextView = TextView(this).apply {
            text = "Measurement Duration: ${measurementDuration.toInt()}s"
            textSize = 18f
            gravity = Gravity.CENTER_VERTICAL
            layoutParams = LinearLayout.LayoutParams(
                0,
                LinearLayout.LayoutParams.WRAP_CONTENT,
                1f // Ensures even spacing
            )
        }

        val decreaseDurationButton = MaterialButton(this, null, com.google.android.material.R.attr.materialIconButtonStyle).apply {
            text = "-"
            textSize = 24f // Make the button text bigger
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        }

        decreaseDurationButton.setOnClickListener {
            if (measurementDuration > 20.0) {
                measurementDuration -= 5.0
                onMeasurementDurationChanged(measurementDuration)
                measurementDurationTextView.text = "Measurement Duration: ${measurementDuration.toInt()}s"
            }
        }

        val increaseDurationButton = MaterialButton(this, null, com.google.android.material.R.attr.materialIconButtonStyle).apply {
            text = "+"
            textSize = 24f // Make the button text bigger
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            )
        }

        increaseDurationButton.setOnClickListener {
            if (measurementDuration < 120.0) {
                measurementDuration += 5.0
                onMeasurementDurationChanged(measurementDuration)
                measurementDurationTextView.text = "Measurement Duration: ${measurementDuration.toInt()}s"
            }
        }

        buttonRow.addView(measurementDurationTextView)
        buttonRow.addView(decreaseDurationButton)
        buttonRow.addView(increaseDurationButton)

        return buttonRow
    }

}
