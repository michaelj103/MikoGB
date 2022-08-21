//
//  LinkSessionManager.swift
//  MikoGB
//
//  Created by Michael Brandt on 8/17/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation
import GBServerPayloads

class LinkSessionManager: NSObject, GBEngineSerialDestination {
    private let engine: GBEngine
    private var userIdentityObserver: NSObjectProtocol? = nil
    
    init(_ engine: GBEngine) {
        self.engine = engine
        super.init()
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
    }
    
    func engine(_ engine: GBEngine, presentByte byte: UInt8) {
        // nothing for now
    }
    
    func engine(_ engine: GBEngine, pushByte byte: UInt8) {
        engine.receivePulledSerialByte(0xFF)
    }
    
    // MARK: - Room state management
    
    static let RoomStatusChangedNotification = NSNotification.Name(rawValue: "RoomStatusChangedNotification")
    
    enum RoomStatus {
        case notChecked
        case noRooms
        case roomAvailable(LinkRoomClientInfo)
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
        roomStatus = .notChecked
    }
    
    // MARK: - Checking for rooms
    
    private func _canRunRoomCheck() -> Bool {
        switch roomStatus {
        case .notChecked, .error, .disconnected:
            return true
        case .noRooms, .roomAvailable, .connectedToRoom:
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
                let response = LinkSessionManager._decodeRoomInfoResponse(data)
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
    
    private static func _decodeRoomInfoResponse(_ data: Data) -> Result<PossibleLinkRoomClientInfo, Error> {
        let response: Result<PossibleLinkRoomClientInfo, Error>
        do {
            let payload = try JSONDecoder().decode(PossibleLinkRoomClientInfo.self, from: data)
            response = .success(payload)
        } catch {
            response = .failure(error)
        }
        
        return response
    }
    
    // MARK: - Creating rooms
    
    private func _canRunRoomCreation() -> Bool {
        switch roomStatus {
        case .notChecked, .error, .disconnected, .roomAvailable, .connectedToRoom:
            return false
        case .noRooms:
            // TODO: Maybe allow error/disconnected states for more automatic stepping through these phases
            return true
        }
    }
    
    func createRoom() {
        guard !isWorking else {
            print("Already running")
            return
        }
        
        guard _canRunRoomCreation() else {
            print("Shouldn't create rooms before checking if any exist")
            return
        }
        
        isWorking = true
        _createRoom { [weak self] result in
            self?._handleRoomCreationResult(result)
        }
    }
    
    private func _handleRoomCreationResult(_ result: Result<LinkRoomClientInfo,Error>) {
        isWorking = false
        switch result {
        case .success(let clientInfo):
            roomStatus = .roomAvailable(clientInfo)
        case .failure(let error):
            print("Room check failed with error: \(error)")
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
        
        var request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        request.httpMethod = "POST"
        request.setValue("application/json; charset=UTF-8", forHTTPHeaderField: "Content-Type")
        request.httpBody = payloadData
        
        let networkManager = NetworkManager.sharedNetworkManager
        networkManager.submitRequest(request) { result in
            switch result {
            case .success(let data):
                let response = LinkSessionManager._decodeRoomCreationResponse(data)
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
    
    private static func _decodeRoomCreationResponse(_ data: Data) -> Result<LinkRoomClientInfo,Error> {
        let response: Result<LinkRoomClientInfo, Error>
        do {
            let payload = try JSONDecoder().decode(LinkRoomClientInfo.self, from: data)
            response = .success(payload)
        } catch {
            response = .failure(error)
        }
        
        return response
    }
}
