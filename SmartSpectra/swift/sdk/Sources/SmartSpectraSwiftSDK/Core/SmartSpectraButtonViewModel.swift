//
//  SmartSpectraButtonViewModel.swift
//
//
//  Created by Ashraful Islam on 8/14/23.
//

import Foundation
import UIKit
import Combine
import SwiftUI
@available(iOS 15.0, *)
/// View model powering the default ``SmartSpectraButtonView``.
///
/// This type coordinates presentation of onboarding flows and the screening
/// page when the user taps the record button. Apps may subclass or replace this
/// view model to customize the button behavior.
final class SmartSpectraButtonViewModel: ObservableObject {
    
    internal let sdk = SmartSpectraSwiftSDK.shared
    public let responseSubject = PassthroughSubject<String, Never>()

    public init() {
        // Empty public initializer
    }
    
    /// Presents the onboarding tutorial and legal agreements if needed.
    ///
    /// This method ensures the walkthrough, terms of service and privacy policy
    /// have been acknowledged before proceeding.
    /// - Parameter completion: Optional closure executed after onboarding is
    ///   complete.
    private func showTutorialAndAgreementIfNecessary(completion: (() -> Void)? = nil) {
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            let walkthroughShown = UserDefaults.standard.bool(forKey: "WalkthroughShown")
            let hasAgreedToTerms = UserDefaults.standard.bool(forKey: "HasAgreedToTerms")
            let hasAgreedToPrivacyPolicy = UserDefaults.standard.bool(forKey: "HasAgreedToPrivacyPolicy")
            
            func showAgreements() {
                if !hasAgreedToTerms {
                    self.presentUserAgreement {
                        showPrivacyPolicy()
                    }
                } else {
                    showPrivacyPolicy()
                }
            }
            
            func showPrivacyPolicy() {
                if !hasAgreedToPrivacyPolicy {
                    self.presentPrivacyPolicy(completion: completion)
                } else {
                    completion?()
                }
            }
            
            if !walkthroughShown {
                self.handleWalkTrough {
                    showAgreements()
                }
            } else {
                showAgreements()
            }
        }
    }


    /// Presents the tutorial walkthrough UI.
    ///
    /// - Parameter completion: Executed when the tutorial view is dismissed.
    internal func handleWalkTrough(completion: (() -> Void)? = nil) {
        let tutorialView = TutorialView(onTutorialCompleted: {
            completion?()
        })
        let hostingController = UIHostingController(rootView: tutorialView)
        findViewController()?.present(hostingController, animated: true, completion: nil)
        
        UserDefaults.standard.set(true, forKey: "WalkthroughShown")
    }
    
    /// Displays the hosted Terms of Service screen.
    /// - Parameter completion: Executed after the agreement view is dismissed.
    private func presentUserAgreement(completion: (() -> Void)? = nil) {
        checkInternetConnectivity { [weak self] isConnected in
            DispatchQueue.main.async {
                if isConnected {
                    let agreementViewController = ViewController.Agreement.Root()
                    agreementViewController.onCompletion = completion
                    let navigationController = UINavigationController(rootViewController: agreementViewController)
                    navigationController.modalPresentationStyle = .fullScreen
                    navigationController.modalTransitionStyle = .coverVertical

                    self?.findViewController()?.present(navigationController, animated: true, completion: nil)
                } else {
                    self?.showNoInternetConnectionAlert()
                }
            }
        }
    }
    
    /// Displays the hosted Privacy Policy screen.
    /// - Parameter completion: Executed after the policy view is dismissed.
    private func presentPrivacyPolicy(completion: (() -> Void)? = nil) {
        checkInternetConnectivity { [weak self] isConnected in
            DispatchQueue.main.async {
                if isConnected {
                    let privacyPolicyViewController = ViewController.PrivacyPolicy.Root()
                    privacyPolicyViewController.onCompletion = completion
                    let navigationController = UINavigationController(rootViewController: privacyPolicyViewController)
                    navigationController.modalPresentationStyle = .fullScreen
                    navigationController.modalTransitionStyle = .coverVertical

                    self?.findViewController()?.present(navigationController, animated: true, completion: nil)
                } else {
                    self?.showNoInternetConnectionAlert()
                }
            }
        }
    }
    
    /// Checks whether the device currently has internet access.
    /// - Parameter completion: Callback invoked with `true` when reachable.
    private func checkInternetConnectivity(completion: @escaping (Bool) -> Void) {
        guard let url = URL(string: "https://www.google.com") else {
            completion(false)
            return
        }
        let task = URLSession.shared.dataTask(with: URLRequest(url: url)) { _, response, _ in
            if let httpResponse = response as? HTTPURLResponse, httpResponse.statusCode == 200 {
                completion(true)
            } else {
                completion(false)
            }
        }
        task.resume()
    }
    
    /// Presents an alert indicating that network connectivity is unavailable.
    private func showNoInternetConnectionAlert() {
        if let rootViewController = findViewController() {
            let alert = UIAlertController(title: "No Internet Connection", message: "Please check your internet connection and try again.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
            rootViewController.present(alert, animated: true, completion: nil)
        }
    }

    /// Opens an external link using `UIApplication`.
    ///
    /// - Parameter urlString: Fully qualified URL string.
    /// - Note: Invalid URLs are ignored.
    func openSafari(withURL urlString: String) {
        guard let url = URL(string: urlString) else {
            return // Invalid URL, handle error or show an alert
        }

        UIApplication.shared.open(url, options: [:], completionHandler: nil)
    }
    
    /// Presents an action sheet containing quick links to tutorials and legal
    /// documents.
    @objc func showActionSheet() {
        let actionSheet = UIAlertController(title: nil, message: nil, preferredStyle: .actionSheet)

        // Add options
        actionSheet.addAction(UIAlertAction(title: "Show Tutorial", style: .default) { _ in
            UserDefaults.standard.set(false, forKey: "WalkthroughShown")
            self.handleWalkTrough()
        })
        actionSheet.addAction(UIAlertAction(title: "Instructions for Use", style: .default) { _ in
            self.openSafari(withURL: "https://api.physiology.presagetech.com/instructions")
        })
        actionSheet.addAction(UIAlertAction(title: "Terms of Service", style: .default) { _ in
            self.presentUserAgreement()
        })
        actionSheet.addAction(UIAlertAction(title: "Privacy Policy", style: .default) { _ in
            self.presentPrivacyPolicy()
        })

        actionSheet.addAction(UIAlertAction(title: "Contact Us", style: .default) { _ in
            self.openSafari(withURL: "https://api.physiology.presagetech.com/contact")
        })
        

        // Add cancel button with red text color
        let cancelButton = UIAlertAction(title: "Cancel", style: .cancel) { _ in
            // Handle cancellation
        }
        cancelButton.setValue(UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00), forKey: "titleTextColor")
        actionSheet.addAction(cancelButton)
        let viewController = self.findViewController()

        // Show action sheet
        viewController?.present(actionSheet, animated: true, completion: nil)
    }
    
    /// Handles SmartSpectra SDK initialization and presents the screening page
    /// when the record button is tapped.
    internal func smartSpectraButtonTapped() {
        
        // show tutorial first time the user taps the button
        showTutorialAndAgreementIfNecessary { [weak self] in
            guard let self = self else { return }
            
            // Add code here to initialize the SmartSpectra SDK
            // Once the SDK is initialized, you can proceed with presenting the screening page.
            if UserDefaults.standard.bool(forKey: "HasAgreedToTerms")  && UserDefaults.standard.bool(forKey: "HasAgreedToPrivacyPolicy") {
                let sPage = SmartSpectra().ScreeningPage(recordButton: Model.Option.Button.Record.init(backgroundColor: UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00), titleColor: .white, borderColor: UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00), title: "Record"))
                
                // Assuming you have access to the view controller where the button is added
                let viewController = self.findViewController()
                // Check if the current view controller is embedded in a navigation controller
                if let navigationController = viewController?.navigationController {
                    navigationController.pushViewController(sPage, animated: true)
                } else {
                    // If not, present it modally
                    let nav = UINavigationController(rootViewController: sPage)
                    nav.modalPresentationStyle = .overFullScreen
                    viewController?.present(nav, animated: true)
                }
            }
        }
    }

    /// Helper to locate the root view controller for presenting UIKit screens.
    private func findViewController() -> UIViewController? {
        guard let scene = UIApplication.shared.connectedScenes.first as? UIWindowScene,
              let window = scene.windows.first,
              let rootViewController = window.rootViewController else {
            return nil
        }
        return rootViewController
    }

    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
}


