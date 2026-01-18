//
//  DemoAppView.swift
//  demo-app
//
//  Created by Ashraful Islam on 3/5/25.
//


//
//  ContentView.swift
//  breathing-app
//
//  Created by Ashraful Islam on 2/22/25.
//

import SwiftUI
struct DemoAppView: View {
    var body: some View {
        TabView {
            ContentView()
                .tabItem {
                    Label("Checkup", systemImage: "heart.fill")
                }
            if #available(iOS 16.0, *) {
                HeadlessSDKExample()
                    .tabItem {
                        Label("Headless Example", systemImage: "heart.text.square.fill")
                    }
            }
        }
    }
}

#Preview {
    DemoAppView()
}
