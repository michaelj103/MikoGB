//
//  AppDelegate.swift
//  MikoGB
//
//  Created on 5/4/21.
//

import Cocoa

let TerminationNotification = NSNotification.Name(rawValue: "MikoGBApplicationWillTerminate")

@main
class AppDelegate: NSObject, NSApplicationDelegate {

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        try! PersistenceManager.prepare()
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
        NotificationCenter.default.post(name: TerminationNotification, object: nil)
    }
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        return true
    }
    
    @objc func a_showDebugger(_ sender: AnyObject) {
        let windowController = AppStateManager.sharedInstance.debuggerWindowController
        windowController.showWindow(self)
    }

}

