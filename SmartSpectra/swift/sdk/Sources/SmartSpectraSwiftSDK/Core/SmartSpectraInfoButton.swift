//
//  SmartSpectraCheckupButton.swift
//
//
//  Created by Ashraful Islam on 8/14/24.
//

import SwiftUI

@available(iOS 15.0, *)
struct SmartSpectraInfoButton: View {
    var action: () -> Void

    var body: some View {
        Button(action: action) {
            ZStack {
                // Custom background with sharp left corners and rounded right corners
                CustomRoundedRectangle(cornerRadius: 20, corners: [.topRight, .bottomRight])
                    .fill(Color.white)
                    .frame(maxHeight: .infinity)
                    .overlay(
                        CustomRoundedRectangle(cornerRadius: 20, corners: [.topRight, .bottomRight])
                            .stroke(Color(red: 0.94, green: 0.34, blue: 0.36), lineWidth: 4)
                    )
                
                Label("Info Button", systemImage: "info.circle.fill")
                    .labelStyle(.iconOnly)
                    .foregroundColor(Color(red: 0.94, green: 0.34, blue: 0.36))
                    .font(.system(size: 20.0))
            }
            .frame(maxHeight: .infinity)
        }
    }
}

fileprivate struct CustomRoundedRectangle: Shape {
    var cornerRadius: CGFloat
    var corners: UIRectCorner

    func path(in rect: CGRect) -> Path {
        let path = UIBezierPath(roundedRect: rect, byRoundingCorners: corners, cornerRadii: CGSize(width: cornerRadius, height: cornerRadius))
        return Path(path.cgPath)
    }
}
