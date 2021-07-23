//
//  DebuggerPauseCommand.swift
//  MikoGB
//
//  Created on 7/22/21.
//

import Foundation

class DebuggerPauseCommand : DebuggerCommand {
    let commandName = "pause"
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func configureSubcommand(command: Command) {
        
    }
    
    func runCommand(input: CommandResult, outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        self.engine.setDesiredRunnable(false) {
            completion()
        }
    }
    
}
