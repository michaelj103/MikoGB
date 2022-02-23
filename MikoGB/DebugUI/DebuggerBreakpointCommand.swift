//
//  DebuggerBreakpointCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/23/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerBreakpointCommand : DebuggerCommand {
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func respondsToInput(_ input: [String]) -> Bool {
        guard let first = input.first else {
            return false
        }
        
        if first == "breakpoint" || first == "bp" {
            return true
        }
        return false
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
    
    private func _byteString(_ byte: UInt8) -> String {
        let hexStr = "0123456789ABCDEF"
        let highNib = Int((byte & 0xF0) >> 4)
        let lowNib = Int(byte & 0x0F)
        
        let c1 = hexStr[hexStr.index(hexStr.startIndex, offsetBy: highNib)]
        let c2 = hexStr[hexStr.index(hexStr.startIndex, offsetBy: lowNib)]
        let output = String(c1) + String(c2)
        return output
    }
    
    private func _runSetCommand(input: [String], outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        guard input.count == 4 else {
            outputHandler("To set a breakpoint provide a ROM bank and address")
            completion()
            return
        }
        
        guard let bank = Int32(input[2]) else {
            outputHandler("ROM bank should be specified as a decimal integer")
            completion()
            return
        }
        
        let addr = input[3]
        var addressValue: UInt16?
        do {
            addressValue = try _parseAddress(addr)
        } catch let error as SimpleError {
            outputHandler(error.description)
        } catch {
            outputHandler("Unknown error: \(error)")
        }
        
        guard let address = addressValue else {
            completion()
            return
        }
        
        if !engine.addLineBreakpoint(forBank: bank, address: address) {
            outputHandler("Breakpoints are not enabled in this build")
        }
        completion()
    }
    
    func runCommand(input: [String], outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        guard input.count >= 2 else {
            outputHandler("Command \'\(input[0])\' expects a subcommand")
            completion()
            return
        }
        
        if input[1] == "set" {
            _runSetCommand(input: input, outputHandler: outputHandler, completion)
        } else {
            outputHandler("Unrecognized subcommand")
            completion()
        }
    }
    
}
