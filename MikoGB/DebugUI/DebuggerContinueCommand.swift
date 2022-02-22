//
//  DebuggerContinueCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/21/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerContinueCommand : DebuggerCommand {
    let commandName = "continue"
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func configureSubcommand(command: Command) {
        
    }
    
    func runCommand(input: CommandResult, outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        self.engine.setDesiredRunnable(true) {
            completion()
        }
    }
    
}
