//
//  DebuggerStepCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/21/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerStepCommand : DebuggerCommand {
    let commandName = "step"
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func configureSubcommand(command: Command) {
        command.registerOption(.integer(nil), shortName: "c", longName: nil, description: nil)
    }
    
    func runCommand(input: CommandResult, outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        let result = input.getResult("c")
        let steps: Int
        switch result {
        case .integer(let s):
            steps = s ?? 1
        case .boolean(_):
            fallthrough
        case .float(_):
            fallthrough
        case .string(_):
            steps = 1
        }
        self.engine.step(steps)
        completion()
    }
    
}
