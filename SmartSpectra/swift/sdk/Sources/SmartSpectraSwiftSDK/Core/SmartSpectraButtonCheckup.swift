//
//  SmartSpectraCheckupButton.swift
//
//
//  Created by Ashraful Islam on 8/14/24.
//

import Foundation
import SwiftUI

/// A custom button with a label on the left and a heart fill image on the right.
@available(iOS 15.0, *)
struct SmartSpectraCheckupButton: View {
    var action: () -> Void

    var body: some View {
        Button(action: action) {
            HStack {
                Image(systemName: "heart.fill")
                    .padding(.leading, 16)
                Text("Checkup")
                    .textCase(.uppercase)
                Spacer()
            }
        }
            .labelStyle(.titleAndIcon)
            .font(.system(size: 20, weight: .bold))
            .foregroundColor(.white)

    }
}
