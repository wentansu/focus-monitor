//
//  MetricsProtobufExtensions.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 3/8/25.
//

import Foundation

/// A protocol for types that have a time-based property.
/// Conforming to this protocol allows elements to be efficiently sorted and merged in an array.
public protocol TimeStamped {
    /// The timestamp associated with the element.
    var time: Float { get }
}

/// Conforming types from SmartSpectra SDK.
extension Presage_Physiology_Measurement: TimeStamped {}
extension Presage_Physiology_DetectionStatus: TimeStamped {}
extension Presage_Physiology_MeasurementWithConfidence: TimeStamped {}

/// An extension that provides efficient methods for appending and merging time-based elements in a sorted manner.
public extension Array where Element: TimeStamped {

    /// Merges a new array of time-stamped elements into the existing array while maintaining sorted order.
    ///
    /// The method optimizes insertion by:
    /// - Directly appending if all new elements have timestamps later than the existing ones.
    /// - Replacing the overlapping section of the existing array with the new elements.
    /// - Appending any remaining new elements beyond the last existing timestamp.
    ///
    /// - Parameter newElements: The new elements to be merged into the array.
    ///
    /// ### Complexity:
    /// - **Best Case:** O(m) when all new elements are later than the existing ones.
    /// - **Average Case:** O(m log n) due to binary search for insertion points.
    /// - **Worst Case:** O(n + m) if multiple elements require insertion or replacement.
    mutating func appendProtoArray(contentsOf newElements: [Element]) {
        guard !newElements.isEmpty else { return }

        // Fast path: If the current array is empty, simply set it to newElements.
        if self.isEmpty {
            self = newElements
            return
        }

        // Fast path: If the first new element is later than the last existing element, append directly.
        if let lastExistingTime = self.last?.time,
           let newFirstTime = newElements.first?.time,
           newFirstTime > lastExistingTime {
            self.append(contentsOf: newElements)
            return
        }

        let firstNewTime = newElements.first!.time
        // Locate the first index in the current array where overlap begins.
        let firstExistingIndex = insertionIndex(forTime: firstNewTime)
        let overlapRange = firstExistingIndex..<self.count
        let lastExistingTime = self.last?.time ?? -Float.greatestFiniteMagnitude
        let lastNewTime = newElements.last!.time

        // If new elements extend past the current array, replace the overlapping section and then append the remainder.
        if lastNewTime > lastExistingTime {
            let remainderIndex = newElements.insertionIndexStrict(forTime: lastExistingTime)
            let overlapElements = newElements[0..<remainderIndex]
            let remainder = newElements[remainderIndex..<newElements.count]
            self.replaceSubrange(overlapRange, with: overlapElements)
            self.append(contentsOf: remainder)
        } else {
            // Fully replace the overlapping portion.
            self.replaceSubrange(overlapRange, with: newElements)
        }
    }

    /// Performs a binary search to find the index of an element with the specified timestamp.
    ///
    /// - Parameter time: The timestamp to search for.
    /// - Returns: The index of the element if an element with an equal timestamp is found, otherwise `nil`.
    ///
    /// ### Complexity: O(log n)
    func indexOfElement(withTime time: Float) -> Int? {
        var lower = 0, upper = count - 1
        while lower <= upper {
            let mid = (lower + upper) / 2
            let midTime = self[mid].time
            if midTime == time { return mid }
            else if midTime < time { lower = mid + 1 }
            else { upper = mid - 1 }
        }
        return nil
    }

    /// Finds the index at which a new element with the given timestamp should be inserted to maintain sorted order.
    ///
    /// This method returns the first index where the element's timestamp is greater than or equal to the provided time.
    ///
    /// - Parameter time: The timestamp of the new element.
    /// - Returns: The index at which the new element should be inserted.
    ///
    /// ### Complexity: O(log n)
    func insertionIndex(forTime time: Float) -> Int {
        var lower = 0, upper = count
        while lower < upper {
            let mid = (lower + upper) / 2
            if self[mid].time < time { lower = mid + 1 }
            else { upper = mid }
        }
        return lower
    }

    /// Finds the index at which a new element with the given timestamp should be inserted,
    /// using a strictly greater-than comparison.
    ///
    /// This method returns the first index where the element's timestamp is strictly greater than the provided time.
    ///
    /// - Parameter time: The timestamp to compare against.
    /// - Returns: The first index at which the new element's timestamp is strictly greater than the provided time.
    ///
    /// ### Complexity: O(log n)
    func insertionIndexStrict(forTime time: Float) -> Int {
        var lower = 0, upper = count
        while lower < upper {
            let mid = (lower + upper) / 2
            if self[mid].time <= time { lower = mid + 1 }
            else { upper = mid }
        }
        return lower
    }
}
