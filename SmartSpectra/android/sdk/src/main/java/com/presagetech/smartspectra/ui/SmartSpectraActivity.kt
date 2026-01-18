package com.presagetech.smartspectra.ui

import android.os.Bundle
import androidx.annotation.OptIn
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.camera2.interop.ExperimentalCamera2Interop
import androidx.fragment.app.Fragment
import com.google.mediapipe.components.PermissionHelper
import com.presagetech.smartspectra.R
import com.presagetech.smartspectra.ui.screening.CameraProcessFragment
import com.presagetech.smartspectra.ui.screening.PermissionsRequestFragment
import com.presagetech.smartspectra.ui.summary.UploadingFragment
import timber.log.Timber

/**
 * This is the MainActivity of SmartSpectra module the project structure is base on SingleActivity
 * structure so we used navigation component to handle navigation between module Fragments.
 *
 * */
internal class SmartSpectraActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main_layout_nav)
        openPermissionsFragment()
    }

    override fun onResume() {
        super.onResume()
        Timber.w("Resumed Smart Spectra Activity")
        val cameraPermissionGranted = PermissionHelper.cameraPermissionsGranted(this)
        if (cameraPermissionGranted) {
            openCameraFragment()
        } else {
            openPermissionsFragment()
        }
    }

    /**
     * Navigates to the permissions screen prompting the user for camera access.
     */
    private fun openPermissionsFragment() {
        openFragment(PermissionsRequestFragment())
    }

    /**
     * Opens the main camera processing fragment once permissions are granted.
     */
    @OptIn(ExperimentalCamera2Interop::class)
    private fun openCameraFragment() {
        openFragment(CameraProcessFragment())
    }

    /**
     * Displays the upload progress screen after a successful spot measurement.
     */
    internal fun openUploadFragment() {
        // TODO: 9/30/23: See if there is a better way to achieve this
        supportFragmentManager.beginTransaction()
            .add(R.id.host_fragment, UploadingFragment())
            .addToBackStack(null)
            .commit()
    }

    /**
     * Replaces the current fragment with [fragment] without adding to the back
     * stack.
     */
    private fun openFragment(fragment: Fragment) {
        Timber.i("Opening fragment: ${fragment::class.java.simpleName}")
        supportFragmentManager.beginTransaction()
            .replace(R.id.host_fragment, fragment)
            .commit()
    }
}
