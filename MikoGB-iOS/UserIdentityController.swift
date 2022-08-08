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
    private static let RegistrationStateKey = "UserIDRegistrationState"
    
    private var deviceID: String? = nil
    private var registrationState: UserIDRegistrationState
    
    static var sharedIdentityController: UserIdentityController = {
        let userIdentityController = UserIdentityController()
        return userIdentityController
    }()
    
    private init() {
        let stateNum = UserDefaults.standard.integer(forKey: UserIdentityController.RegistrationStateKey)
        registrationState = UserIDRegistrationState(rawValue: stateNum) ?? .initial
        switch registrationState {
        case .initial:
            break
        case .generatedID:
            fallthrough
        case .successfullyRegistered:
            deviceID = UserDefaults.standard.string(forKey: UserIdentityController.DeviceIDKey)
        }
        
        if deviceID == nil && registrationState != .initial {
            print("We don't have a device ID despite registration state of \(registrationState)")
            // No choice but to restart
            registrationState = .initial
        }
    }
    
    private var isRunning = false
    private var lastRegistrationAttemptTime: Date?
    func ensureRegistration() {
        DispatchQueue.main.async {
            if self.isRunning {
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
        // Run the basic registration state machine
        switch registrationState {
        case .initial:
            _generateID { success in
                if success {
                    self._runStateMachine()
                } else {
                    self.isRunning = false
                }
            }
        case .generatedID:
            _registerID { success in
                if success {
                    self._runStateMachine()
                } else {
                    self.isRunning = false
                }
            }
        case .successfullyRegistered:
            print("User deviceID is registered")
            self.isRunning = false
            return
        }
    }
    
    private func _generateID(_ completion: @escaping (Bool) -> Void) {
        if let generatedID = UserIdentityController._generateRandomBytes() {
            // success. Store updated state
            self.deviceID = generatedID
            self.registrationState = .generatedID
            UserDefaults.standard.set(self.deviceID, forKey: UserIdentityController.DeviceIDKey)
            UserDefaults.standard.set(self.registrationState.rawValue, forKey: UserIdentityController.RegistrationStateKey)
            DispatchQueue.main.async {
                completion(true)
            }
            
        } else {
            print("Failed to generate deviceID. Will retry later")
            completion(false)
        }
    }
    
    private func _registerID(_ completion: @escaping (Bool) -> Void) {
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/registerUser") else {
            print("Failed to construct registration URL")
            DispatchQueue.main.async {
                completion(false)
            }
            return
        }
        guard let finalDeviceID = self.deviceID else {
            print("Failed to construct registration URL")
            DispatchQueue.main.async {
                completion(false)
            }
            return
        }
        
        let requestPayload = RegisterUserHTTPRequestPayload(deviceID: finalDeviceID, displayName: nil)
        let payloadData: Data
        do {
            payloadData = try JSONEncoder().encode(requestPayload)
        } catch {
            print("Failed to encode user registration payload data with error \(error)")
            DispatchQueue.main.async {
                completion(false)
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
                print("Registered user with server response \(response)")
                DispatchQueue.main.async {
                    self.registrationState = .successfullyRegistered
                    UserDefaults.standard.set(self.registrationState.rawValue, forKey: UserIdentityController.RegistrationStateKey)
                    completion(true)
                }
            case .failure(let error):
                print("Failed to register user with error \(error)")
                DispatchQueue.main.async {
                    completion(false)
                }
            }
        }
    }
    
    private static func _decodeRegistrationResponse(_ data: Data) -> String {
        if let response = try? JSONDecoder().decode(GenericMessageResponse.self, from: data) {
            return response.getMessage()
        } else if let string = String(data: data, encoding: .utf8) {
            return string
        } else {
            return "<Unable to decode response data>"
        }
    }
    
    private static func _generateRandomBytes() -> String? {
        var keyData = Data(count: 16)
        let success = keyData.withUnsafeMutableBytes { (mutableBytes: UnsafeMutableRawBufferPointer) -> Bool in
            if let basePointer = mutableBytes.baseAddress {
                let result = SecRandomCopyBytes(kSecRandomDefault, 16, basePointer)
                return result == errSecSuccess
            } else {
                return false
            }
        }
        if success {
            let encoded = keyData.base64EncodedString().trimmingCharacters(in: CharacterSet(charactersIn: "="))
            return encoded
        } else {
            print("Problem generating random bytes")
            return nil
        }
    }
}

fileprivate enum UserIDRegistrationState: Int {
    case initial
    case generatedID
    case successfullyRegistered
}
