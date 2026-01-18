//
//  DemoApp.swift
//  Demo App
//
//  Created by Bill Vivino on 4/5/24.
//

import SwiftUI

@main
struct DemoApp: App {
    var body: some Scene {
        WindowGroup {
            if #available(iOS 16.0, *) {
                DemoAppView()
            } else {
                ContentView()
            }
        }
    }
}
