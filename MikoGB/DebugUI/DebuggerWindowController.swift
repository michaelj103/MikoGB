//
//  DebuggerWindowController.swift
//  MikoGB
//
//  Created on 6/22/21.
//

import Cocoa

class DebuggerWindowController: NSWindowController, NSTableViewDataSource {
    
    @IBOutlet var textEntryField: NSTextField!
    @IBOutlet var consoleOutputView: NSTextView!
    @IBOutlet var consoleScrollView: NSScrollView!
    @IBOutlet var instructionTableView: NSTableView!
    
    private let _instructionController = DebuggerInstructionController()
    private let _textController = DebuggerConsoleController()
    private let _commandParser = CommandParser()
    private var commands = [String:DebuggerCommand]()
    private var runningCommand: Bool = false {
        didSet {
            textEntryField.isEnabled = !runningCommand
        }
    }
    
    override func windowDidLoad() {
        super.windowDidLoad()
        
        self.window?.contentMinSize = NSSize(width: 700, height: 500)
        self.window?.setContentSize(NSSize(width: 700, height: 500))
        
        _textController.textView = consoleOutputView
        _textController.append("Enter 'help' to see available commands", style: .Prompt)
        
        let engine = AppStateManager.sharedInstance.engine
        let pauseCommand = DebuggerPauseCommand(engine)
        let continueCommand = DebuggerContinueCommand(engine)
        let stepCommand = DebuggerStepCommand(engine)
        commands = [
            pauseCommand.commandName : pauseCommand,
            continueCommand.commandName : continueCommand,
            stepCommand.commandName : stepCommand,
        ]
        
        for (name, subCommand) in commands {
            let parserCommand = _commandParser.rootCommand.registerSubcommand(name)
            subCommand.configureSubcommand(command: parserCommand)
        }
        
        _instructionController.engine = engine
        _instructionController.tableView = instructionTableView
    }
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        return true
    }
    
    private func scrollConsoleToBottom() {
        let newScrollOrigin: NSPoint
        if consoleScrollView.isFlipped {
            newScrollOrigin = NSPoint(x: 0, y: NSMaxY(consoleScrollView.documentView?.frame ?? NSZeroRect) - NSHeight(consoleScrollView.contentView.bounds))
        } else {
            newScrollOrigin = NSPoint(x: 0, y: 0)
        }
        consoleScrollView.documentView?.scroll(newScrollOrigin)
    }
    
    private func _runCommand(_ command: DebuggerCommand, parsedResult: CommandResult) {
        command.runCommand(input: parsedResult) { str in
            self._appendConsoleText(str, style: .Output)
        } _: {
            self.runningCommand = false
        }
    }
    
    private func _runCommandString(_ str: String) {
        runningCommand = true
        let args = CommandParser.tokenize(str)
        let result: CommandResult?
        do {
            let baseResult = try _commandParser.parseArguments(args)
            let results = baseResult.subcommandResults
            if results.count == 1 {
                result = results.first
            } else {
                result = nil
                // TODO: configuration option on the parser?
                _appendConsoleText("Chained commands are unsupported", style: .Output)
            }
        } catch let error as SimpleError {
            result = nil
            _appendConsoleText(error.description, style: .Output)
        } catch {
            result = nil
            print(error)
            _appendConsoleText("Unrecognized error", style: .Output)
        }
        
        if let r = result {
            let name = r.command.name!
            if let cmd = commands[name] {
                _runCommand(cmd, parsedResult: r)
            } else {
                _appendConsoleText("No runnable command found", style: .Output)
                runningCommand = false
            }
        } else {
            runningCommand = false
        }
    }
    
    private func _appendConsoleText(_ str: String, style: DebuggerConsoleStyle) {
        _textController.append(str, style: style)
        
        // Defer scroll until after layout
        DispatchQueue.main.async {
            self.scrollConsoleToBottom()
        }
    }
    
    @objc func a_debuggerEnter(_ sender: AnyObject) {
        if runningCommand {
            NSSound.beep()
            return
        }
        
        let str = textEntryField.stringValue
        if str.isEmpty {
            NSSound.beep()
        } else {
            _appendConsoleText(str, style: .UserInput)
            textEntryField.stringValue = ""
            _runCommandString(str)
        }
    }
}
