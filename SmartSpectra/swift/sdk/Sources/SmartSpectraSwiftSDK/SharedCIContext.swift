//
//  SharedCIContext.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 3/3/25.
//

import Foundation
import CoreImage
import Metal

/// Shared CoreImage context for efficient image processing across the SDK.
class SharedCIContext: @unchecked Sendable {
    static let shared = SharedCIContext()
    
    private let context: CIContext
    private let contextQueue = DispatchQueue(label: "com.smartspectra.cicontext.queue", qos: .userInitiated)
    private var isValid = true
    private let validityLock = NSLock()
    
    private init() {
        let options: [CIContextOption: Any] = [
            .workingColorSpace: CGColorSpace(name: CGColorSpace.sRGB)!,
            .outputColorSpace: CGColorSpace(name: CGColorSpace.sRGB)!,
            .useSoftwareRenderer: false
        ]
        
        if let device = MTLCreateSystemDefaultDevice() {
            context = CIContext(mtlDevice: device, options: options)
            Logger.log("SharedCIContext: Initialized with Metal device")
        } else {
            context = CIContext(options: options)
            Logger.log("SharedCIContext: Initialized with CPU fallback")
        }
    }
    
    func createCGImage(_ image: CIImage, from rect: CGRect, completion: @escaping @Sendable (CGImage?) -> Void) {
        validityLock.lock()
        guard isValid else {
            validityLock.unlock()
            Logger.log("SharedCIContext: Context is invalid, skipping operation")
            completion(nil)
            return
        }
        validityLock.unlock()
        
        contextQueue.async { [weak self] in
            guard let self = self else {
                completion(nil)
                return
            }
            
            autoreleasepool {
                let cgImage = self.context.createCGImage(image, from: rect)
                completion(cgImage)
            }
        }
    }
    
    func invalidate() {
        validityLock.lock()
        isValid = false
        validityLock.unlock()
    }
}