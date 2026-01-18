//
//  ContinuousVitalsPlotView.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 3/5/25.
//

import SwiftUI

/// Live graph of pulse and breathing traces during continuous mode.
public struct ContinuousVitalsPlotView: View {
    @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
    @State private var pulseRate: Int = 0
    @State private var breathingRate: Int = 0
    @State private var pulseTrace: [Presage_Physiology_Measurement] = []
    @State private var breathingTrace: [Presage_Physiology_Measurement] = []

    /// Creates an empty plot view.
    public init() {

    }

    /// Plotting UI updated as metrics arrive.
    public var body: some View {
        VStack {
            GeometryReader { geometry in
                HStack(alignment: .bottom) {
                    Label("Pulse Rate\n\(pulseRate > 0 ? "\(pulseRate) bpm" : "--")", systemImage: "heart.fill")
                        .font(.headline)
                        .shadow(color: .white, radius: 8)
                    Spacer()

                    plotTrace(data: pulseTrace, width: geometry.size.width/2, height: geometry.size.height, color: Color.red, recentCount: 200)
                        .shadow(color: .white, radius: 4)
                        .padding(.horizontal, 10)
                        .frame(width: geometry.size.width / 2)
                }
            }
            .frame(height: 100)
            GeometryReader { geometry in
                HStack(alignment: .bottom) {
                    Label("Breathing Rate\n\(breathingRate > 0 ? "\(breathingRate) bpm" : "--")", systemImage: "lungs.fill")
                        .font(.headline)
                        .shadow(color: .white, radius: 8)
                    Spacer()

                    plotTrace(data: breathingTrace, width: geometry.size.width/2, height: geometry.size.height, color: Color.blue, recentCount: 400)
                        .shadow(color: .white, radius: 4)
                        .padding(.horizontal, 10)
                        .frame(width: geometry.size.width / 2)
                }
            }
            .frame(height: 100)
        }
        .onReceive(sdk.$metricsBuffer) { metricsBuffer in
            guard let metricsBuffer = metricsBuffer, metricsBuffer.isInitialized else { return }
            // update states
            pulseRate = Int(sdk.metricsBuffer?.pulse.rate.last?.value.rounded() ?? 0)
            breathingRate = Int(sdk.metricsBuffer?.breathing.rate.last?.value.rounded() ?? 0)
            withAnimation {
                pulseTrace.appendProtoArray(contentsOf: metricsBuffer.pulse.trace)
                breathingTrace.appendProtoArray(contentsOf: metricsBuffer.breathing.upperTrace)
            }

        }
        .onDisappear {
            print("ContinuousVitalsPlotView is disappearing")
            // reset the view states
            pulseTrace = []
            breathingTrace = []
            pulseRate = 0
            breathingRate = 0
        }
    }

    /// Draws a simple line plot for the given metric data.
    /// - Parameters:
    ///   - data: The measurements to display.
    ///   - width: Width of the plot area.
    ///   - height: Height of the plot area.
    ///   - color: Stroke color for the line.
    ///   - recentCount: Number of points from the end of the array to display.
    private func plotTrace(data: [Presage_Physiology_Measurement], width: CGFloat, height: CGFloat, color: Color, recentCount: Int) -> some View {
        let displayedData = data.suffix(recentCount)
        return Path { path in
            guard displayedData.count > 1 else { return }

            let minTime = displayedData.first!.time
            let maxTime = displayedData.last!.time
            let timeRange = maxTime - minTime

            let minValue = displayedData.map { $0.value }.min()
            let maxValue = displayedData.map { $0.value }.max()
            let valueRange = maxValue! - minValue!

            let points = displayedData.compactMap { measurement -> CGPoint? in
                let x = CGFloat((measurement.time - minTime) / timeRange) * width
                let y = height - CGFloat((measurement.value - minValue!) / valueRange) * height
                guard !x.isNaN, !y.isNaN else { return nil }
                return CGPoint(x: x, y: y)
            }

            // Ensure points array has at least one valid point
            guard let firstPoint = points.first else { return }

            path.move(to: firstPoint)
            points.dropFirst().forEach { point in
                path.addLine(to: point)
            }
        }
        .stroke(color, lineWidth: 2)
    }
}
