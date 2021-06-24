//
//  DebuggerWindowController.swift
//  MikoGB
//
//  Created on 6/22/21.
//

import Cocoa

class DebuggerWindowController: NSWindowController {
    
    @IBOutlet var textEntryField: NSTextField!
    @IBOutlet var consoleOutputView: NSTextView!
    @IBOutlet var consoleScrollView: NSScrollView!
    
    private let _textController = DebuggerConsoleController()
    
    override func windowDidLoad() {
        super.windowDidLoad()
        
        self.window?.contentMinSize = NSSize(width: 575, height: 250)
        
        _textController.textView = consoleOutputView
        _textController.append("Enter 'help' to see available commands", style: .Prompt)
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
    
    @objc func a_debuggerEnter(_ sender: AnyObject) {
        let str = textEntryField.stringValue
        if str.isEmpty {
            NSSound.beep()
        } else {
            _textController.append(str, style: .UserInput)
            textEntryField.stringValue = ""
            
            // Defer scroll until after layout
            DispatchQueue.main.async {
                self.scrollConsoleToBottom()
            }
        }
    }
    
}
