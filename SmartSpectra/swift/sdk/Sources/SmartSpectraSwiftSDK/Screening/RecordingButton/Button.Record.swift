//
//  File.swift
//  
//
//  Created by Benyamin Mokhtarpour on 5/22/23.
//

import Foundation
import UIKit

public extension Model.Option.Button {
    struct Record {
        var backgroundColor: UIColor? = .systemRed
        var titleColor: UIColor? = nil
        var borderColor: UIColor? = .white
        var borderWidth: CGFloat? = nil
        var corner: CGFloat? = nil
        var title: String? = nil

        var backgroundImage: UIImage? = nil
    }
}
public extension Model.Option.Button.Record {
    init(image: UIImage) {
        backgroundImage = image
    }
    init(backgroundColor: UIColor,
         titleColor: UIColor? = nil,
         borderColor: UIColor? = nil,
         borderWidth: CGFloat? = nil,
         corner: CGFloat? = nil,
         title: String? = nil
         
    ) {
        self.backgroundColor = backgroundColor
        self.titleColor  =       titleColor
        self.borderColor =       borderColor
        self.borderWidth =       borderWidth
        self.corner      =       corner
        self.title       =       title

    }
}

