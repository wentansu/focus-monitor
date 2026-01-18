//
//  ViewController.Processing + ProcessingViewModelDelegate.swift
//  
//
//  Created by Benyamin Mokhtarpour on 6/25/23.
//

import Foundation
@available(iOS 15.0, *)
extension ViewController.Processing.Root {
    func updateProgress(_ progress: Float) {
        DispatchQueue.main.async {
            self.progressView.progress = progress
        }
    }
    /**
     Handles the completion of the processing task.
     
     This method is called by the view model when the processing task is completed. It performs any necessary actions upon completion, such as poping and the view controller.
     */
    func processingCompleted(completed: Bool) {
        if completed {
            animationView.checkmarkView.shape = .checkmark
            statusLabel.text = "Processing the data is completed"
            DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
                self.navigationController?.popToRootViewController(animated: true)
            }
        } else {
            animationView.checkmarkView.shape = .cross
            statusLabel.text = "Analyzing data has faced an error. Please contact support in the case of an api key issue."
            DispatchQueue.main.asyncAfter(deadline: .now() + 2.0) {
                self.navigationController?.popToRootViewController(animated: true)
            }
        }
    }
}
