//
//  DebuggerWindowController.swift
//  MikoGB
//
//  Created on 6/22/21.
//

import Cocoa

class DebuggerWindowController: NSWindowController, NSTableViewDataSource, GBEngineObserver {
    
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
    
    private var isEngineRunnable = false
    private var engine: GBEngine? {
        willSet {
            engine?.unregisterObserver(self)
        }
        didSet {
            engine?.register(self)
            isEngineRunnable = engine?.isRunnable ?? false
            _updateStatusLabels()
        }
    }
    
    override func windowDidLoad() {
        super.windowDidLoad()
        
        self.window?.contentMinSize = NSSize(width: 700, height: 500)
        self.window?.setContentSize(NSSize(width: 700, height: 500))
        
        _textController.textView = consoleOutputView
        _textController.append("Enter 'help' to see available commands", style: .Prompt)
        
        let engine = AppStateManager.sharedInstance.engine
        self.engine = engine
        let pauseCommand = DebuggerPauseCommand(engine)
        let continueCommand = DebuggerContinueCommand(engine)
        let stepCommand = DebuggerStepCommand(engine)
        let memCommand = DebuggerMemCommand(engine)
        commands = [
            pauseCommand.commandName : pauseCommand,
            continueCommand.commandName : continueCommand,
            stepCommand.commandName : stepCommand,
            memCommand.commandName : memCommand,
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
    
    func engine(_ engine: GBEngine, runnableDidChange isRunnable: Bool) {
        isEngineRunnable = isRunnable
        _updateStatusLabels()
    }
    
    func didUpdateSuspendedState(for engine: GBEngine) {
        _updateStatusLabels()
    }
    
    private func _updateStatusLabels() {
        _updateRegisterLabels()
    }
    
    private func _registerString(_ val: UInt8) -> String {
        return NSString(format: "0x%02X", val) as String
    }
    
    @IBOutlet var aRegLabel: NSTextField!
    @IBOutlet var bRegLabel: NSTextField!
    @IBOutlet var cRegLabel: NSTextField!
    @IBOutlet var dRegLabel: NSTextField!
    @IBOutlet var eRegLabel: NSTextField!
    @IBOutlet var hRegLabel: NSTextField!
    @IBOutlet var lRegLabel: NSTextField!
    @IBOutlet var zFlagLabel: NSTextField!
    @IBOutlet var nFlagLabel: NSTextField!
    @IBOutlet var hFlagLabel: NSTextField!
    @IBOutlet var cFlagLabel: NSTextField!
    private func _updateRegisterLabels() {
        guard !isEngineRunnable, let regState = engine?.registerState() else {
            aRegLabel.stringValue = "--"
            bRegLabel.stringValue = "--"
            cRegLabel.stringValue = "--"
            dRegLabel.stringValue = "--"
            eRegLabel.stringValue = "--"
            hRegLabel.stringValue = "--"
            lRegLabel.stringValue = "--"
            zFlagLabel.stringValue = "-"
            nFlagLabel.stringValue = "-"
            hFlagLabel.stringValue = "-"
            cFlagLabel.stringValue = "-"
            return
        }
        
        aRegLabel.stringValue = _registerString(regState.A)
        bRegLabel.stringValue = _registerString(regState.B)
        cRegLabel.stringValue = _registerString(regState.C)
        dRegLabel.stringValue = _registerString(regState.D)
        eRegLabel.stringValue = _registerString(regState.E)
        hRegLabel.stringValue = _registerString(regState.H)
        lRegLabel.stringValue = _registerString(regState.L)
        zFlagLabel.stringValue = regState.ZFlag.boolValue ? "1" : "0"
        nFlagLabel.stringValue = regState.NFlag.boolValue ? "1" : "0"
        hFlagLabel.stringValue = regState.HFlag.boolValue ? "1" : "0"
        cFlagLabel.stringValue = regState.CFlag.boolValue ? "1" : "0"
    }
}
