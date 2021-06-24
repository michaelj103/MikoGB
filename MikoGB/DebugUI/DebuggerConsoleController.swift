//
//  DebuggerConsoleController.swift
//  MikoGB
//
//  Created on 6/23/21.
//

import Cocoa

enum DebuggerConsoleStyle {
    case Output
    case UserInput
    case Prompt
}

class DebuggerConsoleController {
    var textView: NSTextView?
    private let _defaultAttributes: [NSAttributedString.Key : Any]
    private let _inputAttributes: [NSAttributedString.Key : Any]
    private var needsNewline = false
    
    init() {
        var defaultAttrs = [NSAttributedString.Key : Any]()
        defaultAttrs[NSAttributedString.Key.font] = NSFont(name: "Helvetica", size: 12.0)
        defaultAttrs[NSAttributedString.Key.foregroundColor] = NSColor.textColor
        _defaultAttributes = defaultAttrs
        
        var inputAttrs = [NSAttributedString.Key : Any]()
        let descriptor = NSFontDescriptor(name: "Helvetica", size: 12.0).withSymbolicTraits([.bold])
        inputAttrs[NSAttributedString.Key.font] = NSFont(descriptor: descriptor, size: 0.0)
        inputAttrs[NSAttributedString.Key.foregroundColor] = NSColor.textColor
        _inputAttributes = inputAttrs
    }
    
    private func _modifiedString(_ str: String, style: DebuggerConsoleStyle) -> String {
        switch style {
        case .UserInput:
            return "> \(str)"
        default:
            return str
        }
    }
    
    private func _makeAttributedString(_ str: String, style: DebuggerConsoleStyle) -> NSAttributedString {
        let baseString = _modifiedString(str, style: style)
        let attributed: NSAttributedString
        switch style {
        case .Output, .Prompt:
            attributed = NSAttributedString(string: baseString, attributes: _defaultAttributes)            
        case .UserInput:
            attributed = NSAttributedString(string: baseString, attributes: _inputAttributes)
        }
        
        return attributed
    }
    
    func append(_ str: String, style: DebuggerConsoleStyle) {
        if needsNewline {
            let newline = NSAttributedString(string: "\n", attributes: _defaultAttributes)
            textView?.textStorage?.append(newline)
        }
        
        let attributed = _makeAttributedString(str, style: style)
        textView?.textStorage?.append(attributed)
        needsNewline = true
    }
}
