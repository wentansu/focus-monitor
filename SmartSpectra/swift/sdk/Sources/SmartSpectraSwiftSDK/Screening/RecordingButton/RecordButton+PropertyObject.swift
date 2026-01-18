//
//  RecordButtonPropertyObject.swift
//  
//
//  Created by Benyamin Mokhtarpour on 5/22/23.
//

import Foundation

class RecordButtonPropertyObject: ScreeningPageCustomizingProvider {
    let recordButton: Model.Option.Button.Record?
    
    init(recordButton: Model.Option.Button.Record? = nil) {
        self.recordButton = recordButton
    }
}


