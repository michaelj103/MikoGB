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
    
    override func windowDidLoad() {
        super.windowDidLoad()

        // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
        consoleOutputView.string = "Enter 'help' to see available commands"
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
            print("Got \"\(str)\"")
            consoleOutputView.string.append("\n\(str)")
            textEntryField.stringValue = ""
            
            DispatchQueue.main.async {
                self.scrollConsoleToBottom()
            }
        }
    }
    
}
