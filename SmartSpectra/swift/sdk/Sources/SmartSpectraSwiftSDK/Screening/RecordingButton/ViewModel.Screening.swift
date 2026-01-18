//
//  ViewModel.Screening.swift
//  
//
//  Created by Benyamin Mokhtarpour on 5/22/23.
//

import Foundation

extension ViewModel {

    class Screening {
        private let propertyProvider: ScreeningPageCustomizingProvider?
        
        init(propertyProvider: ScreeningPageCustomizingProvider? = nil) {
            self.propertyProvider = propertyProvider
        }
        
        func getCustomProperty() -> Model.Option.Button.Record? {
            return propertyProvider?.recordButton
        }
    }
    
}
