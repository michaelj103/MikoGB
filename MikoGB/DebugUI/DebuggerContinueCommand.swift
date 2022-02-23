//
//  DebuggerContinueCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/21/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerContinueCommand : DebuggerCommand {
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func respondsToInput(_ input: [String]) -> Bool {
        guard let first = input.first else {
            return false
        }
        if first == "continue" || first == "c" {
            return true
        }
        return false
    }
    
    func runCommand(input: [String], outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        guard input.count == 1 else {
            outputHandler("Command \'continue\' expects no arguments")
            completion()
            return
        }
        
        self.engine.setDesiredRunnable(true) {
            completion()
        }
    }
    
}
