//
//  LinkManagerWindowController.swift
//  MikoGB
//
//  Created by Michael Brandt on 8/17/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Cocoa

class LinkManagerWindowController: NSWindowController {
    @IBOutlet var serverLabel: NSTextField!
    @IBOutlet var loginButton: NSButton!
    @IBOutlet var loadingIndicator: NSProgressIndicator!
    @IBOutlet var statusIcon: NSImageView!
    
    @IBOutlet var roomActionButton: NSButton!
    @IBOutlet var roomCodeLabel: NSTextField!
    
    private var linkSessionObserver: NSObjectProtocol?
    
    private enum LoginStatus {
        case notLoggedIn
        case loggingIn
        case loggedIn(String)
        case loginFailed
    }
    private var loginStatus: LoginStatus = .notLoggedIn {
        didSet {
            _updateLoginButton()
            _updateLoadingStatus()
            _updateRoomStatusUI()
        }
    }
    
    override func windowDidLoad() {
        super.windowDidLoad()
        serverLabel.stringValue = ServerConfiguration.Hostname
        _updateLoginButton()
        _updateLoadingStatus()
        _updateRoomStatusUI()
        
        linkSessionObserver = NotificationCenter.default.addObserver(forName: LinkSessionManager.RoomStatusChangedNotification, object: nil, queue: nil, using: { [weak self] _ in
            DispatchQueue.main.async {
                self?._updateRoomStatusUI()
            }
        })
    }
    
    private func _updateLoginButton() {
        switch loginStatus {
        case .notLoggedIn, .loginFailed:
            self.loginButton.isEnabled = true
        case .loggedIn(_), .loggingIn:
            self.loginButton.isEnabled = false
        }
    }
    
    private func _updateLoadingStatus() {
        let wantsLoadingIndicator: Bool
        let statusImage: NSImage?
        switch loginStatus {
        case .notLoggedIn:
            wantsLoadingIndicator = false
            statusImage = NSImage(systemSymbolName: "exclamationmark.triangle", accessibilityDescription: nil)?.withSymbolConfiguration(NSImage.SymbolConfiguration(hierarchicalColor: .yellow))
        case .loggingIn:
            wantsLoadingIndicator = true
            statusImage = nil
        case .loggedIn(_):
            wantsLoadingIndicator = false
            statusImage = NSImage(systemSymbolName: "checkmark.square", accessibilityDescription: nil)?.withSymbolConfiguration(NSImage.SymbolConfiguration(hierarchicalColor: .green))
        case .loginFailed:
            wantsLoadingIndicator = false
            statusImage = NSImage(systemSymbolName: "xmark.diamond", accessibilityDescription: nil)?.withSymbolConfiguration(NSImage.SymbolConfiguration(hierarchicalColor: .red))
        }
        
        if wantsLoadingIndicator {
            loadingIndicator.isHidden = false
            loadingIndicator.startAnimation(nil)
            statusIcon.isHidden = true
        } else {
            loadingIndicator.isHidden = true
            loadingIndicator.stopAnimation(nil)
            statusIcon.image = statusImage
            statusIcon.isHidden = false
        }
    }
    
    private func _updateRoomStatusUI() {
        guard case .loggedIn = loginStatus else {
            roomActionButton.title = "Check"
            roomActionButton.isEnabled = false
            roomCodeLabel.stringValue = "-none-"
            return
        }
        
        // we're logged in, so reflect the link session manager's status
        let linkSessionManager = AppStateManager.sharedInstance.linkSessionManager
        let roomStatus = linkSessionManager.roomStatus
        let isWorking = linkSessionManager.isWorking
        switch roomStatus {
        case .notChecked:
            roomActionButton.title = "Check"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "Unknown"
        case .noRooms:
            roomActionButton.title = "Create"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "No Rooms"
        case .roomAvailable(let linkRoomClientInfo):
            roomActionButton.title = "Connect"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "\(linkRoomClientInfo.roomCode)"
        case .connectedToRoom(let linkRoomClientInfo):
            roomActionButton.title = "Close"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "\(linkRoomClientInfo.roomCode)"
        case .error:
            roomActionButton.title = "Check"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "error"
        case .disconnected:
            roomActionButton.title = "Check"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "disconnected"
        }
    }
    
    private func _handleUpdatedServer(_ response: NSApplication.ModalResponse, textField: NSTextField) {
        let oldValue = serverLabel.stringValue
        if response == .alertFirstButtonReturn {
            let newServer = textField.stringValue
            if ServerConfiguration.setTemporaryHost(newServer) {
                serverLabel.stringValue = newServer
            }
        } else if response == .alertSecondButtonReturn {
            let newServer = ServerConfiguration.Hostname
            ServerConfiguration.stopTemporaryOverride()
            serverLabel.stringValue = newServer
        }
        if serverLabel.stringValue != oldValue {
            loginStatus = .notLoggedIn
        }
    }
    
    private func _handleLoginResult(_ result: Result<String, Error>) {
        switch result {
        case .success(let deviceID):
            loginStatus = .loggedIn(deviceID)
        case .failure(let error):
            print("Log in failed with error \(error)")
            loginStatus = .loginFailed
        }
    }
    
    @objc @IBAction
    private func _handleChangeServerButton(_ sender: AnyObject) {
        guard let window = self.window else {
            preconditionFailure("No window? No sense.")
        }
        let textField = NSTextField(frame: NSMakeRect(0, 0, 200, 24))
        textField.stringValue = "www.example.com"
        textField.sizeToFit()
        let alert = NSAlert()
        alert.messageText = "Message text"
        alert.informativeText = "Informative text"
        alert.accessoryView = textField
        alert.addButton(withTitle: "OK")
        alert.addButton(withTitle: "Clear")
        alert.addButton(withTitle: "Cancel")
        alert.beginSheetModal(for: window) { response in
            DispatchQueue.main.async {
                self._handleUpdatedServer(response, textField: textField)
            }
        }
    }
    
    @objc @IBAction
    private func _logIn(_ sender: AnyObject) {
        loginStatus = .loggingIn
        UserIdentityController.sharedIdentityController.ensureRegistration(force: true) { [weak self] result in
            self?._handleLoginResult(result)
        }
    }
    
    @objc @IBAction
    private func _handleRoomActionButton(_ sender: AnyObject) {
        let linkSessionManager = AppStateManager.sharedInstance.linkSessionManager
        let roomStatus = linkSessionManager.roomStatus
        switch roomStatus {
        case .notChecked, .error, .disconnected:
            linkSessionManager.checkForRooms()
            _updateRoomStatusUI()
            break
        case .noRooms:
            // TODO: Create/Join
            break
        case .roomAvailable(_):
            // TODO: Connect
            break
        case .connectedToRoom(_):
            // TODO: Close
            break
        }
    }
}

