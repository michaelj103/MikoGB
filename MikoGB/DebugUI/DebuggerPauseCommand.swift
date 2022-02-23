//
//  DebuggerPauseCommand.swift
//  MikoGB
//
//  Created on 7/22/21.
//

import Foundation

class DebuggerPauseCommand : DebuggerCommand {
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func respondsToInput(_ input: [String]) -> Bool {
        guard let first = input.first else {
            return false
        }
        
        if first == "pause" {
            return true
        }
        return false
    }
    
    func runCommand(input: [String], outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        guard input.count == 1 else {
            outputHandler("Command \'\(input[0])\' expects no arguments")
            completion()
            return
        }
        
        self.engine.setDesiredRunnable(false) {
            completion()
        }
    }
}
