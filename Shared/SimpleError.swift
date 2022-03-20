//
//  SimpleError.swift
//  btutil
//
//  Created on 3/14/21.
//

public struct SimpleError : Error, CustomStringConvertible {
    public let description: String
    public init(_ desc: String) {
        description = desc
    }
}
