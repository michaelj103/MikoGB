//
//  NoROMView.swift
//  MikoGB
//
//  Created on 5/8/21.
//

import Cocoa

class NoROMView : NSView {
    
    var delegate: NoROMViewDelegate?
    private lazy var button: NSButton = NSButton(title: "Choose ROM…", target: self, action: #selector(NoROMView.tappedButton))
    
    
    init() {
        super.init(frame: NSZeroRect)
        self.addSubview(button)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    @objc private func tappedButton(_ sender: NSButton?) {
        delegate?.didSelectChooseRom(view: self)
    }
    
    override func layout() {
        super.layout()
        
        let bounds = self.bounds
        button.sizeToFit()
        let xPos = (bounds.size.width - button.frame.size.width) / 2.0
        let yPos = (bounds.size.height - button.frame.size.height) / 2.0
        let buttonOrigin = CGPoint(x: xPos, y: yPos)
        button.frame.origin = buttonOrigin
    }
}

protocol NoROMViewDelegate {
    func didSelectChooseRom(view: NoROMView)
}
