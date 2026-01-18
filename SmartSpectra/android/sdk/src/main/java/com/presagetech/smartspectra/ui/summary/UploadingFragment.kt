package com.presagetech.smartspectra.ui.summary

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ProgressBar
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.presagetech.smartspectra.R

internal class UploadingFragment : Fragment() {

    private lateinit var statusText: TextView
    private lateinit var progressBar: ProgressBar

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragment_uploading_layout, container, false).also {
            statusText = it.findViewById(R.id.text_status)
            progressBar = it.findViewById(R.id.progress_bar)
        }

        return view
    }
}
