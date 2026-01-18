//
//  TutorialView.swift
//  SmartSpectraSwiftSDK
//
//  Created by Ashraful Islam on 10/10/24.
//
import SwiftUI

// Data model for each tutorial page
struct TutorialPage: Identifiable {
    let id = UUID()
    let imageName: String
    let description: String
}

struct TutorialView: View {
    var onTutorialCompleted: (() -> Void)?
    @Environment(\.presentationMode) var presentationMode
    @State private var currentPage = 0

    private let pages: [TutorialPage] = [
        TutorialPage(
            imageName: "tutorial_image1",
            description: "Place your device running SmartSpectra on a stable surface, like a table."
        ),
        TutorialPage(
            imageName: "tutorial_image2",
            description: "SmartSpectra works best when you're in a well-lit environment with natural sunlight for optimal performance."
        ),
        TutorialPage(
            imageName: "tutorial_image3",
            description: "SmartSpectra works best when your face is evenly lit and does not have shadows."
        ),
        TutorialPage(
            imageName: "tutorial_image4",
            description: "Avoid having bright light sources directly behind your face, such as overhead lighting."
        ),
        TutorialPage(
            imageName: "tutorial_image5",
            description: "Stay still and refrain from talking while using SmartSpectra."
        ),
        TutorialPage(
            imageName: "tutorial_image6",
            description: "You'll receive real-time feedback during the measurement process to assist you."
        ),
        TutorialPage(
            imageName: "tutorial_image7",
            description: "Start recording with SmartSpectra upon the 'Hold Still and Record' prompt. A 30-second recording follows, and should feedback appear, comply with the prompts for an auto-restart."
        )
    ]

    var body: some View {
        TabView(selection: $currentPage) {
            ForEach(Array(pages.enumerated()), id: \.offset) { index, page in
                TutorialPageView(
                    page: page,
                    isLastPage: index == pages.count - 1,
                    onTutorialCompleted: {
                        onTutorialCompleted?()
                        presentationMode.wrappedValue.dismiss()
                    }
                )
                .tag(index)
            }
        }
        .tabViewStyle(PageTabViewStyle())
        .indexViewStyle(.page(backgroundDisplayMode: .always))
        .accentColor(.black)
        .edgesIgnoringSafeArea(.all)
        .onChange(of: currentPage) { newPage in
            // Check if the user swiped past the last page
            if newPage >= pages.count {
                onTutorialCompleted?()
                presentationMode.wrappedValue.dismiss()
            }
        }
    }
}

// View for each individual tutorial page
struct TutorialPageView: View {
    let page: TutorialPage
    let isLastPage: Bool
    let onTutorialCompleted: (() -> Void)?

    var body: some View {
        VStack {
            Color.white
                .edgesIgnoringSafeArea(.all) // Extend background to fill screen
                .overlay(
                    Image(page.imageName, bundle: .module)
                        .resizable()
                        .scaledToFit()
                        .frame(maxHeight: .infinity)
                        .padding()
                )
            
            Spacer()
            
            Text(page.description)
                .font(.system(size: 20, weight: .medium, design: .rounded))
                .padding()
                .background(Color.white.opacity(0.9))
                .foregroundColor(.black)
                .multilineTextAlignment(.center)
                .frame(maxWidth: .infinity)
            Text(isLastPage ? "Swipe down to dismiss" : "Swipe left/right to navigate")
                .font(.footnote)
                .padding(10)
                .background(Color.black.opacity(0.7))
                .foregroundColor(.white)
                .cornerRadius(10)
                .padding(.bottom, 10)
        }
        .background(.white)
    }
}

#Preview {
    TutorialView() {
        print("Tutorial Completed")
    }
}
