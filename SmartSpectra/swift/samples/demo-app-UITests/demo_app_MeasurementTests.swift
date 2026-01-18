//
//  demo_app_MeasurementTests.swift
//  demo-app-UITests
//
//  Full measurement flow tests for Continuous and Spot modes
//

import XCTest

final class demo_app_MeasurementTests: UITestBase {

    override func setUpWithError() throws {
        try super.setUpWithError()

        // Handle camera permissions on real device
        if isRealDevice {
            app.handleCameraPermissionIfNeeded()
        }
    }

    private func handleTutorialFlow() {
        // Handle tutorial/onboarding flow if it appears (only shows on first run)

        // Check if any tutorial exists by checking the first image
        let firstTutorialImage = app.images["tutorial_image1"]
        if !firstTutorialImage.waitForExistence(timeout: 1) {
            print("ℹ️ No tutorial flow detected, skipping...")
            return
        }

        // Tutorial exists, swipe through all images
        print("📖 Tutorial flow detected, processing...")
        let tutorialImages = ["tutorial_image1", "tutorial_image2", "tutorial_image3",
                             "tutorial_image4", "tutorial_image5", "tutorial_image6", "tutorial_image7"]

        for (index, imageName) in tutorialImages.enumerated() {
            let image = app.images[imageName]
            if image.waitForExistence(timeout: 1) {
                print("📖 Swiping tutorial image \(index + 1)...")
                if index == tutorialImages.count - 1 {
                    image.swipeRight() // Last image swipes right
                } else {
                    image.swipeLeft() // Other images swipe left
                }
                Thread.sleep(forTimeInterval: 0.5)
            }
        }

        // Handle agreement/terms if they appear
        let agreeButton = app.staticTexts["Agree"]
        if agreeButton.waitForExistence(timeout: 2) {
            print("📜 Accepting terms...")
            agreeButton.tap()
            Thread.sleep(forTimeInterval: 1)
        }

        // Handle system permissions dialog if it appears in springboard
        let springboardApp = XCUIApplication(bundleIdentifier: "com.apple.springboard")
        let allowButton = springboardApp.buttons["Allow"]
        if allowButton.waitForExistence(timeout: 2) {
            print("🔐 Allowing camera permission...")
            allowButton.tap()
            Thread.sleep(forTimeInterval: 1)
        }

        print("✅ Tutorial flow completed")
    }

    @MainActor
    func testContinuousMeasurementFlow() throws {
        // Test full continuous measurement workflow
        XCTAssertTrue(waitForAppToLoad(), "App should launch")

        // Ensure we're in continuous mode
        let switchToSpotButton = app.buttons["Switch to Spot"]
        if switchToSpotButton.waitForExistence(timeout: 2) {
            // Already in continuous mode, good
            print("✅ Already in Continuous mode")
        } else {
            // Switch from spot to continuous
            let switchToContinuousButton = app.buttons["Switch to Continuous"]
            if switchToContinuousButton.waitForExistence(timeout: 2) {
                switchToContinuousButton.tap()
                XCTAssertTrue(switchToSpotButton.waitForExistence(timeout: 2), "Should switch to continuous mode")
                print("✅ Switched to Continuous mode")
            }
        }

        // Handle tutorial/onboarding flow if it appears
        handleTutorialFlow()

        // Find and tap the SmartSpectra Checkup button
        let checkupButton = app.buttons.containing(.image, identifier: "Love").firstMatch
        if checkupButton.waitForExistence(timeout: 3) {
            print("✅ Found SmartSpectra Checkup button")
            checkupButton.tap()

            // wait for the UI to be visible
            Thread.sleep(forTimeInterval: 1)
            // Look for Record button in the measurement UI and wait for it to be tappable
            let recordButton = app.staticTexts["Record"]
            if recordButton.waitForExistence(timeout: 3) {
                print("✅ Found Record button, waiting for it to be tappable...")

                // Wait for button to be hittable/tappable (up to 5 seconds)
                let timeout = Date().addingTimeInterval(5)
                while !recordButton.isHittable && Date() < timeout {
                    Thread.sleep(forTimeInterval: 0.5)
                }

                if recordButton.isHittable {
                    print("✅ Record button is now tappable, starting measurement")
                    recordButton.tap()
                } else {
                    print("⚠️ Record button exists but is not tappable after 5 seconds")
                    takeScreenshot(name: "Record Button Not Tappable")
                    recordButton.tap() // Try anyway
                }

                // In continuous mode, wait 10 seconds then stop recording
                print("⏱️ Recording for 10 seconds...")
                Thread.sleep(forTimeInterval: 10)

                // Look for and click the Stop button
                let stopButton = app.staticTexts["Stop"]
                if stopButton.waitForExistence(timeout: 2) {
                    print("✅ Found Stop button, stopping recording")
                    stopButton.tap()

                    // Wait a moment for processing
                    Thread.sleep(forTimeInterval: 3)
                    
                    // Press back button to return to main screen
                    let backButton = app.navigationBars.buttons.element(boundBy: 0)
                    if backButton.waitForExistence(timeout: 2) {
                        print("✅ Found back button, returning to main screen")
                        backButton.tap()
                        Thread.sleep(forTimeInterval: 1)
                    } else {
                        // Try alternative back button selectors
                        let backButton2 = app.buttons["Back"]
                        if backButton2.waitForExistence(timeout: 1) {
                            print("✅ Found 'Back' button, returning to main screen")
                            backButton2.tap()
                            Thread.sleep(forTimeInterval: 1)
                        } else {
                            print("⚠️ Back button not found, may still be in measurement view")
                            takeScreenshot(name: "Continuous Mode - No Back Button")
                        }
                    }
                } else {
                    print("⚠️ Stop button not found, measurement may still be running")
                    takeScreenshot(name: "Continuous Mode - No Stop Button")
                }
            } else {
                print("❌ Record button not found")
                takeScreenshot(name: "Continuous Mode - No Record Button")
                XCTFail("Could not find Record button in measurement UI")
                return
            }
        } else {
            print("❌ SmartSpectra Checkup button not found")
            takeScreenshot(name: "Continuous Mode - No Checkup Button")
            XCTFail("Could not find SmartSpectra Checkup button")
            return
        }

        // Check for results without scrolling (they might be visible)
        let sectionHeaders = ["Pulse", "Breathing", "Blood Pressure", "Face"]
        var foundResults = false

        for header in sectionHeaders {
            let section = app.staticTexts[header]
            if section.exists {
                print("✅ Found \(header) section")
                foundResults = true
            }
        }

        // Also check for any chart titles that might appear
        let chartTitles = ["Pulse Pleth", "Breathing Pleth", "Pulse Rates", "Breathing Rates"]
        for title in chartTitles {
            let chart = app.staticTexts[title]
            if chart.exists {
                print("✅ Found \(title) chart")
                foundResults = true
            }
        }

        if foundResults {
            takeScreenshot(name: "Continuous Mode - Data Found")
        } else {
            print("ℹ️ No immediate data visible - continuous mode may need more time")
            takeScreenshot(name: "Continuous Mode - Waiting for Data")
        }
    }

    @MainActor
    func testSpotMeasurementFlow() throws {
        // Test full spot measurement workflow
        XCTAssertTrue(waitForAppToLoad(), "App should launch")

        // Switch to spot mode
        let switchToSpotButton = app.buttons["Switch to Spot"]
        if switchToSpotButton.waitForExistence(timeout: 2) {
            switchToSpotButton.tap()
            XCTAssertTrue(app.buttons["Switch to Continuous"].waitForExistence(timeout: 2), "Should switch to spot mode")
            print("✅ Switched to Spot mode")
        } else {
            print("✅ Already in Spot mode")
        }

        // Set measurement duration to 30 seconds (default should be 30)
        let stepper = app.steppers.firstMatch
        if stepper.waitForExistence(timeout: 2) {
            let durationText = app.staticTexts.containing(NSPredicate(format: "label CONTAINS 'Measurement Duration'")).firstMatch
            if durationText.exists {
                print("📏 Current duration: \(durationText.label)")

                // Ensure we're at 30 seconds
                let currentLabel = durationText.label
                if !currentLabel.contains("30") {
                    // Adjust to 30 if needed
                    let incrementButton = stepper.buttons["Increment"]
                    let decrementButton = stepper.buttons["Decrement"]

                    // Simple approach: reset to reasonable value
                    if incrementButton.exists && currentLabel.contains("25") {
                        incrementButton.tap()
                        print("✅ Adjusted duration to 30 seconds")
                    } else if decrementButton.exists && currentLabel.contains("35") {
                        decrementButton.tap()
                        print("✅ Adjusted duration to 30 seconds")
                    }
                }
            }
        }

        // Handle tutorial/onboarding flow if it appears
        handleTutorialFlow()

        // Find and tap the SmartSpectra Checkup button
        let checkupButton = app.buttons.containing(.image, identifier: "Love").firstMatch
        if checkupButton.waitForExistence(timeout: 3) {
            print("✅ Found SmartSpectra Checkup button")
            checkupButton.tap()

            // wait for the UI to be visible
            Thread.sleep(forTimeInterval: 1)

            // Look for Record button in the measurement UI and wait for it to be tappable
            let recordButton = app.staticTexts["Record"]
            if recordButton.waitForExistence(timeout: 3) {
                print("✅ Found Record button, waiting for it to be tappable...")

                // Wait for button to be hittable/tappable (up to 5 seconds)
                let timeout = Date().addingTimeInterval(5)
                while !recordButton.isHittable && Date() < timeout {
                    Thread.sleep(forTimeInterval: 0.5)
                }

                if recordButton.isHittable {
                    print("✅ Record button is now tappable, starting spot measurement")
                    takeScreenshot(name: "Spot Measurement UI - Ready to Record")
                    recordButton.tap()
                } else {
                    print("⚠️ Record button exists but is not tappable after 5 seconds")
                    takeScreenshot(name: "Record Button Not Tappable")
                    recordButton.tap() // Try anyway
                }

                // Wait for countdown + measurement + processing (30s + buffers)
                print("⏱️ Starting 30-second spot measurement...")
                Thread.sleep(forTimeInterval: 60) // allowing extra time for data to be uploaded and returning from the api

                // Look for results directly without scrolling first
                let sectionHeaders = ["Pulse", "Breathing", "Blood Pressure", "Face"]
                var foundResults = false

                for header in sectionHeaders {
                    let section = app.staticTexts[header]
                    if section.exists {
                        print("✅ Found \(header) section")
                        foundResults = true
                    }
                }

                // Check for chart titles
                let chartTitles = ["Pulse Pleth", "Breathing Pleth", "Pulse Rates", "Breathing Rates"]
                for title in chartTitles {
                    let chart = app.staticTexts[title]
                    if chart.exists {
                        print("✅ Found \(title) chart")
                        foundResults = true
                    }
                }

                // Look for BPM values
                let bpmResults = app.staticTexts.containing(NSPredicate(format: "label CONTAINS 'bpm' OR label CONTAINS 'BPM'")).firstMatch
                if bpmResults.exists {
                    print("✅ Found BPM measurement: \(bpmResults.label)")
                    foundResults = true
                }

                // Take screenshot regardless of results found
                if foundResults {
                    takeScreenshot(name: "Spot Measurement - Results Found")
                } else {
                    takeScreenshot(name: "Spot Measurement - Completed")
                }

                // Try gentle scroll to see more results if scroll view exists
                let scrollView = app.scrollViews.firstMatch
                if scrollView.exists && scrollView.isHittable {
                    print("🔄 Attempting to view more results...")
                    scrollView.swipeUp()
                    Thread.sleep(forTimeInterval: 2)
                    takeScreenshot(name: "Spot Measurement - After Scroll")
                }
            } else {
                print("❌ Record button not found")
                takeScreenshot(name: "Spot Measurement UI - No Record Button")
                XCTFail("Could not find Record button in measurement UI")
                return
            }
        } else {
            print("❌ SmartSpectra Checkup button not found")
            takeScreenshot(name: "Spot Mode - No Checkup Button")
            XCTFail("Could not find SmartSpectra Checkup button")
            return
        }
    }

    @MainActor
    func testMeasurementPreparation() throws {
        // Helper test to prepare for manual measurement testing
        // This test sets up the app in the right state for manual testing

        XCTAssertTrue(waitForAppToLoad(), "App should launch")

        // Test both modes are accessible
        let continuousButton = app.buttons["Switch to Continuous"]
        let spotButton = app.buttons["Switch to Spot"]

        if spotButton.waitForExistence(timeout: 2) {
            print("✅ Continuous mode active")
            takeScreenshot(name: "Continuous Mode Setup")

            spotButton.tap()
            if continuousButton.waitForExistence(timeout: 2) {
                print("✅ Spot mode active")
                takeScreenshot(name: "Spot Mode Setup")
            }
        }

        // Show all available controls
        print("Available buttons:")
        for button in app.buttons.allElementsBoundByIndex {
            if button.exists {
                print("- \(button.label)")
            }
        }

        takeScreenshot(name: "App Ready for Manual Testing")
    }
}
