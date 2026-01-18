//
//  File.swift
//
//
//  Created by Benyamin Mokhtarpour on 5/22/23.
//

import Foundation
import UIKit
import PresagePreprocessing
import Network
import SwiftUI
import Combine

enum ButtonState {
    case disable, ready, countdown, running
}

@available(iOS 15.0, *)
public extension ViewController.Screening {
    class Root: UIViewController {

        //Screen Brightness
        private var originalBrightness: CGFloat = 0.0

        //MARK: - UI Components
        var sdkConfig = SmartSpectraSwiftSDK.shared.config
        @ObservedObject var sdk = SmartSpectraSwiftSDK.shared
        @ObservedObject var vitalsProcessor = SmartSpectraVitalsProcessor.shared

        var processingStatus = PresageProcessingStatus.idle {
            didSet {
                DispatchQueue.main.async {
                    switch self.processingStatus {
                    case .idle:
                        print("Idle")
                    case .processing:
                        print("Presage Processing")
                        self.moveToProcessing()
                    case .processed:
                        print("Done processing")
                        //assume no error occured
                        self.sdk.updateErrorText("")
                        let vc = ViewController.Processing.Root()
                        vc.processingCompleted(completed: true)
                        self.dismiss(animated: true)
                        //TODO: 9/23/24 Might need to make it conditional for continuous processing
                        self.vitalsProcessor.stopProcessing()
                    case .error:
                        print("Error in processing")
                        let vc = ViewController.Processing.Root()
                        vc.processingCompleted(completed: false)
                        self.dismiss(animated: true)
                        self.vitalsProcessor.stopProcessing()

                    }
                }
            }
        }

        var buttonState: ButtonState = .disable {
            didSet {
                DispatchQueue.main.async {
                    switch self.buttonState {
                    case .disable:
                        self.recordButton.backgroundColor = .lightGray
                        self.recordButton.setTitle("Record", for: .normal)
                        self.recordButton.titleLabel?.font = .systemFont(ofSize: 20)
                    case .ready:
                        self.recordButton.backgroundColor = self.recordButtonOptionObject?.backgroundColor
                        self.recordButton.setTitle("Record", for: .normal)
                        self.recordButton.titleLabel?.font = .systemFont(ofSize: 20)
                    case .countdown:
                        self.recordButton.setTitle("\(self.sdkConfig.recordingDelay)", for: .normal)
                        self.recordButton.titleLabel?.font = .boldSystemFont(ofSize: 40)
                    case .running:
                        self.recordButton.backgroundColor = self.recordButtonOptionObject?.backgroundColor
                        self.recordButton.setTitle("Stop", for: .normal)
                        self.recordButton.titleLabel?.font = .systemFont(ofSize: 20)
                    }
                }
            }
        }
        internal var imageHolder: UIImageView = {
            let res = UIImageView()
            res.contentMode = .scaleAspectFit
            res.translatesAutoresizingMaskIntoConstraints = false
            res.backgroundColor = .white
            return res

        }()

        internal var startOverlayView: UIView = {
            let res = UIView()
            res.backgroundColor = UIColor.white.withAlphaComponent(0.9)
            res.translatesAutoresizingMaskIntoConstraints = false
            return res
        }()

        internal var endOverlayView: UIView = {
            let res = UIView()
            res.backgroundColor = UIColor.white.withAlphaComponent(0.9)
            res.translatesAutoresizingMaskIntoConstraints = false
            return res
        }()

        internal var bottomOverlayView: UIView = {
            let res = UIView()
            res.backgroundColor = UIColor.white.withAlphaComponent(0.9)
            res.translatesAutoresizingMaskIntoConstraints = false
            return res
        }()

        internal lazy var recordButton : UIButton = {
            let res = UIButton()
            res.setTitle((self.recordButtonOptionObject?.title) ?? "Record", for: .normal)
            res.layer.cornerRadius = self.recordButtonOptionObject?.corner ?? 40
            res.layer.borderColor = self.recordButtonOptionObject?.borderColor?.cgColor ?? UIColor.white.cgColor
            res.layer.borderWidth = self.recordButtonOptionObject?.borderWidth ?? 2.0
            res.backgroundColor = self.recordButtonOptionObject?.backgroundColor
            res.setTitleColor(self.recordButtonOptionObject?.titleColor ?? .white, for: .normal)
            res.titleLabel?.font =  UIFont.systemFont(ofSize: 20)
            res.translatesAutoresizingMaskIntoConstraints  = false
            return res
        }()


        internal lazy var counterView: UILabel = {
            let res = UILabel()
            res.text  = "\(Int(sdkConfig.measurementDuration))"
            res.font = UIFont.systemFont(ofSize: 40)
            res.backgroundColor = UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00)
            res.layer.cornerRadius = 30
            res.textAlignment = .center
            res.textColor = .white
            res.layer.borderColor = UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00).cgColor
            res.layer.borderWidth = 5
            res.isHidden = false
            res.clipsToBounds = true

            res.translatesAutoresizingMaskIntoConstraints = false
            return res
        }()


        //MARK: - Properties
        
        var onRecordButtonStateChange: ((Bool) -> Void)?

        let viewModel: ViewModel.Screening
        var showWalkThrough: Bool = false
        internal var recordButtonOptionObject: Model.Option.Button.Record?
        internal var countdownTimer: Timer?
        internal var isRecording = false
        var lastStatusCode: StatusCode = .processingNotStarted
        var fpsLabel: UILabel = {
            let label = UILabel()
            label.text = "FPS: 0"
            label.font = UIFont.systemFont(ofSize: 16)
            label.textColor = UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00)
            label.translatesAutoresizingMaskIntoConstraints = false
            return label
        }()

        internal var toastView: Common.View.Toast?
        deinit {
            UIApplication.shared.isIdleTimerDisabled = false
            // Clear old subscriptions to prevent duplication
            cancellables.removeAll()
            Logger.log("ViewController.Screening is De-inited!")
        }

        internal var screeningPlotView: UIHostingController<ScreeningPlotView>?

        private var cancellables = Set<AnyCancellable>()

        init(viewModel: ViewModel.Screening) {
            self.viewModel = viewModel
            self.recordButtonOptionObject = self.viewModel.getCustomProperty()
            self.fpsLabel.isHidden = !sdkConfig.showFps
            super.init(nibName: nil, bundle: nil)
        }

        required init?(coder aDecoder: NSCoder) {
            fatalError("init(coder:) has not been implemented")
        }

        public override func viewDidLoad() {
            super.viewDidLoad()
            self.navigationController?.interactivePopGestureRecognizer?.delegate = self
            self.title = "Presage SmartSpectra"
            self.view.backgroundColor = .white
            navigationController?.setNavigationBarHidden(false, animated: true)
            setCustomBackButton()
            setTitleView()
            setupUIComponents()
            self.counterView.text = "\(Int(sdkConfig.measurementDuration))"
            self.vitalsProcessor.startProcessing()

            // Check for internet connection
            monitorInternetConnection()
            // setup combine observers
            setupObservers()
        }
        
        private func setupObservers() {
            print("Setting up observers...")
            // show counterview only on spot mode
            // TODO: 11/14/24 remove this once view is refactored in SwiftUI
            sdk.config.$smartSpectraMode
                .receive(on: DispatchQueue.main) // Ensure updates occur on the main thread
                .sink { [weak self] newMode in
                    self?.counterView.isHidden = newMode != .spot
                }
                .store(in: &cancellables)
            
            // TODO: move to swiftUI for more efficient handling of published variables
            vitalsProcessor.$lastStatusCode
                .receive(on: DispatchQueue.main)
                .sink { [weak self] statusCode in
                    self?.updateButtonState(for: statusCode)
                }
                .store(in: &cancellables)
            vitalsProcessor.$statusHint
                .receive(on: DispatchQueue.main)
                .sink { [weak self] statusHint in
                    self?.showToast(msg: statusHint)
                }
                .store(in: &cancellables)

            vitalsProcessor.$imageOutput
                .receive(on: DispatchQueue.main)
                .sink { [weak self] image in
                    self?.imageHolder.image = image
                }
                .store(in: &cancellables)

            vitalsProcessor.$counter
                .receive(on: DispatchQueue.main)
                .sink { [weak self] counter in
                    self?.counterView.text = "\(Int(counter))"
                }
                .store(in: &cancellables)

            vitalsProcessor.$fps
                .receive(on: DispatchQueue.main)
                .sink { [weak self] fps in
                    self?.fpsLabel.text = "FPS: \(fps)"
                }
                .store(in: &cancellables)

            vitalsProcessor.$processingStatus
                .receive(on: DispatchQueue.main)
                .sink { [weak self] status in
                    self?.updateProcessingStatus(for: status)
                }
                .store(in: &cancellables)
        }

        private func updateButtonState(for statusCode: StatusCode) {
            DispatchQueue.main.async {
                // update button state
                if statusCode != .ok {
                    self.buttonState = .disable
                } else {
                    self.buttonState = self.isRecording ? .running : .ready
                }
            }
        }

        private func updateProcessingStatus(for status: PresageProcessingStatus?) {
            DispatchQueue.main.async {
                self.processingStatus = status ?? .idle
            }
        }

        private func monitorInternetConnection() {
            let monitor = NWPathMonitor()
            let queue = DispatchQueue(label: "InternetConnectionMonitor")

            monitor.pathUpdateHandler = { [weak self] path in
                if path.status == .satisfied {
                    print("We're connected!")
                    // You can perform tasks that require internet here
                } else {
                    print("No connection.")
                    DispatchQueue.main.async {
                        self?.showNoInternetAlert()
                    }
                }
            }

            monitor.start(queue: queue)
        }

        private func showNoInternetAlert() {
            let alert = UIAlertController(title: "No Internet Connection", message: "Please check your internet connection and try again.", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { _ in
                self.backButtonPressed() // Assuming you want to use your custom back function
            }))
            self.present(alert, animated: true, completion: nil)
        }

        public override func viewDidLayoutSubviews() {
            super.viewDidLayoutSubviews()
        }

        public override func viewWillAppear(_ animated: Bool) {
            super.viewWillAppear(animated)
            originalBrightness = UIScreen.main.brightness  // Store current brightness
            UIScreen.main.brightness = 1.0
        }

        public override func viewWillDisappear(_ animated: Bool) {
            super.viewWillDisappear(animated)
            UIScreen.main.brightness = originalBrightness
        }

        private func setTitleView() {
            let titleView = UIView()
            let titleLabel = UILabel()
            titleLabel.text = "Presage SmartSpectra"
            titleLabel.textColor = .black
            titleLabel.font = UIFont.systemFont(ofSize: 17, weight: .bold)
            titleLabel.sizeToFit()
            titleView.addSubview(titleLabel)
            titleLabel.center = titleView.center
            navigationItem.titleView = titleView
        }

        private func setCustomBackButton() {
            let backButton = UIButton(type: .system)
            backButton.setImage(UIImage(systemName: "arrow.left"), for: .normal)
            backButton.addTarget(self, action: #selector(backButtonPressed), for: .touchUpInside)
            backButton.tintColor = .black
            let customBackButton = UIBarButtonItem(customView: backButton)
            navigationItem.leftBarButtonItem = customBackButton
        }

        @objc func backButtonPressed() {
            DispatchQueue.main.async {
                self.vitalsProcessor.stopRecording()
                self.vitalsProcessor.stopProcessing()
                self.dismiss(animated: true, completion: nil)
            }
        }
    }
}
@available(iOS 15.0, *)
extension ViewController.Screening.Root: UIGestureRecognizerDelegate {
    public func gestureRecognizer(_ gestureRecognizer: UIGestureRecognizer, shouldBeRequiredToFailBy otherGestureRecognizer: UIGestureRecognizer) -> Bool {
        return true
    }

    public func gestureRecognizerShouldBegin(_ gestureRecognizer: UIGestureRecognizer) -> Bool {
        guard gestureRecognizer.isEqual(self.navigationController?.interactivePopGestureRecognizer) else { return true }
        self.vitalsProcessor.stopRecording()
        return true
    }
}


@available(iOS 15.0, *)
extension ViewController.Screening.Root {
    // TODO: This seems unnecessary and performance intensive. Should use a static TextView and update the text and visibility instead
    func showToast(msg: String) {
        DispatchQueue.main.async {
            if self.toastView == nil {
                // Create a new instance of Common.View.Toast
                let toast = Common.View.Toast(backgroundColor: .black, textColor: .white)

                // Configure the toast with the provided message
                toast.setMessage(msg)

                // Add the toast to the view hierarchy
                toast.addToView(self.bottomOverlayView)

                // Set the currentToast property to the new toast instance
                self.toastView = toast

                // Animate the toast's appearance
                UIView.animate(withDuration: 0.3, delay: 0, options: .curveEaseInOut, animations: {
                    toast.frame.size.height = 70
                }, completion: nil)
            } else {
                self.toastView?.setMessage(msg)
            }
        }
    }
}
