//
//  Benchmarking.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 3/8/25.
//

import Dispatch

/**
 Measures the execution time of a function and prints the result.

 - Parameters:
    - label: A descriptive label for the benchmark test.
    - operation: The function to be benchmarked.

 Use this function to benchmark the performance of different implementations.

 ## Example Usage

 ```swift
 benchmark("Improved Version") {
    updateTracesImproved(metricsBuffer)
 }
 ```
 - Note: The execution time is printed in milliseconds.
 */
internal func benchmark(_ label: String, operation: () -> Void) {
    let startTime = DispatchTime.now()
    operation()
    let endTime = DispatchTime.now()

    let nanoTime = endTime.uptimeNanoseconds - startTime.uptimeNanoseconds
    let timeInterval = Double(nanoTime) / 1_000_000 // Convert to milliseconds

    print("\(label): \(timeInterval) ms")
}
