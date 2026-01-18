//
//  File.swift
//  
//
//  Created by Benyamin Mokhtarpour on 6/29/23.
//

import Foundation
import UIKit

extension UIImage {
    /// Calculates the average brightness of the image.
    /// - Returns: A value between 0 and 1 where `0` is dark and `1` is bright.
    func averageBrightness() -> CGFloat? {
        guard let cgImage = self.cgImage else {
            return nil
        }

        let imageData = cgImage.dataProvider?.data
        let pointer = CFDataGetBytePtr(imageData)

        guard let dataPointer = pointer else {
            return nil
        }

        var totalBrightness: CGFloat = 0.0
        let bytesPerPixel = 4 // Assuming the image format is RGBA

        let width = cgImage.width
        let height = cgImage.height
        let pixelCount = width * height

        for pixelIndex in 0..<pixelCount {
            let pixelOffset = pixelIndex * bytesPerPixel
            let red = CGFloat(dataPointer[pixelOffset])
            let green = CGFloat(dataPointer[pixelOffset + 1])
            let blue = CGFloat(dataPointer[pixelOffset + 2])
            let alpha = CGFloat(dataPointer[pixelOffset + 3])

            let brightness = (red + green + blue) / 3.0 / 255.0
            totalBrightness += brightness * alpha
        }

        let averageBrightness = totalBrightness / CGFloat(pixelCount)
        return averageBrightness
    }
}
