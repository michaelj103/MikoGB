//
//  DebuggerStepCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/21/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerStepCommand : DebuggerCommand {
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func respondsToInput(_ input: [String]) -> Bool {
        guard let first = input.first else {
            return false
        }
        if first == "step" || first == "s" {
            return true
        }
        return false
    }
    
    func runCommand(input: [String], outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        guard input.count <= 2 else {
            outputHandler("Command \'step\' expects up to 1 argument")
            completion()
            return
        }
        
        var count: Int?
        if input.count == 2 {
            if let x = Int(input[1]), x > 0 {
                count = x
            } else {
                outputHandler("Expects an integer number of steps greater than 0")
            }
        } else {
            count = 1
        }
        
        if let steps = count {
            self.engine.step(steps)
        }
        completion()
    }
}
