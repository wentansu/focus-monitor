package com.presagetech.smartspectra.ui.screening

import android.content.Context
import android.graphics.Color
import android.util.AttributeSet
import android.view.LayoutInflater
import android.widget.LinearLayout
import android.widget.TextView
import androidx.lifecycle.LifecycleOwner
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.presage.physiology.proto.MetricsProto.MetricsBuffer
import com.presagetech.smartspectra.SmartSpectraSdk
import com.presagetech.smartspectra.R
import java.util.Locale
import kotlin.math.roundToInt

/**
 * View that renders pulse and breathing plots during continuous measurements.
 * Use [bindLifecycleOwner] to start observing metrics.
 */
class ScreeningPlotView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : LinearLayout(context, attrs, defStyleAttr) {

    private val pulseRateTitle: TextView
    private val pulseRateValue: TextView
    private val breathingRateTitle: TextView
    private val breathingRateValue: TextView
    private val pulsePlot: LineChart
    private val breathingPlot: LineChart

    private val pulseTraces = mutableListOf<Entry>()
    private val breathingTraces = mutableListOf<Entry>()

    private val viewModel: SmartSpectraSdk by lazy {
        SmartSpectraSdk.getInstance()
    }

    init {
        LayoutInflater.from(context).inflate(R.layout.screening_plot, this, true)
        orientation = VERTICAL

        pulseRateTitle = findViewById(R.id.pulseRateTitle)
        pulseRateValue = findViewById(R.id.pulseRateValue)
        breathingRateTitle = findViewById(R.id.breathingRateTitle)
        breathingRateValue = findViewById(R.id.breathingRateValue)
        pulsePlot = findViewById(R.id.pulsePlot)
        breathingPlot = findViewById(R.id.breathingPlot)

        setupChart(pulsePlot)
        setupChart(breathingPlot)
    }
    /**
     * Starts observing metrics from [SmartSpectraSdk] using the provided lifecycle.
     */
    fun bindLifecycleOwner(lifecycleOwner: LifecycleOwner) {
        // Observe metricsBuffer LiveData
        viewModel.metricsBuffer.observe(lifecycleOwner) { metricsBuffer ->
            if (metricsBuffer.pulse.rateCount > 0) {
                pulseRateValue.text = String.format(Locale.ROOT,"%d bpm", metricsBuffer.pulse.rateList.last().value.roundToInt())
            }
            if (metricsBuffer.breathing.rateCount > 0) {
                breathingRateValue.text = String.format(Locale.ROOT,"%d bpm", metricsBuffer.breathing.rateList.last().value.roundToInt())
            }
            updateTraces(metricsBuffer)
        }
    }

    /** Configures default styling for each chart. */
    private fun setupChart(chart: LineChart) {
        chart.description.text = ""
        chart.setNoDataText("")
        chart.setTouchEnabled(false)
        chart.isDragEnabled = false
        chart.setScaleEnabled(false)
        chart.setPinchZoom(false)
        // Remove grid lines and axis markers
        chart.xAxis.isEnabled = false
        chart.axisLeft.isEnabled = false
        chart.axisRight.isEnabled = false
        chart.legend.isEnabled = false
        chart.setDrawGridBackground(false)
    }

    /**
     * Merges new metric traces into the internal buffers and refreshes the
     * displayed charts.
     */
    private fun updateTraces(metricsBuffer: MetricsBuffer) {
        val newPulseEntries = metricsBuffer.pulse.traceList.map { Entry(it.time.toFloat(), it.value) }
        val newBreathingEntries = metricsBuffer.breathing.upperTraceList.map { Entry(it.time.toFloat(), it.value) }

        mergeData(pulseTraces, newPulseEntries, maxSize = 200)
        mergeData(breathingTraces, newBreathingEntries, maxSize = 400)

        updateChart(pulsePlot, pulseTraces, Color.RED)
        updateChart(breathingPlot, breathingTraces, Color.BLUE)
    }

    /**
     * Appends [newEntries] to [existingEntries] ensuring the list never exceeds
     * [maxSize].
     */
    private fun mergeData(existingEntries: MutableList<Entry>, newEntries: List<Entry>, maxSize: Int) {
        val firstNewTime = newEntries.firstOrNull()?.x ?: return
        val firstOverlapIndex = existingEntries.indexOfFirst { it.x >= firstNewTime }
        if (firstOverlapIndex != -1) {
            existingEntries.subList(firstOverlapIndex, existingEntries.size).clear()
        }
        existingEntries.addAll(newEntries)
        if (existingEntries.size > maxSize) {
            existingEntries.subList(0, existingEntries.size - maxSize).clear()
        }
    }

    /**
     * Applies the supplied [entries] to [chart] using the specified line color.
     */
    private fun updateChart(chart: LineChart, entries: List<Entry>, colorResId: Int) {
        val dataSet = LineDataSet(entries, chart.description.text.toString()).apply {
            lineWidth = 2f
            color = colorResId
            setDrawCircles(false)
            setDrawValues(false)
        }
        chart.data = LineData(dataSet)
        chart.notifyDataSetChanged()
        chart.invalidate()
        chart.animate()
    }
}
