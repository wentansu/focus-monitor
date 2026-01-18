//
//  SmartSpectraSwiftView.swift
//
//
//  Created by Ashraful Islam on 8/13/24.
//

import Foundation
import SwiftUI
import PresagePreprocessing
import AVFoundation

@available(iOS 15.0, *)
/// Turn‑key view that presents the SmartSpectra capture UI.
///
/// Include this view inside your SwiftUI hierarchy to allow users to record
/// vitals using the camera. Configuration is performed via
/// ``SmartSpectraSwiftSDK``.
@available(iOS 15.0, *)
public struct SmartSpectraView: View {

    /// Creates a new capture view using default styling.
    public init() {
    }

    /// Layout containing the recording button and results view.
    public var body: some View {
        VStack {
            SmartSpectraButtonView()
            SmartSpectraResultView()
        }
        .edgesIgnoringSafeArea(.all)
    }
}
