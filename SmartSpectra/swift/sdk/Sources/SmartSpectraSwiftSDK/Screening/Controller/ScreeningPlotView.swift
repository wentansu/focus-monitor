//
//  ScreeningPlotView.swift
//  SmartSpectraIosSDK
//
//  Created by Ashraful Islam on 10/24/24.
//
import SwiftUI
import AVFoundation
import PresagePreprocessing

struct ScreeningPlotView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared
    @State private var cameraPosition: AVCaptureDevice.Position
    @State private var smartSpectraMode: SmartSpectraMode

    init() {
        cameraPosition = SmartSpectraSwiftSDK.shared.config.cameraPosition
        smartSpectraMode = SmartSpectraSwiftSDK.shared.config.smartSpectraMode
    }

    var body: some View {
        VStack(alignment: .leading) {
            
            if(sdk.config.showControlsInScreeningView) {
                HStack {
                    Button(smartSpectraMode == .spot ? "Spot Mode": "Continuous Mode" , systemImage: smartSpectraMode == .spot ? "chart.dots.scatter" : "waveform.path") {
                        if smartSpectraMode == .spot {
                            smartSpectraMode = .continuous
                        } else {
                            smartSpectraMode = .spot
                        }
                        
                        vitalsProcessor.changeProcessingMode(smartSpectraMode)
                    }
                    .labelStyle(.iconOnly)
                    .font(.system(size: 24))
                    .disabled(vitalsProcessor.isRecording)
                    
                    Spacer()
                    
                    Button("Switch Camera", systemImage: "camera.rotate") {
                        if cameraPosition == .front {
                            cameraPosition = .back
                        } else {
                            cameraPosition = .front
                        }
                        
                        vitalsProcessor.setProcessingCameraPosition(cameraPosition)
                    }
                    .labelStyle(.iconOnly)
                    .font(.system(size: 24))
                    .disabled(vitalsProcessor.isRecording)
                }
            }
            

            if smartSpectraMode == .continuous {
                ContinuousVitalsPlotView()
            }
            Spacer()
        }
        .padding()
    }
}
