//
//  LinkSessionViewController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/24/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

class LinkSessionViewController: UIViewController {
    
    static func presentModal(with linkSessionManager: LinkSessionManager, on viewController: UIViewController) {
        let sessionVC = LinkSessionViewController(linkSessionManager: linkSessionManager)
        let navigationController = UINavigationController(rootViewController: sessionVC)
        if let sheet = navigationController.sheetPresentationController {
            sheet.detents = [.medium(), .large()]
            sheet.prefersScrollingExpandsWhenScrolledToEdge = false
            sheet.prefersEdgeAttachedInCompactHeight = true
            sheet.widthFollowsPreferredContentSizeWhenEdgeAttached = true
        }
        viewController.present(navigationController, animated: true)
    }
    
    private static let CreateSessionString = "Create"
    private static let JoinSessionString = "Join"
    private static let CheckForSessionsString = "Check"
    private static let ConnectToSessionString = "Connect"
    private static let LeaveSessionString = "Disconnect"
    private static let EndSessionString = "Close"
    
    let linkSessionManager:LinkSessionManager
    private var statusLabel: UILabel! = nil
    private var primaryButton: UIButton! = nil
    private var secondaryButton: UIButton! = nil
    
    private var lsmObservation: NSObjectProtocol?
    
    init(linkSessionManager: LinkSessionManager) {
        self.linkSessionManager = linkSessionManager
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.title = "Link Session"
        self.view.backgroundColor = .secondarySystemBackground
        
        statusLabel = UILabel(frame: .zero)
        statusLabel.font = UIFont.systemFont(ofSize: 20.0, weight: .bold)
        statusLabel.numberOfLines = 2
        statusLabel.textAlignment = .center
        self.view.addSubview(statusLabel)
        
        var primaryButtonConfig = UIButton.Configuration.filled()
        primaryButtonConfig.title = LinkSessionViewController.CreateSessionString
        primaryButtonConfig.buttonSize = .large
        primaryButtonConfig.baseBackgroundColor = .systemGreen
        let primaryButtonAction = UIAction { [weak self] _ in
            self?._handlePrimaryAction()
        }
        primaryButton = UIButton(configuration: primaryButtonConfig, primaryAction: primaryButtonAction)
        self.view.addSubview(primaryButton)
        
        var secondaryButtonConfig = UIButton.Configuration.filled()
        secondaryButtonConfig.title = LinkSessionViewController.JoinSessionString
        secondaryButtonConfig.buttonSize = .large
        secondaryButtonConfig.baseBackgroundColor = .systemBlue
        let secondaryButtonAction = UIAction { [weak self] _ in
            self?._handleSecondaryAction()
        }
        secondaryButton = UIButton(configuration: secondaryButtonConfig, primaryAction: secondaryButtonAction)
        self.view.addSubview(secondaryButton)
        
        NotificationCenter.default.addObserver(forName: LinkSessionManager.RoomStatusChangedNotification, object: nil, queue: nil) { [weak self] _ in
            DispatchQueue.main.async {
                self?._handleStatusChange()
            }
        }
        
        
        _autoRunIfNecessary()
        _updateUI()
    }
    
    private func _handleStatusChange() {
        _autoRunIfNecessary()
        _updateUI()
    }
    
    override func viewWillLayoutSubviews() {
        super.viewWillLayoutSubviews()
        
        var availableBounds = self.view.bounds.inset(by: self.view.layoutMargins)
        // 1. place the secondary button on the bottom
        let minSecondaryButtonSize = secondaryButton.sizeThatFits(availableBounds.size)
        let secondaryButtonSize = CGSize(width: max(minSecondaryButtonSize.width, availableBounds.width), height: minSecondaryButtonSize.height)
        let secondaryButtonX = availableBounds.minX + ((availableBounds.width - secondaryButtonSize.width) / 2.0)
        let secondaryButtonY = availableBounds.maxY - secondaryButtonSize.height
        let secondaryButtonFrame = CGRect(x: secondaryButtonX, y: secondaryButtonY, width: secondaryButtonSize.width, height: secondaryButtonSize.height)
        secondaryButton.frame = secondaryButtonFrame
        // reduce available bounds from the bottom
        availableBounds.size.height -= (availableBounds.maxY - secondaryButtonFrame.minY)
        
        // 2. place the primary button just above that
        let minPrimaryButtonSize = primaryButton.sizeThatFits(availableBounds.size)
        let primaryButtonSize = CGSize(width: max(minPrimaryButtonSize.width, availableBounds.width), height: minPrimaryButtonSize.height)
        let primaryButtonX = availableBounds.minX + ((availableBounds.width - primaryButtonSize.width) / 2.0)
        let primaryButtonY = availableBounds.maxY - 10.0 - primaryButtonSize.height
        let primaryButtonFrame = CGRect(x: primaryButtonX, y: primaryButtonY, width: primaryButtonSize.width, height: primaryButtonSize.height)
        primaryButton.frame = primaryButtonFrame
        // reduce available bounds from the bottom
        availableBounds.size.height -= (availableBounds.maxY - primaryButtonFrame.minY)
        
        // 3. center the status label in the rest of the available space
        let statusSize = statusLabel.sizeThatFits(availableBounds.size)
        let statusX = availableBounds.minX + ((availableBounds.width - statusSize.width) / 2.0)
        let statusY = availableBounds.minY + ((availableBounds.height - statusSize.height) / 2.0)
        let statusFrame = CGRect(x: statusX, y: statusY, width: statusSize.width, height: statusSize.height)
        statusLabel.frame = statusFrame
    }
    
    private var autorunThroughCheck = true
    private var autorunThroughConnect = false
    private func _autoRunIfNecessary() {
        if linkSessionManager.isWorking {
            return
        }
        
        let roomStatus = linkSessionManager.roomStatus
        let isPostCheck: Bool
        let isPostConnect: Bool
        switch roomStatus {
        case .notChecked:
            if autorunThroughCheck {
                linkSessionManager.checkForRooms()
            }
            isPostCheck = false
            isPostConnect = false
        case .noRooms:
            isPostCheck = true
            isPostConnect = false
        case .roomAvailable:
            if autorunThroughConnect {
                linkSessionManager.connectToRoom()
            }
            isPostCheck = true
            isPostConnect = false
        case .connectingToRoom:
            isPostCheck = true
            isPostConnect = false
        case .connectedToRoom:
            isPostCheck = true
            isPostConnect = true
        case .error:
            isPostCheck = true
            isPostConnect = true
        case .disconnected:
            isPostCheck = true
            isPostConnect = true
        }
        
        if isPostCheck {
            autorunThroughCheck = false
        }
        if isPostConnect {
            autorunThroughConnect = false
        }
    }
    
    private func _updateUI() {
        let roomStatus = linkSessionManager.roomStatus
        var isWorking = linkSessionManager.isWorking || autorunThroughCheck || autorunThroughConnect
        if case .connectingToRoom = roomStatus {
            isWorking = true
        }
        print("Updating with status \(roomStatus), isWorking \(isWorking)")
        let primaryTitle: String
        let secondaryTitle: String
        let statusText: String
        var primaryColor = UIColor.systemBlue
        var secondaryColor = UIColor.systemBlue
        switch roomStatus {
        case .notChecked:
            primaryTitle = LinkSessionViewController.CheckForSessionsString
            secondaryTitle = LinkSessionViewController.JoinSessionString
            secondaryColor = .systemGreen
            statusText = isWorking ? "Checking for link sessions…" : "Check for active sessions or join with a code"
        case .noRooms:
            primaryTitle = LinkSessionViewController.CreateSessionString
            secondaryTitle = LinkSessionViewController.JoinSessionString
            secondaryColor = .systemGreen
            statusText = "No active link sessions"
        case .roomAvailable(let linkRoomClientInfo), .connectingToRoom(let linkRoomClientInfo):
            primaryTitle = LinkSessionViewController.ConnectToSessionString
            secondaryTitle = LinkSessionViewController.EndSessionString
            secondaryColor = .systemRed
            statusText = "Available session with code \n\(linkRoomClientInfo.roomCode)"
        case .connectedToRoom(let linkRoomClientInfo):
            primaryTitle = LinkSessionViewController.EndSessionString
            secondaryTitle = LinkSessionViewController.LeaveSessionString
            primaryColor = .systemRed
            secondaryColor = .systemRed
            statusText = "Connected to session with code \n\(linkRoomClientInfo.roomCode)"
        case .error:
            primaryTitle = LinkSessionViewController.CheckForSessionsString
            secondaryTitle = LinkSessionViewController.JoinSessionString
            secondaryColor = .systemGreen
            // TODO: Better error messages
            statusText = "An error occurred"
        case .disconnected:
            primaryTitle = LinkSessionViewController.CheckForSessionsString
            secondaryTitle = LinkSessionViewController.JoinSessionString
            secondaryColor = .systemGreen
            statusText = "Disconnected"
        }
        
        if !isWorking {
            // only change titles when not working
            if var config = primaryButton.configuration {
                config.title = primaryTitle
                config.baseBackgroundColor = primaryColor
                primaryButton.configuration = config
            }
            if var config = secondaryButton.configuration {
                config.title = secondaryTitle
                config.baseBackgroundColor = secondaryColor
                secondaryButton.configuration = config
            }
        }
        primaryButton.isEnabled = !isWorking
        secondaryButton.isEnabled = !isWorking
        statusLabel.text = statusText
        self.view.setNeedsLayout()
    }
    
    private func _handlePrimaryAction() {
        let roomStatus = linkSessionManager.roomStatus
        switch roomStatus {
        case .notChecked, .error, .disconnected:
            linkSessionManager.checkForRooms()
        case .noRooms:
            autorunThroughConnect = true
            linkSessionManager.createRoom()
        case .roomAvailable:
            linkSessionManager.connectToRoom()
        case .connectingToRoom:
            break
        case .connectedToRoom:
            linkSessionManager.closeRoom()
        }
        _updateUI()
    }
    
    private func _handleSecondaryAction() {
        let roomStatus = linkSessionManager.roomStatus
        switch roomStatus {
        case .notChecked, .noRooms, .error, .disconnected:
            _showJoinRoomAlert()
        case .roomAvailable:
            linkSessionManager.closeRoom()
        case .connectingToRoom:
            break
        case .connectedToRoom:
            linkSessionManager.disconnect()
            break
        }
        _updateUI()
    }
    
    private func _showJoinRoomAlert() {
        let alert = UIAlertController(title: "Join Link Session", message: "Enter join code", preferredStyle: .alert)
        alert.addTextField { textField in
            textField.placeholder = "ABC123"
        }
        let okAction = UIAlertAction(title: "OK", style: .default) { [weak self] _ in
            if let text = alert.textFields?.first?.text {
                self?._joinRoom(text)
            }
        }
        alert.addAction(okAction)
        alert.addAction(UIAlertAction(title: "Cancel", style: .cancel))
        self.present(alert, animated: true)
    }
    
    private func _joinRoom(_ roomCode: String) {
        autorunThroughConnect = true
        linkSessionManager.joinRoom(roomCode)
        _updateUI()
    }
}
