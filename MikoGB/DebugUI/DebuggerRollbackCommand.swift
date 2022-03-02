//
//  DebuggerRollbackCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 3/1/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerRollbackCommand : DebuggerCommand {
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func respondsToInput(_ input: [String]) -> Bool {
        guard let first = input.first else {
            return false
        }
        
        if first == "rollback" {
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
        
        let descriptions = engine.lastExecutedInstructions()
        var i = 0
        while i < descriptions.count {
            let str = descriptions[i] + " " + descriptions[i+1]
            outputHandler(str)
            i += 2
        }
        completion()
    }
    
}
