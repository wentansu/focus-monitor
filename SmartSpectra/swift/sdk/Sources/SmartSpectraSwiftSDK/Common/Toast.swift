//
//  File.swift
//  
//
//  Created by Benyamin Mokhtarpour on 6/21/23.
//

import Foundation
import UIKit

extension Common {
    struct View {
        
        class Toast: UIView {
            private let messageLabel: UILabel = {
                let label = UILabel()
                label.textColor = .white
                label.textAlignment = .center
                label.numberOfLines = 0
                return label
            }()
            
            private let toastBackgroundColor: UIColor
            private let toastTextColor: UIColor
            
            init(backgroundColor: UIColor, textColor: UIColor) {
                self.toastBackgroundColor = backgroundColor
                self.toastTextColor = textColor
                super.init(frame: .zero)
                
                configureUI()
            }
            
            required init?(coder: NSCoder) {
                fatalError("init(coder:) has not been implemented")
            }
            
            private func configureUI() {
                backgroundColor = toastBackgroundColor
                layer.cornerRadius = 10
                layer.masksToBounds = true
                
                addSubview(messageLabel)
                messageLabel.translatesAutoresizingMaskIntoConstraints = false
                
                NSLayoutConstraint.activate([
                    messageLabel.topAnchor.constraint(equalTo: topAnchor, constant: 8),
                    messageLabel.leadingAnchor.constraint(equalTo: leadingAnchor, constant: 8),
                    messageLabel.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -8),
                    messageLabel.bottomAnchor.constraint(equalTo: bottomAnchor, constant: -8)
                ])
            }
            
            func setMessage(_ message: String) {
                messageLabel.text = message
                messageLabel.textColor = toastTextColor
            }
            
            func addToView(_ view: UIView) {
                view.addSubview(self)
                self.translatesAutoresizingMaskIntoConstraints = false
                
                NSLayoutConstraint.activate([
                    self.topAnchor.constraint(equalTo: view.topAnchor, constant: 32),
                    self.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: -24),
                    self.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: 24),
                    self.heightAnchor.constraint(greaterThanOrEqualToConstant: 48)
                ])
            }
            
            func show() {
                DispatchQueue.main.async {
                    
                    self.tag = 999
                    guard let windowScene = UIApplication.shared.connectedScenes.first as? UIWindowScene,
                    let window = windowScene.windows.first  else  { return }

                    self.frame = CGRect(x: 16, y: window.frame.height - 100, width: window.frame.width - 32, height: 0)
                    if let _toastv = window.subviews.first(where: { $0.tag == 999}) {
                        (
                            _toastv as? Common.View.Toast
                        )?.setMessage(self.messageLabel.text ?? "")
                    } else {
                        window.addSubview(self)
                    }
                    
                    UIView.animate(withDuration: 0.1, delay: 0, options: .curveEaseInOut, animations: {
                        self.frame.size.height = 70
                    }, completion: nil)
                }
            }
        }
    }
}
