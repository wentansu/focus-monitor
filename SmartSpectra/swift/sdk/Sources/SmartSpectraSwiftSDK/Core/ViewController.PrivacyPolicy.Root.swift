import UIKit
import WebKit

extension ViewController.PrivacyPolicy {

    class Root: UIViewController, WKNavigationDelegate {
        let webView: WKWebView = {
            let webView = WKWebView()
            webView.translatesAutoresizingMaskIntoConstraints = false
            return webView
        }()

        let agreeButton: UIButton = {
            let button = UIButton(type: .system)
            button.translatesAutoresizingMaskIntoConstraints = false
            button.setTitle("Agree", for: .normal)
            button.titleLabel?.font = UIFont.boldSystemFont(ofSize: 18)
            button.setTitleColor(.white, for: .normal)
            button.backgroundColor = .systemBlue
            button.layer.cornerRadius = 10
            return button
        }()
        
        let declineButton: UIButton = {
            let button = UIButton(type: .system)
            button.translatesAutoresizingMaskIntoConstraints = false
            button.setTitle("Decline", for: .normal)
            button.titleLabel?.font = UIFont.boldSystemFont(ofSize: 18)
            button.setTitleColor(.white, for: .normal)
            button.backgroundColor = .systemRed
            button.layer.cornerRadius = 10
            return button
        }()
        
        var onCompletion: (() -> Void)?

        override func viewDidLoad() {
            super.viewDidLoad()

            view.backgroundColor = .white

            view.addSubview(webView)
            view.addSubview(agreeButton)
            view.addSubview(declineButton)

            NSLayoutConstraint.activate([
                webView.topAnchor.constraint(equalTo: view.topAnchor, constant: 20),
                webView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 20),
                webView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -20),
                webView.bottomAnchor.constraint(equalTo: view.bottomAnchor, constant: -80),

                agreeButton.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -20),
                agreeButton.heightAnchor.constraint(equalToConstant: 40),
                agreeButton.widthAnchor.constraint(equalTo: view.widthAnchor, multiplier: 0.45),
                
                declineButton.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 20),
                declineButton.trailingAnchor.constraint(equalTo: agreeButton.leadingAnchor, constant: -10),
                declineButton.bottomAnchor.constraint(equalTo: view.safeAreaLayoutGuide.bottomAnchor, constant: -20),
                declineButton.heightAnchor.constraint(equalToConstant: 40),
                declineButton.widthAnchor.constraint(equalTo: view.widthAnchor, multiplier: 0.45)
            ])

            // Load the agreement page
            if let url = URL(string: "https://api.physiology.presagetech.com/privacypolicy") {
                webView.navigationDelegate = self
                webView.load(URLRequest(url: url))
            }

            // Add a target for the Agree button
            agreeButton.addTarget(self, action: #selector(agreeButtonTapped), for: .touchUpInside)
            
            // Add a target for the Decline button
            declineButton.addTarget(self, action: #selector(declineButtonTapped), for: .touchUpInside)
        }

        @objc func agreeButtonTapped() {
            // Dismiss the view controller when the user clicks "Agree"
            UserDefaults.standard.set(true, forKey: "HasAgreedToPrivacyPolicy")
            dismiss(animated: true, completion: onCompletion)
        }
        
        @objc func declineButtonTapped() {
            // Handle the decline action
            UserDefaults.standard.set(false, forKey: "HasAgreedToPrivacyPolicy")
            //TODO: show a toast
//            showToast(message: "You have declined the privacy policy.")
            dismiss(animated: true, completion: onCompletion)
        }

        func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
            // Inject JavaScript to hide any UI elements (e.g., web menus, URL bar)
            let hideScript = "document.documentElement.style.webkitTouchCallout='none';"
                + "document.documentElement.style.webkitUserSelect='none';"
            webView.evaluateJavaScript(hideScript, completionHandler: nil)
        }

        override func viewDidAppear(_ animated: Bool) {
            super.viewDidAppear(animated)

            // Check if the user has agreed to terms previously
            if UserDefaults.standard.bool(forKey: "HasAgreedToPrivacyPolicy") {
                // Allow other actions or navigation as the user has agreed
            } else {
                // Handle the scenario where the user hasn't agreed yet
            }
        }
    }
}
