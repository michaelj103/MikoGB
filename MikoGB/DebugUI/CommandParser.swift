//
//  CommandParser.swift
//  btutil
//
//  Created on 3/14/21.
//

/// Possible types of options. The associated value has contextual meaning. When registering an option, it defines the default value. When inspecting a result, it's the resulting value
public enum CommandOptionType {
    /// If the option is present, the value is the inverse of the default (allows simple implementation of options to disable something that is enabled by default)
    case boolean(Bool)
    /// requires a value that parses as an integer
    case integer(Int?)
    /// requires a value that parses as a Double
    case float(Double?)
    /// requires a value, but it can be basically anything
    case string(String?)
    
    public func getBool() -> Bool {
        switch self {
        case .boolean(let b):
            return b
        default:
            preconditionFailure("getBool() is invalid for non-bool type")
        }
    }
    
    public func getInt() -> Int? {
        switch self {
        case .integer(let x):
            return x
        default:
            preconditionFailure("getInt() is invalid for non-bool type")
        }
    }
    
    public func getDouble() -> Double? {
        return getFloat()
    }
    
    public func getFloat() -> Double? {
        switch self {
        case .float(let x):
            return x
        default:
            preconditionFailure("getFloat() is invalid for non-bool type")
        }
    }
    
    public func getString() -> String? {
        switch self {
        case .string(let s):
            return s
        default:
            preconditionFailure("getString() is invalid for non-bool type")
        }
    }
}

fileprivate struct CommandOption {
    let type: CommandOptionType
    let shortName: String
    let longName: String?
    let description: String?
    
    var nameDescription: String {
        var str = ""
        if shortName.count == 1 {
            str += "-\(shortName)"
        } else {
            str += "--\(shortName)"
        }
        if let long = longName {
            str += "/--\(long)"
        }
        return str
    }
}

public class CommandResult {
    /// Only contains subcommand results for subcommands that were present
    public private(set) var subcommandResults = [CommandResult]()
    public let command: Command
    
    private let shortNamesByLongName: [String:String]
    private var resultsByShortName = [String:CommandOptionType]()
    
    fileprivate init(_ command: Command, _ shortNamesByLongName: [String:String]) {
        self.command = command
        self.shortNamesByLongName = shortNamesByLongName
    }
    
    fileprivate func addSubcommandResult(_ result: CommandResult) {
        subcommandResults.append(result)
    }
    
    fileprivate func addResult(_ name: String, _ val: CommandOptionType) {
        resultsByShortName[name] = val
    }
    
    public func getResult(_ name: String) -> CommandOptionType {
        let nameToLookup: String
        if let shortName = shortNamesByLongName[name] {
            nameToLookup = shortName
        } else {
            nameToLookup = name
        }
        
        guard let result = resultsByShortName[nameToLookup] else {
            preconditionFailure("Result requested for unregistered name \(name)")
        }
        return result
    }
}

public class Command {
    public let name: String?
    private var subcommandsByName = [String:Command]()
    private var optionsByShortName = [String:CommandOption]()
    private var shortNamesByLongName = [String:String]()
    
    fileprivate init(_ name: String?) {
        self.name = name
    }
    
    public func registerSubcommand(_ name: String) -> Command {
        precondition(name.count > 0, "An empty string cannot be registered as a subcommand")
        precondition(subcommandsByName[name] == nil, "Multiple subcommands with the same name (\"\(name)\") is not supported")
        let cmd = Command(name)
        subcommandsByName[name] = cmd
        return cmd
    }
    
    public func registerOption(_ type: CommandOptionType, shortName: String, longName: String? = nil, description: String? = nil) {
        let option = CommandOption(type: type, shortName: shortName, longName: longName, description: description)
        precondition(shortName.count > 0 && (longName == nil || longName!.count > 0), "An empty string cannot be registered as an option")
        precondition(optionsByShortName[shortName] == nil && shortNamesByLongName[shortName] == nil, "Option named \"\(shortName)\" is already taken. Explicitly unregister it before replacing")
        optionsByShortName[shortName] = option
        
        if let long = longName {
            precondition(optionsByShortName[long] == nil && shortNamesByLongName[long] == nil, "Option named \"\(long)\" is already taken. Explicitly unregister it before replacing")
            precondition(long.count > 1, "Single character long options like \(long) don't make sense. If you want multiple single characters to mean the same thing, consider registering multiple short options")
            shortNamesByLongName[long] = shortName
        }
    }
    
    public func unregisterOption(_ name: String) {
        let shortName: String
        let isLong: Bool
        if let short = shortNamesByLongName[name] {
            shortName = short
            isLong = true
        } else {
            shortName = name
            isLong = false
        }
        
        if isLong {
            shortNamesByLongName.removeValue(forKey: name)
        }
        optionsByShortName.removeValue(forKey: shortName)
    }
    
    //MARK: - Parsing
    
    // Creates a result for this command with default values
    private func _createDefaultResult() -> CommandResult {
        let result =  CommandResult(self, shortNamesByLongName)
        for (shortName, opt) in optionsByShortName {
            result.addResult(shortName, opt.type)
        }
        return result
    }
    
    private func _findOption(_ name: String, canBeLong: Bool) -> CommandOption? {
        let shortName: String
        if canBeLong, let short = shortNamesByLongName[name] {
            shortName = short
        } else {
            shortName = name
        }
        
        return optionsByShortName[shortName]
    }
    
    fileprivate func parse(_ arguments: [String], isRoot: Bool, startIndex: Int) throws -> (CommandResult, Int) {
        let result = _createDefaultResult()
        
        var argIndex = startIndex
        // Helper function that adds an option result to the current result and handles error cases. Useful for avoiding code repetition
        // to understand the parse function, it's probably to come back to this helper after understanding the main loop
        func handleOption(_ opt: CommandOption, requireBool: Bool) throws {
            
            // helper function that adds a result for options that take arguments and handles error cases
            func parseArgWithCreator(_ creator: (String)->CommandOptionType?) throws {
                argIndex += 1
                if argIndex >= arguments.count {
                    throw SimpleError("Option \"\(opt.nameDescription)\" requires an argument")
                }
                if let val = creator(arguments[argIndex]) {
                    result.addResult(opt.shortName, val)
                } else {
                    throw SimpleError("Invalid argument for option \"\(opt.nameDescription)\": \"\(arguments[argIndex])\"")
                }
            }
            
            func boolFail() throws {
                if requireBool {
                    throw SimpleError("Option -\(opt.shortName) must be the last option in a grouping of multiple single-character options")
                }
            }
            
            switch opt.type {
            case .boolean(let defaultVal):
                result.addResult(opt.shortName, .boolean(!defaultVal))
            case .integer(_):
                try boolFail()
                try parseArgWithCreator({ (str) -> CommandOptionType? in
                    if let x = Int(str) {
                        return .integer(x)
                    } else {
                        return nil
                    }
                })
            case .float(_):
                try boolFail()
                try parseArgWithCreator({ (str) -> CommandOptionType? in
                    if let x = Double(str) {
                        return .float(x)
                    } else {
                        return nil
                    }
                })
            case .string(_):
                try boolFail()
                try parseArgWithCreator({ .string($0) })
            }
        }
        
        while argIndex < arguments.count {
            let arg = arguments[argIndex]
            if arg.hasPrefix("--") { // Long option or short that is multiple characters
                let firstIdx = arg.firstIndex { $0 != "-" }
                let optionName = String(arg.suffix(from: firstIdx!))
                if let option = _findOption(optionName, canBeLong: true) {
                    try handleOption(option, requireBool: false)
                } else {
                    //Unrecognized option. If we're root, this is an error. If not, return parsing control to parent.
                    if isRoot {
                        throw SimpleError("Unrecognized option \"\(arg)\"")
                    } else {
                        break
                    }
                }
                argIndex += 1
            } else if arg.hasPrefix("-") { // Single character short option(s)
                let firstIdx = arg.firstIndex { $0 != "-" }
                let optionChars = Array(arg.suffix(from: firstIdx!))
                // collect all chained options before evaluating any because if one is unrecognized we may want to fall back to the parent
                // and in that case, we would have to roll back any evaluated options
                var optsToHandle = [CommandOption]()
                for ch in optionChars {
                    let optAsStr = String(ch)
                    if let option = _findOption(optAsStr, canBeLong: false) {
                        optsToHandle.append(option)
                    } else {
                        break
                    }
                }
                if optsToHandle.count == optionChars.count {
                    // all recognized for this command. Evaluate them
                    for i in 0..<optsToHandle.count {
                        let requireBool = i != optsToHandle.count - 1 // all but the last must be a bool option
                        try handleOption(optsToHandle[i], requireBool: requireBool)
                    }
                } else {
                    // at least one option was not recognized. If we're root, this is an error. If not, return parsing control to parent.
                    if isRoot {
                        throw SimpleError("Unrecognized option \"\(arg)\"")
                    } else {
                        break
                    }
                }
                argIndex += 1
            } else { // Subcommand
                if let subcommand = subcommandsByName[arg] {
                    let (subResult, nextIndex) = try subcommand.parse(arguments, isRoot: false, startIndex: argIndex + 1)
                    result.addSubcommandResult(subResult)
                    argIndex = nextIndex
                } else {
                    // Unrecognized subcommand. If we're root, this is an error. If not, return parsing control to parent.
                    if isRoot {
                        throw SimpleError("Unrecognized subcommand \"\(arg)\"")
                    } else {
                        break
                    }
                }
            }
        }
        
        return (result, argIndex)
    }
}

public class CommandParser {
    public let rootCommand: Command
    
    public init() {
        rootCommand = Command(nil)
    }
    
    public func parseArguments(_ arguments: [String]) throws -> CommandResult {
        let (result, idx) = try rootCommand.parse(arguments, isRoot: true, startIndex: 0)
        assert(idx == arguments.count, "Internal consistency assertion: did not parse arguments to the end")
        return result
    }
    
    // Obviously not a good or complete tokenizer
    // TODO: make it a good-ish and somewhat complete tokenizer
    public static func tokenize(_ string: String) -> [String] {
        let substrings = string.split(separator: " ")
        let strings = substrings.map { String($0) }
        return strings
    }
}
