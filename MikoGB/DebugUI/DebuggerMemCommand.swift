//
//  DebuggerMemCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/22/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerMemCommand : DebuggerCommand {
    let commandName = "mem"
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func configureSubcommand(command: Command) {
        command.registerOption(.string(nil), shortName: "a", longName: "addr", description: "address to read")
    }
    
    private func _parseAddress(_ str: String) throws -> UInt16 {
        if str.count > 4 {
            throw SimpleError("Addresses should be hex values from 0000 through FFFF")
        }
        var value: UInt16 = 0
        for c in str {
            value *= 16
            if let hexVal = c.hexDigitValue {
                value += UInt16(hexVal)
            } else {
                throw SimpleError("Invalid hex digit \"\(c)\"")
            }
        }
        return value
    }
    
    private func _printByte(_ byte: UInt8) {
        let hexStr = "0123456789ABCDEF"
        let highNib = Int((byte & 0xF0) >> 4)
        let lowNib = Int(byte & 0x0F)
        
        let c1 = hexStr[hexStr.index(hexStr.startIndex, offsetBy: highNib)]
        print(c1, terminator: "")
        let c2 = hexStr[hexStr.index(hexStr.startIndex, offsetBy: lowNib)]
        print(c2)
    }
    
    func runCommand(input: CommandResult, outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        let result = input.getResult("a")
        let addr: String
        switch result {
        case .string(let s):
            addr = s ?? ""
        case .boolean(_):
            fallthrough
        case .float(_):
            fallthrough
        case .integer(_):
            addr = ""
        }
        
        if addr.isEmpty {
            print("Must supply an address to read via -a")
            completion()
        } else {
            var addressValue: UInt16?
            do {
                addressValue = try _parseAddress(addr)
            } catch let error as SimpleError {
                print(error.description)
            } catch {
                print("Unknown error: \(error)")
            }
            
            if let addressValue = addressValue {
                let byte = engine.readByte(addressValue)
                _printByte(byte)
                completion()
            } else {
                completion()
            }
        }
    }
    
}