//
//  LinkSessionManager.swift
//  MikoGB
//
//  Created by Michael Brandt on 8/17/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class LinkSessionManager: NSObject, GBEngineSerialDestination {
    private let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func engine(_ engine: GBEngine, presentByte byte: UInt8) {
        // nothing for now
    }
    
    func engine(_ engine: GBEngine, pushByte byte: UInt8) {
        engine.receivePulledSerialByte(0xFF)
    }
}
