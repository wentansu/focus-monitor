//
//  File.swift
//
//
//  Created by Benyamin Mokhtarpour on 5/23/23.
//

import Foundation
import UIKit
import AVFoundation
import SwiftUI

@available(iOS 15.0, *)
extension ViewController.Screening.Root {

    // UI CComponents methods
    internal func setupUIComponents() {
        addImagePreview()
        addBorderView()
        addRecordButton()
        addTimerView()
        addFPSLabel()
        addInfoButton()
        setupCharts()

    }

    internal func setupCharts() {
        screeningPlotView = UIHostingController(rootView: ScreeningPlotView())

        guard let screeningPlotView = screeningPlotView else { return }
        screeningPlotView.view.backgroundColor = UIColor.clear

        self.addChild(screeningPlotView)
        self.view.addSubview(screeningPlotView.view)
        screeningPlotView.didMove(toParent: self)

        setupChartConstraints(chartView: screeningPlotView.view)
    }
    internal func setupChartConstraints(chartView: UIView) {
        chartView.translatesAutoresizingMaskIntoConstraints = false

        NSLayoutConstraint.activate([
            chartView.leadingAnchor.constraint(equalTo: self.view.leadingAnchor, constant: 24),
            chartView.trailingAnchor.constraint(equalTo: self.view.trailingAnchor, constant: -24),
            chartView.topAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.topAnchor, constant: 24),
            chartView.heightAnchor.constraint(equalToConstant: 400)
        ])
    }

    internal func addRecordButton() {
        self.view.addSubview(recordButton)

        NSLayoutConstraint.activate([
            recordButton.heightAnchor.constraint(equalToConstant: 80),
            recordButton.widthAnchor.constraint(equalToConstant: 80),
            recordButton.centerXAnchor.constraint(equalTo: self.view.centerXAnchor),
            recordButton.bottomAnchor.constraint(equalTo: self.view.bottomAnchor, constant: -50)

        ])
        recordButton.addTarget(self, action: #selector(recordButtonTapped), for: .touchUpInside)
    }

    internal func addBorderView() {
        view.addSubview(startOverlayView)
        view.addSubview(endOverlayView)
        view.addSubview(bottomOverlayView)

        let screenHeight = UIScreen.main.bounds.height
        let targetHeightPercentage: CGFloat = 0.3
        let targetHeight = screenHeight * targetHeightPercentage

        NSLayoutConstraint.activate([
            startOverlayView.leadingAnchor.constraint(equalTo: imageHolder.leadingAnchor),
            startOverlayView.topAnchor.constraint(equalTo: imageHolder.topAnchor),
            startOverlayView.bottomAnchor.constraint(equalTo: imageHolder.bottomAnchor),
            startOverlayView.widthAnchor.constraint(equalToConstant: 40),

            endOverlayView.trailingAnchor.constraint(equalTo: imageHolder.trailingAnchor),
            endOverlayView.topAnchor.constraint(equalTo: imageHolder.topAnchor),
            endOverlayView.bottomAnchor.constraint(equalTo: imageHolder.bottomAnchor),
            endOverlayView.widthAnchor.constraint(equalToConstant: 40),

            bottomOverlayView.leadingAnchor.constraint(equalTo: imageHolder.leadingAnchor, constant: 40),
            bottomOverlayView.trailingAnchor.constraint(equalTo: imageHolder.trailingAnchor, constant: -40),
            bottomOverlayView.bottomAnchor.constraint(equalTo: imageHolder.bottomAnchor),
            bottomOverlayView.widthAnchor.constraint(equalToConstant: UIScreen.main.bounds.width - 80),
            bottomOverlayView.heightAnchor.constraint(equalToConstant: targetHeight)
        ])
    }

    internal func addImagePreview() {
        self.view.addSubview(imageHolder)

        NSLayoutConstraint.activate([
            imageHolder.leadingAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.leadingAnchor),
            imageHolder.trailingAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.trailingAnchor),
            imageHolder.bottomAnchor.constraint(equalTo: self.view.bottomAnchor),
            imageHolder.topAnchor.constraint(equalTo: self.view.topAnchor, constant: topBarHeight)

        ])
    }

    internal func addTimerView() {
        self.view.addSubview(counterView)
        NSLayoutConstraint.activate([
            counterView.heightAnchor.constraint(equalToConstant: 60),
            counterView.widthAnchor.constraint(equalToConstant: 60),
            counterView.leftAnchor.constraint(equalTo: self.view.leftAnchor, constant: 40),
            counterView.centerYAnchor.constraint(equalTo: recordButton.centerYAnchor)
        ])
        // Only show for spot runningMode
        counterView.isHidden = self.sdk.config.smartSpectraMode != .spot
    }

    internal func addFPSLabel() {
        self.view.addSubview(fpsLabel)

        NSLayoutConstraint.activate([
            fpsLabel.heightAnchor.constraint(equalToConstant: 30),
            fpsLabel.widthAnchor.constraint(equalToConstant: 80),
            fpsLabel.rightAnchor.constraint(equalTo: self.view.rightAnchor, constant: -40),
            fpsLabel.centerYAnchor.constraint(equalTo: recordButton.centerYAnchor)
        ])
    }

    internal func addInfoButton() {
        let infoButton = UIButton(type: .infoLight)
        infoButton.translatesAutoresizingMaskIntoConstraints = false
        self.view.addSubview(infoButton)

        // Positioning the button in the top-right corner with some padding
        NSLayoutConstraint.activate([
            infoButton.topAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.topAnchor, constant: 0),
            infoButton.trailingAnchor.constraint(equalTo: self.view.safeAreaLayoutGuide.trailingAnchor, constant: -10),
            infoButton.widthAnchor.constraint(equalToConstant: 40),
            infoButton.heightAnchor.constraint(equalToConstant: 40)
        ])

        // Add target to show the tip message when tapped
        infoButton.addTarget(self, action: #selector(infoButtonTapped), for: .touchUpInside)
    }

    @objc private func infoButtonTapped() {
        DispatchQueue.main.async {
            let alert = UIAlertController(title: "Tip", message: "Please ensure the subject’s face, shoulders, and upper chest are in view and remove any clothing that may impede visibility. Please refer to Instructions For Use for more information.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
            self.present(alert, animated: true, completion: nil)
        }
    }

    @objc private func recordButtonTapped() {
        if buttonState == .ready {
            // start auth workflow (this re-authenticates if expired)
            // TODO: might no longer be necessary or convert to use closure before starting countdown
            AuthHandler.shared.startAuthWorkflow()

            startCountdownForRecording { [weak self] in
                guard let self = self else { return }
                self.vitalsProcessor.startRecording()
                self.buttonState = .running
                self.isRecording = true
            }

        } else if buttonState == .countdown {
            buttonState = .ready
        }
        else if buttonState == .running {
            vitalsProcessor.stopRecording()
            buttonState = .ready
            isRecording = false
        } else {
            Logger.log("Recording button is disabled.")
        }
    }

    internal func moveToProcessing() {
        DispatchQueue.main.async { [weak self] in
            guard let self = self else { return }
            self.vitalsProcessor.stopRecording()
            let vc = ViewController.Processing.Root()
            self.navigationController?.pushViewController(vc, animated: true)
        }
    }

    private func startCountdownForRecording(completion: @escaping () -> Void) {
        var countdown = sdkConfig.recordingDelay

        guard countdown > 0 else { completion() ; return}

        buttonState = .countdown

        // Invalidate any existing timer before creating a new one
        countdownTimer?.invalidate()

        countdownTimer = Timer.scheduledTimer(withTimeInterval: 1, repeats: true) { [weak self] timer in
                // only move forward if button state is still countdown
                guard let self = self, self.buttonState == .countdown else {
                    timer.invalidate()
                    return
                }
                countdown -= 1
                if countdown > 0 {
                    DispatchQueue.main.async {
                        self.recordButton.setTitle("\(countdown)", for: .normal)
                    }
                } else {
                    timer.invalidate()
                    self.countdownTimer = nil // Clear the reference when done
                    completion()
                }
            }
    }
}

extension UIViewController {
    var topBarHeight: CGFloat {
        let navigationBarHeight = self.navigationController?.navigationBar.frame.height ?? 0.0
        let statusBarHeight = view.safeAreaInsets.top

        return navigationBarHeight + statusBarHeight
    }
}
