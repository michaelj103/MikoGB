//
//  LinkManagerWindowController.swift
//  MikoGB
//
//  Created by Michael Brandt on 8/17/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Cocoa

class LinkManagerWindowController: NSWindowController, NSTextFieldDelegate {
    @IBOutlet var serverLabel: NSTextField!
    @IBOutlet var loginButton: NSButton!
    @IBOutlet var loadingIndicator: NSProgressIndicator!
    @IBOutlet var statusIcon: NSImageView!
    
    @IBOutlet var roomActionButton: NSButton!
    @IBOutlet var roomCodeLabel: NSTextField!
    @IBOutlet var roomJoinButton: NSButton!
    
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
        roomCodeLabel.isSelectable = true
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
            roomJoinButton.isEnabled = false
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
            roomJoinButton.isEnabled = false
        case .noRooms:
            roomActionButton.title = "Create"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "No Rooms"
            roomJoinButton.isEnabled = !isWorking
        case .roomAvailable(let linkRoomClientInfo):
            roomActionButton.title = "Connect"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "\(linkRoomClientInfo.roomCode)"
            roomJoinButton.isEnabled = false
        case .connectingToRoom(let linkRoomClientInfo):
            roomActionButton.title = "Connect"
            roomActionButton.isEnabled = false
            roomCodeLabel.stringValue = "\(linkRoomClientInfo.roomCode)"
            roomJoinButton.isEnabled = false
        case .connectedToRoom(let linkRoomClientInfo):
            roomActionButton.title = "Close"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "\(linkRoomClientInfo.roomCode)"
            roomJoinButton.isEnabled = false
        case .error:
            roomActionButton.title = "Check"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "error"
            roomJoinButton.isEnabled = false
        case .disconnected:
            roomActionButton.title = "Check"
            roomActionButton.isEnabled = !isWorking
            roomCodeLabel.stringValue = "disconnected"
            roomJoinButton.isEnabled = false
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
        textField.placeholderString = "www.example.com"
        textField.sizeToFit()
        let alert = NSAlert()
        alert.messageText = "Enter a Server"
        alert.informativeText = "Enter a valid server URL for interacting with the link room system"
        alert.accessoryView = textField
        let okButton = alert.addButton(withTitle: "OK")
        okButton.isEnabled = false
        alert.addButton(withTitle: "Clear")
        alert.addButton(withTitle: "Cancel")
        
        textField.delegate = self
        activeTextField = textField
        activeTextFieldButton = okButton
        alert.beginSheetModal(for: window) { response in
            DispatchQueue.main.async {
                self.activeTextField = nil
                self.activeTextFieldButton = nil
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
        case .noRooms:
            linkSessionManager.createRoom()
            _updateRoomStatusUI()
        case .roomAvailable:
            linkSessionManager.connectToRoom()
            _updateRoomStatusUI()
        case .connectingToRoom:
            // Nothing to do
            break
        case .connectedToRoom:
            // TODO: Close
            break
        }
    }
    
    private func _handleJoinCode(_ response: NSApplication.ModalResponse, textField: NSTextField) {
        if response == .alertFirstButtonReturn {
            let joinCode = textField.stringValue
            let linkSessionManager = AppStateManager.sharedInstance.linkSessionManager
            linkSessionManager.joinRoom(joinCode)
            _updateRoomStatusUI()
        }
    }
    
    @objc @IBAction
    private func _handleRoomJoinButton(_ sender: AnyObject) {
        guard let window = self.window else {
            preconditionFailure("No window? No sense.")
        }
        let textField = NSTextField(frame: NSMakeRect(0, 0, 200, 24))
        textField.placeholderString = "AAAAAA"
        textField.sizeToFit()
        let alert = NSAlert()
        alert.messageText = "Enter a Room Code"
        alert.informativeText = "Get the 6-character alphanumeric code for joining a room from the room's creator"
        alert.accessoryView = textField
        let okButton = alert.addButton(withTitle: "OK")
        okButton.isEnabled = false
        alert.addButton(withTitle: "Cancel")
        
        textField.delegate = self
        activeTextField = textField
        activeTextFieldButton = okButton
        alert.beginSheetModal(for: window) { response in
            DispatchQueue.main.async {
                self.activeTextField = nil
                self.activeTextFieldButton = nil
                self._handleJoinCode(response, textField: textField)
            }
        }
    }
    
    // MARK: - NSTextFieldDelegate
    
    private var activeTextFieldButton: NSButton?
    private var activeTextField: NSTextField?
    func controlTextDidChange(_ obj: Notification) {
        activeTextFieldButton?.isEnabled = (activeTextField?.stringValue.count ?? 0) > 0
    }
}

