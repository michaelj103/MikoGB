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
    private static let RegistrationStateKeyLegacy = "UserIDRegistrationState"
    
    // "default" device ID. May phase out
    private static let DeviceIDKey = "DeviceID"
    private static let OverrideDeviceIDKey = "OverrideDeviceIDKey"
    static let UserIDChangedNotification = Notification.Name(rawValue: "UserIDChangedNotification")
    
    enum RegistrationStatus: Equatable {
        case notRegistered
        case unverified(String)
        case verified(String)
    }
    private(set) var registrationStatus: RegistrationStatus {
        didSet {
            if oldValue != registrationStatus {
                _runPendingVerificationBlocksIfNecessary()
                NotificationCenter.default.post(name: UserIdentityController.UserIDChangedNotification, object: nil)
            }
        }
    }
    
    typealias RegistrationCompletion = (Result<String, Error>) -> Void
    private var pendingVerificationBlocks = [RegistrationCompletion]()
    private var serverConfigurationObservation: NSObjectProtocol? = nil
    
    static var sharedIdentityController: UserIdentityController = {
        let userIdentityController = UserIdentityController()
        return userIdentityController
    }()
    
    private init() {
        // can remove the key deletion in a future version
        UserDefaults.standard.removeObject(forKey: UserIdentityController.RegistrationStateKeyLegacy)
        
        // Standard user ID for default config
        if let deviceID = UserDefaults.standard.string(forKey: UserIdentityController.DeviceIDKey) {
            registrationStatus = .unverified(deviceID)
        } else {
            registrationStatus = .notRegistered
        }
        
        serverConfigurationObservation = NotificationCenter.default.addObserver(forName: ServerConfiguration.ServerConfigurationChangedNotification, object: nil, queue: nil) { [weak self] _ in
            DispatchQueue.main.async {
                self?._hostOverrideDidChange()
            }
        }
    }
    
    deinit {
        if let serverConfigurationObservation = serverConfigurationObservation {
            NotificationCenter.default.removeObserver(serverConfigurationObservation)
        }
    }
    
    private func _runPendingVerificationBlocks(with result: Result<String, Error>) {
        dispatchPrecondition(condition: .onQueue(.main))
        let blocks = pendingVerificationBlocks
        pendingVerificationBlocks = []
        for block in blocks {
            DispatchQueue.main.async {
                block(result)
            }
        }
    }
    
    private func _runPendingVerificationBlocksIfNecessary() {
        if case .verified(let deviceID) = registrationStatus {
            _runPendingVerificationBlocks(with: .success(deviceID))
        }
    }
    
    private func _failPendingCompletionBlocks(_ error: Error) {
        _runPendingVerificationBlocks(with: .failure(error))
    }
    
    private func _saveIDIfNecessary(_ newID: String) {
        if !ServerConfiguration.hasHostOverride() {
            UserDefaults.standard.setValue(newID, forKey: UserIdentityController.DeviceIDKey)
        } else {
            UserDefaults.standard.setValue(newID, forKey: UserIdentityController.OverrideDeviceIDKey)
        }
    }
    
    private func _deleteIDIfNecessary() {
        if !ServerConfiguration.hasHostOverride() {
            UserDefaults.standard.removeObject(forKey: UserIdentityController.DeviceIDKey)
        } else {
            UserDefaults.standard.removeObject(forKey: UserIdentityController.OverrideDeviceIDKey)
        }
    }
    
    // MARK: - Managing temporary accounts for test server instances
    
    private func _hostOverrideDidChange() {
        if !ServerConfiguration.hasHostOverride() {
            // reload standard ID
            if let deviceID = UserDefaults.standard.string(forKey: UserIdentityController.DeviceIDKey) {
                registrationStatus = .unverified(deviceID)
            } else {
                registrationStatus = .notRegistered
            }
        } else {
            // load override ID
            if let deviceID = UserDefaults.standard.string(forKey: UserIdentityController.OverrideDeviceIDKey) {
                registrationStatus = .unverified(deviceID)
            } else {
                registrationStatus = .notRegistered
            }
        }
    }
    
    // MARK: - Register Users
    
    private var isRunning = false
    private var lastRegistrationAttemptTime: Date?
    func ensureRegistration(force: Bool = false, completion: RegistrationCompletion? = nil) {
        DispatchQueue.main.async {
            if self.isRunning {
                completion?(.failure(SimpleError("Registration already running")))
                return
            }
            if case .verified(let deviceID) = self.registrationStatus {
                completion?(.success(deviceID))
                return
            }
            
            var canAttempt = true
            let currentTime = Date()
            if !force {
                // Only check once an hour automatically
                if let lastAttemptTime = self.lastRegistrationAttemptTime {
                    let timeSinceLast = currentTime.timeIntervalSince(lastAttemptTime)
                    if timeSinceLast < 3600.0 {
                        print("Skipping user registration attempt, it's only been \(timeSinceLast) seconds")
                        canAttempt = false
                    }
                }
            }
            
            if canAttempt {
                if let completion = completion {
                    self.pendingVerificationBlocks.append(completion)
                }
                self.lastRegistrationAttemptTime = currentTime
                self.isRunning = true
                self._runStateMachine()
            } else {
                completion?(.failure(SimpleError("Automatic login attempted too recently")))
            }
        }
    }
    
    private func _runStateMachine() {
        switch registrationStatus {
        case .notRegistered:
            // register a new ID
            _registerID { [weak self] result in
                switch result {
                case .success(let newDeviceID):
                    self?._userRegistered(newDeviceID)
                case .failure(let error):
                    self?._failPendingCompletionBlocks(SimpleError("Failed to verify user with error \(error)"))
                    self?.isRunning = false
                }
            }
        case .unverified(let unverifiedID):
            // verify that the ID exists
            _verifyID(unverifiedID) { [weak self] result in
                switch result {
                case .success(let exists):
                    self?._userVerified(exists, id: unverifiedID)
                case .failure(let error):
                    self?._failPendingCompletionBlocks(SimpleError("Failed to verify user with error \(error)"))
                    self?.isRunning = false
                }
            }
        case .verified(_):
            preconditionFailure("This shouldn't be reachable")
        }
    }
    
    private func _userVerified(_ exists: Bool, id: String) {
        if exists {
            print("Verified user")
            registrationStatus = .verified(id) // call completion indirectly
            isRunning = false
        } else {
            // we received an explicit negative response to verification. Delete the ID we have and try to register a new one
            registrationStatus = .notRegistered
            _deleteIDIfNecessary()
            DispatchQueue.main.async {
                self._runStateMachine()
            }
        }
    }
    
    private func _userRegistered(_ deviceID: String) {
        _saveIDIfNecessary(deviceID)
        registrationStatus = .verified(deviceID) // call completion indirectly
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
        
        let key = ServerConfiguration.hasHostOverride() ? "foo" : ServerConfiguration.APIKey
        let requestPayload = RegisterUserHTTPRequestPayload(key: key, displayName: nil)
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
        guard case .verified(let deviceID) = registrationStatus else {
            print("Skipping checkin because we don't have a verified user")
            pendingVerificationBlocks.append { [weak self] _ in
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
        
        let requestPayload = CheckInUserHTTPRequestPayload(deviceID: deviceID, version: version)
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
        guard case .verified(let deviceID) = registrationStatus else {
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
