//
//  WindowController.swift
//  MikoGB
//
//  Created on 5/8/21.
//

import Cocoa

class WindowController : NSWindowController {
    override func windowDidLoad() {
        super.windowDidLoad()
        
        guard let window = self.window else {
            assertionFailure("Window must be non-nil here")
            return
        }
        
        window.contentAspectRatio = CGSize(width: 10, height: 9)
        window.contentMinSize = CGSize(width: 160, height: 144)
    }
}
