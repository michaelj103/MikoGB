//
//  DebuggerCommand.swift
//  MikoGB
//
//  Created on 7/22/21.
//

import Foundation

protocol DebuggerCommand {    
    func respondsToInput(_ input: [String]) -> Bool
        
    /// The concrete command will be given the command input and asked to run
    /// Call the outputHandler with any string that should be displayed in the console. Call completion when the command completes
    func runCommand(input: [String], outputHandler: @escaping (String)->(), _ completion: @escaping ()->())
}
