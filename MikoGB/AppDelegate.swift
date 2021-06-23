//
//  AppDelegate.swift
//  MikoGB
//
//  Created on 5/4/21.
//

import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        return true
    }
    
    @objc func a_showDebugger(_ sender: AnyObject) {
        let windowController = AppStateManager.sharedInstance.debuggerWindowController
        windowController.showWindow(self)
    }

}

