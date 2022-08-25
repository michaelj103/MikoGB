//
//  LinkSessionManager.swift
//  MikoGB
//
//  Created by Michael Brandt on 8/17/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation
import GBServerPayloads
import GBLinkServerProtocol

class LinkSessionManager: NSObject, GBEngineSerialDestination {
    private let engine: GBEngine
    private var userIdentityObserver: NSObjectProtocol? = nil
    
    init(_ engine: GBEngine) {
        self.engine = engine
        super.init()
        engine.serialDestination = self
        userIdentityObserver = NotificationCenter.default.addObserver(forName: UserIdentityController.UserIDChangedNotification, object: nil, queue: nil) { [weak self] _ in
            DispatchQueue.main.async {
                self?._userIDStateDidChange()
            }
        }
    }
    
    deinit {
        if let userIdentityObserver = userIdentityObserver {
            NotificationCenter.default.removeObserver(userIdentityObserver)
        }
        
        linkConnection?.close()
    }
    
    private var presentedBytePendingConnection: UInt8?
    func _handleInitialConnection(_ connection: LinkClientConnection) {
        if let byte = presentedBytePendingConnection {
            connection.write([LinkServerCommand.presentByte.rawValue] + [byte])
        }
        presentedBytePendingConnection = nil
    }
    
    func engine(_ engine: GBEngine, presentByte byte: UInt8) {
        // on main
        if let connection = linkConnection {
            connection.write([LinkServerCommand.presentByte.rawValue] + [byte])
            presentedBytePendingConnection = nil
        } else {
            presentedBytePendingConnection = byte
        }
    }
    
    func engine(_ engine: GBEngine, pushByte byte: UInt8) {
        // on main
        presentedBytePendingConnection = nil
        if let connection = linkConnection {
            connection.write([LinkServerCommand.pushByte.rawValue] + [byte])
        } else {
            engine.receivePulledSerialByte(0xFF)
        }
    }
    
    // incoming mesage from server
    private func _handleLinkMessage(_ message: LinkClientMessage) {
        presentedBytePendingConnection = nil
        switch message {
        case .didConnect:
            _handleDidConnect()
        case .pullByte(let byte):
            engine.receivePulledSerialByte(byte)
        case .pullByteStale, .commitStaleByte:
            // ignore for now (ever needed?)
            break
        case .bytePushed(let byte):
            engine.receivePushedSerialByte(byte)
        }
    }
    
    // MARK: - Room state management
    
    static let RoomStatusChangedNotification = NSNotification.Name(rawValue: "RoomStatusChangedNotification")
    
    enum RoomStatus {
        case notChecked
        case noRooms
        case roomAvailable(LinkRoomClientInfo)
        case connectingToRoom(LinkRoomClientInfo)
        case connectedToRoom(LinkRoomClientInfo)
        case error
        case disconnected
    }
    private(set) var roomStatus: RoomStatus = .notChecked {
        didSet {
            NotificationCenter.default.post(name: LinkSessionManager.RoomStatusChangedNotification, object: nil)
        }
    }
    
    private(set) var isWorking: Bool = false
    
    private func _userIDStateDidChange() {
        if let linkConnection = linkConnection {
            // if user account changes during a connection, close it. Otherwise reset the check pipeline
            linkConnection.close()
            roomStatus = .disconnected
        } else {
            roomStatus = .notChecked
        }
    }
    
    private static func _serverPost<T: Decodable>(_ url: URL, payload: Data, completion: @escaping (Result<T,Error>) -> Void) {
        var request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        request.httpMethod = "POST"
        request.setValue("application/json; charset=UTF-8", forHTTPHeaderField: "Content-Type")
        request.httpBody = payload
        
        let networkManager = NetworkManager.sharedNetworkManager
        networkManager.submitRequest(request) { result in
            switch result {
            case .success(let data):
                let response: Result<T,Error> = LinkSessionManager._decodeServerResponse(data)
                DispatchQueue.main.async {
                    completion(response)
                }
            case .failure(let error):
                DispatchQueue.main.async {
                    completion(.failure(error))
                }
            }
        }
    }
    
    private static func _decodeServerResponse<T: Decodable>(_ data: Data) -> Result<T, Error> {
        let response: Result<T, Error>
        do {
            let payload = try JSONDecoder().decode(T.self, from: data)
            response = .success(payload)
        } catch {
            response = .failure(error)
        }
        
        return response
    }
    
    // MARK: - Checking for rooms
    
    private func _canRunRoomCheck() -> Bool {
        switch roomStatus {
        case .notChecked, .error, .disconnected:
            return true
        case .noRooms, .roomAvailable, .connectingToRoom, .connectedToRoom:
            return false
        }
    }
    
    func checkForRooms() {
        guard !isWorking else {
            print("Already running")
            return
        }
        
        guard _canRunRoomCheck() else {
            print("Already checked for rooms")
            return
        }
        
        isWorking = true
        _checkForRooms { [weak self] result in
            self?._handleRoomCheckResult(result)
        }
    }
    
    private func _handleRoomCheckResult(_ result: Result<PossibleLinkRoomClientInfo,Error>) {
        isWorking = false
        switch result {
        case .success(let clientInfo):
            switch clientInfo {
            case .isNotInRoom:
                roomStatus = .noRooms
            case .isInRoom(let info):
                roomStatus = .roomAvailable(info)
            }
        case .failure(let error):
            print("Room check failed with error: \(error)")
            roomStatus = .error
        }
    }
    
    private func _checkForRooms(_ completion: @escaping (Result<PossibleLinkRoomClientInfo,Error>) -> Void) {
        let registrationStatus = UserIdentityController.sharedIdentityController.registrationStatus
        guard case .verified(let deviceID) = registrationStatus else {
            print("No verified user ID")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("No verified user ID for room check")))
            }
            return
        }
        
        let queryItems = [URLQueryItem(name: "deviceID", value: deviceID)]
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/getRoomInfo", queryItems: queryItems) else {
            print("Failed to construct room info URL")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("Failed to construct room info URL")))
            }
            return
        }
        
        let request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        let networkManager = NetworkManager.sharedNetworkManager
        networkManager.submitRequest(request) { result in
            switch result {
            case .success(let data):
                let response: Result<PossibleLinkRoomClientInfo,Error> = LinkSessionManager._decodeServerResponse(data)
                DispatchQueue.main.async {
                    completion(response)
                }
            case .failure(let error):
                DispatchQueue.main.async {
                    completion(.failure(error))
                }
            }
        }
    }
    
    // MARK: - Creating rooms
    
    private func _canRunRoomCreationOrJoin() -> Bool {
        switch roomStatus {
        case .notChecked, .roomAvailable, .connectingToRoom, .connectedToRoom:
            return false
        case .noRooms, .disconnected, .error:
            return true
        }
    }
    
    func createRoom() {
        guard !isWorking else {
            print("Already running")
            return
        }
        
        guard _canRunRoomCreationOrJoin() else {
            print("Shouldn't create rooms before checking if any exist")
            return
        }
        
        isWorking = true
        _createRoom { [weak self] result in
            self?._handleRoomCreateOrJoinResult(result)
        }
    }
    
    private func _handleRoomCreateOrJoinResult(_ result: Result<LinkRoomClientInfo,Error>) {
        isWorking = false
        switch result {
        case .success(let clientInfo):
            roomStatus = .roomAvailable(clientInfo)
        case .failure(let error):
            print("Room creation/join failed with error: \(error)")
            roomStatus = .error
        }
    }
    
    private func _createRoom(_ completion: @escaping (Result<LinkRoomClientInfo,Error>) -> Void) {
        let registrationStatus = UserIdentityController.sharedIdentityController.registrationStatus
        guard case .verified(let deviceID) = registrationStatus else {
            print("No verified user ID")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("No verified user ID for room creation")))
            }
            return
        }
        
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/createRoom") else {
            print("Failed to construct room creation URL")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("Failed to construct room creation URL")))
            }
            return
        }
        
        let requestPayload = CreateRoomHTTPRequestPayload(deviceID: deviceID)
        let payloadData: Data
        do {
            payloadData = try JSONEncoder().encode(requestPayload)
        } catch {
            print("Failed to encode room creation payload data with error \(error)")
            DispatchQueue.main.async {
                completion(.failure(error))
            }
            return
        }
        
        LinkSessionManager._serverPost(url, payload: payloadData, completion: completion)
    }
    
    // MARK: - Joining rooms
    
    func joinRoom(_ roomCode: String) {
        guard !isWorking else {
            print("Already running")
            return
        }
        
        guard _canRunRoomCreationOrJoin() else {
            print("Shouldn't join rooms before checking if any exist")
            return
        }
        
        isWorking = true
        _joinRoom(roomCode) { [weak self] result in
            self?._handleRoomCreateOrJoinResult(result)
        }
    }
    
    private func _joinRoom(_ roomCode: String, completion: @escaping (Result<LinkRoomClientInfo,Error>) -> Void) {
        let registrationStatus = UserIdentityController.sharedIdentityController.registrationStatus
        guard case .verified(let deviceID) = registrationStatus else {
            print("No verified user ID")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("No verified user ID for room join")))
            }
            return
        }
        
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/joinRoom") else {
            print("Failed to construct room join URL")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("Failed to construct room join URL")))
            }
            return
        }
        
        let requestPayload = JoinRoomHTTPRequestPayload(deviceID: deviceID, roomCode: roomCode)
        let payloadData: Data
        do {
            payloadData = try JSONEncoder().encode(requestPayload)
        } catch {
            print("Failed to encode room join payload data with error \(error)")
            DispatchQueue.main.async {
                completion(.failure(error))
            }
            return
        }
        
        LinkSessionManager._serverPost(url, payload: payloadData, completion: completion)
    }
    
    // MARK: - Closing rooms
    
    private func _canRunCloseRoom() -> Bool {
        switch roomStatus {
        case .notChecked, .noRooms, .error, .disconnected, .connectingToRoom:
            return false
        case .roomAvailable, .connectedToRoom:
            return true
        }
    }
    
    func closeRoom() {
        guard !isWorking else {
            print("Already running")
            return
        }
        
        guard _canRunCloseRoom() else {
            print("Can't close rooms if the user doesn't have any")
            return
        }
        
        isWorking = true
        _closeRoom { [weak self] result in
            self?._handleRoomCloseResult(result)
        }
    }
    
    private func _handleRoomCloseResult(_ result: Result<GenericMessageResponse,Error>) {
        isWorking = false
        switch result {
        case .success(let response):
            if case .success = response {
                roomStatus = .disconnected
            } else {
                let message = response.getMessage()
                print("Failed to close room with message: \(message)")
                roomStatus = .error
            }
        case .failure(let error):
            print("Room close failed with error: \(error)")
            roomStatus = .error
        }
    }
    
    private func _closeRoom(_ completion: @escaping (Result<GenericMessageResponse,Error>) -> Void) {
        let registrationStatus = UserIdentityController.sharedIdentityController.registrationStatus
        guard case .verified(let deviceID) = registrationStatus else {
            print("No verified user ID")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("No verified user ID for room close")))
            }
            return
        }
        
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/closeRoom") else {
            print("Failed to construct room close URL")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("Failed to construct room close URL")))
            }
            return
        }
        
        let requestPayload = CloseRoomHTTPRequestPayload(deviceID: deviceID)
        let payloadData: Data
        do {
            payloadData = try JSONEncoder().encode(requestPayload)
        } catch {
            print("Failed to encode room close payload data with error \(error)")
            DispatchQueue.main.async {
                completion(.failure(error))
            }
            return
        }
        
        LinkSessionManager._serverPost(url, payload: payloadData, completion: completion)
    }
    
    // MARK: - Connecting to rooms
    
    private var linkClientSession: LinkClientSession?
    private var linkConnection: LinkClientConnection?
    func connectToRoom() {
        guard !isWorking else {
            print("Already running")
            return
        }
        
        guard case .roomAvailable(let clientInfo) = roomStatus else {
            print("No available room to connect to")
            return
        }
        
        let (host, _, _) = ServerConfiguration.getHostAndPortAndScheme()
        linkClientSession = LinkClientSession(host: host, port: clientInfo.linkPort)
        
        let connection: LinkClientConnection?
        do {
            connection = try linkClientSession?.makeConnection()
            roomStatus = .connectingToRoom(clientInfo)
        } catch {
            print("Failed to connect with error \(error)")
            roomStatus = .error
            connection = nil
        }
        
        linkConnection = connection
        guard let realizedConnection = connection else {
            print("Failed to make connection without error")
            return
        }
        
        realizedConnection.setCloseCallback({ [weak self] result in
            DispatchQueue.main.async {
                self?._handleDidDisconnect(result)
            }
        })
        
        realizedConnection.setMessageCallback({ [weak self] message in
            DispatchQueue.main.async {
                self?._handleLinkMessage(message)
            }
        })
        
        let keyBytes = clientInfo.roomKey.stringValue.map{ $0.asciiValue! }
        realizedConnection.write([LinkServerCommand.connect.rawValue] + keyBytes)
    }
    
    private func _handleDidConnect() {
        guard case .connectingToRoom(let clientInfo) = roomStatus else {
            return
        }
        roomStatus = .connectedToRoom(clientInfo)
        
        guard let linkConnection = linkConnection else {
            return
        }
        _handleInitialConnection(linkConnection)
    }
    
    private var finishWorkOnDisconnect = false
    private func _handleDidDisconnect(_ result: Result<Void,Error>) {
        print("Did disconnect")
        linkClientSession = nil
        linkConnection = nil
        roomStatus = .disconnected
        if finishWorkOnDisconnect {
            finishWorkOnDisconnect = false
            isWorking = false
        }
    }
    
    // MARK: - Disconnecting from rooms
    
    func disconnect() {
        guard !isWorking else {
            print("Already running")
            return
        }
        
        guard let linkConnection = linkConnection else {
            print("No active connection to disconnect")
            return
        }
        
        print("Disconnect called")
        isWorking = true
        finishWorkOnDisconnect = true
        linkConnection.close()
    }
}

