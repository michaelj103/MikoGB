//
//  DebuggerMemCommand.swift
//  MikoGB
//
//  Created by Michael Brandt on 2/22/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class DebuggerMemCommand : DebuggerCommand {
    let engine: GBEngine
    
    init(_ engine: GBEngine) {
        self.engine = engine
    }
    
    func respondsToInput(_ input: [String]) -> Bool {
        guard let first = input.first else {
            return false
        }
        
        if first == "mem" {
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
    
    func runCommand(input: [String], outputHandler: @escaping (String) -> (), _ completion: @escaping () -> ()) {
        guard input.count == 2 else {
            outputHandler("Command \'mem\' expects an address argument")
            completion()
            return
        }
        
        let addr = input[1]
        var addressValue: UInt16?
        do {
            addressValue = try _parseAddress(addr)
        } catch let error as SimpleError {
            outputHandler(error.description)
        } catch {
            outputHandler("Unknown error: \(error)")
        }
        
        if let addressValue = addressValue {
            let byte = engine.readByte(addressValue)
            let output = _byteString(byte)
            outputHandler(output)
        }
        completion()
    }
    
}
