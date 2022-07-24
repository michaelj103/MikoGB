//
//  DebuggerStackCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 4/13/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerStackCommand : DebuggerCommand {
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func respondsToInput(_ input: [String]) -> Bool {
        guard let first = input.first else {
            return false
        }
        
        if first == "stack" {
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
        
        let description = engine.stackDescription()
        outputHandler(description)
        completion()
    }
}
