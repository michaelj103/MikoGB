//
//  UserIdentityController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/8/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation
import GBServerPayloads

class UserIdentityController {
    private static let DeviceIDKey = "DeviceID"
    private static let RegistrationStateKeyLegacy = "UserIDRegistrationState"
    
    private var deviceID: String? = nil
    private var isUserVerified = false {
        didSet {
            _runPendingVerificationBlocksIfNecessary()
        }
    }
    
    private var pendingVerificationBlocks = [ () -> Void ]()
    
    static var sharedIdentityController: UserIdentityController = {
        let userIdentityController = UserIdentityController()
        return userIdentityController
    }()
    
    private init() {
        // can remove the key deletion in a future version
        UserDefaults.standard.removeObject(forKey: UserIdentityController.RegistrationStateKeyLegacy)
        deviceID = UserDefaults.standard.string(forKey: UserIdentityController.DeviceIDKey)
    }
    
    private func _runPendingVerificationBlocksIfNecessary() {
        dispatchPrecondition(condition: .onQueue(.main))
        if isUserVerified {
            let blocks = pendingVerificationBlocks
            pendingVerificationBlocks = []
            for block in blocks {
                DispatchQueue.main.async {
                    block()
                }
            }
        }
    }
    
    // MARK: - Register Users
    
    private var isRunning = false
    private var lastRegistrationAttemptTime: Date?
    func ensureRegistration() {
        DispatchQueue.main.async {
            if self.isRunning {
                return
            }
            if self.isUserVerified {
                return
            }
            
            var canAttempt = true
            let currentTime = Date()
            if let lastAttemptTime = self.lastRegistrationAttemptTime {
                let timeSinceLast = currentTime.timeIntervalSince(lastAttemptTime)
                if timeSinceLast < 3600.0 {
                    print("Skipping user registration attempt, it's only been \(timeSinceLast) seconds")
                    canAttempt = false
                }
            }
            
            if canAttempt {
                self.lastRegistrationAttemptTime = currentTime
                self.isRunning = true
                self._runStateMachine()
            }
        }
    }
    
    private func _runStateMachine() {
        if let deviceID = deviceID {
            // verify that the ID exists
            _verifyID(deviceID) { [weak self] result in
                switch result {
                case .success(let exists):
                    self?._userVerified(exists)
                case .failure(let error):
                    print("Failed to verify user with error \(error)")
                    self?.isRunning = false
                }
            }
            
        } else {
            // register a new ID
            _registerID { [weak self] result in
                switch result {
                case .success(let newDeviceID):
                    self?._userRegistered(newDeviceID)
                case .failure(let error):
                    print("Failed to register new user with error \(error)")
                    self?.isRunning = false
                }
            }
        }
    }
    
    private func _userVerified(_ exists: Bool) {
        if exists {
            print("Verified user")
            isUserVerified = true
            isRunning = false
        } else {
            // we received an explicit negative response to verification. Delete the ID we have and try to register a new one
            deviceID = nil
            UserDefaults.standard.removeObject(forKey: UserIdentityController.DeviceIDKey)
            DispatchQueue.main.async {
                self._runStateMachine()
            }
        }
    }
    
    private func _userRegistered(_ deviceID: String) {
        UserDefaults.standard.setValue(deviceID, forKey: UserIdentityController.DeviceIDKey)
        self.deviceID = deviceID
        self.isUserVerified = true
        self.isRunning = false
    }
    
    private func _verifyID(_ deviceID: String, completion: @escaping (Result<Bool, Error>) -> Void) {
        let queryItems = [URLQueryItem(name: "deviceID", value: deviceID)]
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/verifyUser", queryItems: queryItems) else {
            DispatchQueue.main.async {
                completion(.failure(SimpleError("Failed to construct registration URL")))
            }
            return
        }
        
        let request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        let networkManager = NetworkManager.sharedNetworkManager
        networkManager.submitRequest(request) { result in
            let finalResult: Result<Bool,Error>
            switch result {
            case .success(let data):
                finalResult = UserIdentityController._decodeVerificationResponse(data)
            case .failure(let error):
                finalResult = .failure(error)
            }
            
            DispatchQueue.main.async {
                completion(finalResult)
            }
        }
    }
    
    private func _registerID(_ completion: @escaping (Result<String,Error>) -> Void) {
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/registerUser2") else {
            print("Failed to construct registration URL")
            DispatchQueue.main.async {
                completion(.failure(SimpleError("Failed to construct registration URL")))
            }
            return
        }
        
        let requestPayload = RegisterUserHTTPRequestPayload(key: ServerConfiguration.APIKey, displayName: nil)
        let payloadData: Data
        do {
            payloadData = try JSONEncoder().encode(requestPayload)
        } catch {
            print("Failed to encode user registration payload data with error \(error)")
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
                let response = UserIdentityController._decodeRegistrationResponse(data)
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
    
    private static func _decodeVerificationResponse(_ data: Data) -> Result<Bool, Error> {
        let response: Result<Bool, Error>
        do {
            let payload = try JSONDecoder().decode(VerifyUserHTTPResponsePayload.self, from: data)
            switch payload {
            case .userExists:
                response = .success(true)
            case .userDoesNotExist:
                response = .success(false)
            }
        } catch {
            response = .failure(error)
        }
        return response
    }
    
    private static func _decodeRegistrationResponse(_ data: Data) -> Result<String,Error> {
        let response: Result<String, Error>
        do {
            let payload = try JSONDecoder().decode(RegisterUserHTTPResponsePayload.self, from: data)
            response = .success(payload.deviceID)
        } catch {
            response = .failure(error)
        }
        
        return response
    }
    
    // MARK: - Check In
    
    private var lastCheckInAttemptTime: Date?
    func checkIn() {
        dispatchPrecondition(condition: .onQueue(.main))
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/checkIn") else {
            print("Failed to construct check in URL")
            return
        }
        guard let finalDeviceID = deviceID, isUserVerified else {
            print("Skipping checkin because we don't have a verified user")
            pendingVerificationBlocks.append { [weak self] in
                self?.checkIn()
            }
            return
        }
        
        let currentTime = Date()
        if let lastAttemptTime = lastCheckInAttemptTime {
            let timeSinceLast = currentTime.timeIntervalSince(lastAttemptTime)
            if timeSinceLast < (60.0 * 30.0) {
                print("Skipping user checkin attempt, it's only been \(timeSinceLast) seconds")
                return
            }
        }
        lastCheckInAttemptTime = currentTime
        
        let (version, _) = UpdateManager.getCurrentVersionAndBuild()
        
        let requestPayload = CheckInUserHTTPRequestPayload(deviceID: finalDeviceID, version: version)
        let payloadData: Data
        do {
            payloadData = try JSONEncoder().encode(requestPayload)
        } catch {
            print("Failed to encode user checkin payload data with error \(error)")
            return
        }
        
        var request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        request.httpMethod = "POST"
        request.setValue("application/json; charset=UTF-8", forHTTPHeaderField: "Content-Type")
        request.httpBody = payloadData
        
        NetworkManager.sharedNetworkManager.submitRequest(request) { result in
            switch result {
            case .success(let data):
                if let response = try? JSONDecoder().decode(GenericMessageResponse.self, from: data) {
                    print("Check in response from server: \(response.getMessage())")
                } else {
                    print("Unable to decode check in response from server")
                }
            case .failure(let error):
                print("Check in attempt failed with error \(error)")
            }
        }
    }
    
    // MARK: - Check for debug authorization
    private var debugAuth: Bool?
    func getDebugAuthorization(_ completion: @escaping (Bool) -> Void) {
        if let knownAuth = debugAuth {
            DispatchQueue.main.async {
                completion(knownAuth)
            }
            return
        }
        guard let deviceID = deviceID else {
            print("No device ID yet")
            DispatchQueue.main.async {
                completion(false)
            }
            return
        }
        
        let query = [URLQueryItem(name: "deviceID", value: deviceID)]
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/debugAuth", queryItems: query) else {
            print("Failed to construct debug auth URL")
            return
        }
        
        let request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        NetworkManager.sharedNetworkManager.submitRequest(request) { result in
            switch result {
            case .success(let data):
                if let response = try? JSONDecoder().decode(UserGetDebugAuthHTTPResponsePayload.self, from: data) {
                    DispatchQueue.main.async {
                        completion(response.authorized)
                    }
                } else {
                    print("Unable to decode debug auth response from server")
                    DispatchQueue.main.async {
                        completion(false)
                    }
                }
            case .failure(let error):
                print("Debug auth check failed with error \(error)")
                DispatchQueue.main.async {
                    completion(false)
                }
            }
        }
    }
}
