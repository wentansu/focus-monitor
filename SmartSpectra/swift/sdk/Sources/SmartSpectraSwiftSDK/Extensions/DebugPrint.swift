//
//  File.swift
//  
//
//  Created by Benyamin Mokhtarpour on 5/23/23.
//

import Foundation

/// Helper for printing debug messages only in DEBUG builds.
struct Logger {
    static func log(_ items: Any..., separator: String = " ", terminator: String = "\n", file: String = #file, line: Int = #line, function: String = #function) {
        #if DEBUG
        let prefix = "\(file):\(line) - \(function):"
        Swift.debugPrint(prefix, terminator: " ")
        Swift.debugPrint(items, separator: separator, terminator: terminator)
        #endif
    }
}
