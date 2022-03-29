//
//  SizeExtension.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/22/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

extension CGSize {
    func fittingSize(_ aspect: CGFloat) -> CGSize {
        // scale an imaginary size with dimensions width=aspect, height=1.0 to fit self
        // the min scale factor between height and width is the one to use
        let widthRatio = width / aspect
        let heightRatio = height
        let scale = min(widthRatio, heightRatio)
        let finalSize = CGSize(width: aspect * scale, height: scale)
        return finalSize
    }
}

extension CGRect {
    // return rect fitting self with the given aspect and centered
    func fitRect(_ aspect: CGFloat) -> CGRect {
        let fitSize = size.fittingSize(aspect)
        let x = minX + (width - fitSize.width) / 2.0
        let y = minY + (height - fitSize.height) / 2.0
        let finalRect = CGRect(x: x, y: y, width: fitSize.width, height: fitSize.height)
        return finalRect
    }
}
