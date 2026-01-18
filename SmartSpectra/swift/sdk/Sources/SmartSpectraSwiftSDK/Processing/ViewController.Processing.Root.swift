//
//  File.swift
//
//
//  Created by Benyamin Mokhtarpour on 6/21/23.
//

import Foundation
import UIKit

@available(iOS 15.0, *)
extension ViewController.Processing {
    /**
     The `Root` class is the view controller responsible for managing the processing screen.
     */
    class Root: UIViewController {
        // UI elements
        let progressView = UIProgressView(progressViewStyle: .default)
        lazy var statusLabel: UILabel = {
            let res = UILabel()
            res.translatesAutoresizingMaskIntoConstraints = false
            res.font = UIFont.systemFont(ofSize: 20, weight: .bold)
            res.text = "Uploading the captured data for analysis..."
            res.textAlignment = .center
            res.textColor = .black
            res.numberOfLines = 0
            return res
        }()
        lazy var animationView : AnimationView = {
            let res = AnimationView()
            res.translatesAutoresizingMaskIntoConstraints = false

            return res
        }()

        override func viewDidLoad() {
            self.view.backgroundColor = .white
            super.viewDidLoad()
            navigationController?.setNavigationBarHidden(true, animated: true)
            // Set up UI elements
            progressView.translatesAutoresizingMaskIntoConstraints = false
            progressView.progressTintColor = UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00)
            statusLabel.translatesAutoresizingMaskIntoConstraints = false
            statusLabel.textAlignment = .center

            view.addSubview(progressView)
            view.addSubview(statusLabel)
            view.addSubview(animationView)

            NSLayoutConstraint.activate([
                statusLabel.centerYAnchor.constraint(equalTo: view.centerYAnchor),
                statusLabel.leadingAnchor.constraint(equalTo: self.view.leadingAnchor, constant: 16),
                statusLabel.trailingAnchor.constraint(equalTo: self.view.trailingAnchor, constant: -16),

                progressView.topAnchor.constraint(equalTo: statusLabel.bottomAnchor, constant: 20),
                progressView.centerXAnchor.constraint(equalTo: view.centerXAnchor),
                progressView.widthAnchor.constraint(equalTo: view.widthAnchor, multiplier: 0.8),

                animationView.heightAnchor.constraint(equalToConstant: 105),
                animationView.widthAnchor.constraint(equalToConstant: 100),
                animationView.centerXAnchor.constraint(equalTo: view.centerXAnchor),

                animationView.bottomAnchor.constraint(equalTo: statusLabel.topAnchor, constant: -40)
            ])

            // Perform the animations
            animationView.animateCheckmark()
            animationView.animateHorizontalLine()
        }
    }
}







class AnimationView: UIView {

    var checkmarkView: AnimationStatusView!
    var horizontalLineView: UIView!

    override init(frame: CGRect) {
        super.init(frame: frame)

        // Create the checkmark view
        checkmarkView = AnimationStatusView(frame: CGRect(x: 0, y: 0, width: 100, height: 100))
        checkmarkView.backgroundColor = UIColor.clear
        checkmarkView.shape = .arrow
        addSubview(checkmarkView)

        // Create the horizontal line view
        horizontalLineView = UIView(frame: CGRect(x: 25, y: 105, width: 50, height: 5))
        horizontalLineView.backgroundColor = UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00)
        horizontalLineView.layer.cornerRadius = 2.5
        addSubview(horizontalLineView)
    }

    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }

    func animateCheckmark() {
        UIView.animate(withDuration: 0.5, delay: 0, usingSpringWithDamping: 0.7, initialSpringVelocity: 0, options: [.repeat, .autoreverse], animations: {
            self.checkmarkView.transform = CGAffineTransform(scaleX: 1.2, y: 1.2)
        }, completion: nil)
    }

    func animateHorizontalLine() {
        UIView.animate(withDuration: 0.5, delay: 0, usingSpringWithDamping: 0.7, initialSpringVelocity: 0, options: [.repeat, .autoreverse], animations: {
            self.horizontalLineView.transform = CGAffineTransform(scaleX: 2, y: 1)
        }, completion: nil)
    }
}

class AnimationStatusView: UIView {

    enum Shape {
        case checkmark
        case arrow
        case cross
    }

    var shape: Shape = .checkmark {
        didSet {
            setNeedsDisplay()
        }
    }

    override func draw(_ rect: CGRect) {
        guard UIGraphicsGetCurrentContext() != nil else { return }

        let path = UIBezierPath()

        switch shape {
        case .checkmark:
            drawCheckmark(in: rect, path: path)
        case .arrow:
            drawArrow(in: rect, path: path)
        case .cross:
            drawCross(in: rect, path: path)
        }

        path.lineWidth = 15
        path.lineCapStyle = .round

        UIColor(red: 0.94, green: 0.34, blue: 0.36, alpha: 1.00).setStroke()
        path.stroke()
    }

    private func drawCheckmark(in rect: CGRect, path: UIBezierPath) {
        let startPoint = CGPoint(x: rect.width * 0.2, y: rect.height * 0.5)
        let middlePoint = CGPoint(x: rect.width * 0.45, y: rect.height * 0.75)
        let endPoint = CGPoint(x: rect.width * 0.8, y: rect.height * 0.25)

        path.move(to: startPoint)
        path.addLine(to: middlePoint)
        path.addLine(to: endPoint)
    }

    private func drawArrow(in rect: CGRect, path: UIBezierPath) {
        let startPoint = CGPoint(x: rect.width * 0.5, y: rect.height * 0.8)
        let topPoint = CGPoint(x: rect.width * 0.5, y: rect.height * 0.2)
        let endPoint1 = CGPoint(x: rect.width * 0.25, y: rect.height * 0.4)
        let endPoint2 = CGPoint(x: rect.width * 0.75, y: rect.height * 0.4)

        path.move(to: startPoint)
        path.addLine(to: topPoint)
        path.move(to: topPoint)
        path.addLine(to: endPoint1)
        path.move(to: topPoint)
        path.addLine(to: endPoint2)
    }

    private func drawCross(in rect: CGRect, path: UIBezierPath) {
        let startPoint1 = CGPoint(x: rect.width * 0.2, y: rect.height * 0.2)
        let endPoint1 = CGPoint(x: rect.width * 0.8, y: rect.height * 0.8)
        let startPoint2 = CGPoint(x: rect.width * 0.8, y: rect.height * 0.2)
        let endPoint2 = CGPoint(x: rect.width * 0.2, y: rect.height * 0.8)

        path.move(to: startPoint1)
        path.addLine(to: endPoint1)
        path.move(to: startPoint2)
        path.addLine(to: endPoint2)
    }
}

