//
//  AppStateManager.swift
//  MikoGB
//
//  Created on 6/22/21.
//

import Cocoa

class AppStateManager {
    static let sharedInstance = AppStateManager()
    
    let engine = GBEngine()
    
    private var _debuggerWindowController: DebuggerWindowController?
    var debuggerWindowController: DebuggerWindowController {
        get {
            if let wc = _debuggerWindowController {
                return wc
            } else {
                let wc = DebuggerWindowController(windowNibName: "Debugger")
                _debuggerWindowController = wc
                return wc
            }
        }
    }
}
