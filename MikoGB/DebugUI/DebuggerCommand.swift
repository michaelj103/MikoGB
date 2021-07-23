//
//  DebuggerCommand.swift
//  MikoGB
//
//  Created on 7/22/21.
//

import Foundation

protocol DebuggerCommand {
    var commandName: String { get }
        
    /// The concrete command is responsible for adding its configuration as a subcommand for parsing
    func configureSubcommand(command: Command)
    
    /// The concrete command will be given a parsed command result and asked to run
    /// Call the outputHandler with any string that should be displayed in the console. Call completion when the command completes
    func runCommand(input: CommandResult, outputHandler: @escaping (String)->(), _ completion: @escaping ()->())
}
