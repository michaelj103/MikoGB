//
//  DPadView.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/29/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

enum DPadDirection : CustomStringConvertible {
    case Neutral
    case Up
    case Down
    case Left
    case Right
    
    var description: String {
        switch self {
        case .Neutral:
            return "Neutral"
        case .Up:
            return "Up"
        case .Down:
            return "Down"
        case .Left:
            return "Left"
        case .Right:
            return "Right"
        }
    }
}

class DPadView : UIView {
    
    weak var delegate: DPadDelegate?
    
    private(set) var currentDirection = DPadDirection.Neutral {
        didSet {
            if currentDirection != oldValue {
                dPadImage.image = preloadedImages[currentDirection]
                delegate?.directionChanged(currentDirection, oldDirection: oldValue)
            }
        }
    }
    
    private var dPadImage: UIImageView
    private var preloadedImages = [DPadDirection : UIImage]()
    override init(frame: CGRect) {
        let neutralImage = UIImage(named: "DPad", in: nil, with: nil)
        preloadedImages[.Neutral] = neutralImage
        preloadedImages[.Up] = UIImage(named: "DPad_Up", in: nil, with: nil)
        preloadedImages[.Down] = UIImage(named: "DPad_Down", in: nil, with: nil)
        preloadedImages[.Left] = UIImage(named: "DPad_Left", in: nil, with: nil)
        preloadedImages[.Right] = UIImage(named: "DPad_Right", in: nil, with: nil)
        dPadImage = UIImageView(image: neutralImage)
        super.init(frame: frame)
        self.addSubview(dPadImage)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func layoutSubviews() {
        let imageScale = 1.8
        dPadImage.frame = self.bounds.scaleRect(imageScale)
        super.layoutSubviews()
    }
    
    private func _updateDirectionOffset() {
        guard let touch = currentTouch else {
            currentDirection = .Neutral
            return
        }
        
        let myBounds = self.bounds
        let threshold = myBounds.height * 0.05
        let center = myBounds.center
        
        let touchLocation = touch.location(in: self)
        let vector = touchLocation - center
        let verticalMag = abs(vector.y)
        let horizontalMag = abs(vector.x)
        
        if verticalMag >= horizontalMag {
            // check for up/down
            if verticalMag > threshold {
                currentDirection = vector.y > 0 ? .Down : .Up
            } else {
                currentDirection = .Neutral
            }
        } else {
            // check for right/left
            if horizontalMag > threshold {
                currentDirection = vector.x > 0 ? .Right : .Left
            } else {
                currentDirection = .Neutral
            }
        }
    }
    
    private var currentTouch: UITouch?
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        if currentTouch != nil {
            // new touches when we're already tracking
            return
        }
        
        currentTouch = touches.first
        _updateDirectionOffset()
    }
    
    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard let trackingTouch = currentTouch else {
            return
        }
        
        if touches.contains(trackingTouch) {
            _updateDirectionOffset()
        }
    }
    
    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard let trackingTouch = currentTouch else {
            return
        }
        
        if touches.contains(trackingTouch) {
            currentTouch = nil
            _updateDirectionOffset()
        }
    }
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard let trackingTouch = currentTouch else {
            return
        }
        
        if touches.contains(trackingTouch) {
            currentTouch = nil
            _updateDirectionOffset()
        }
    }
}

protocol DPadDelegate : AnyObject {
    func directionChanged(_ newDirection: DPadDirection, oldDirection: DPadDirection)
}
