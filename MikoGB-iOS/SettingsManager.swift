//
//  SettingsManager.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/9/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class SettingsManager {
    static var sharedInstance: SettingsManager = {
        let settingsManager = SettingsManager()
        return settingsManager
    }()
    
    static let SettingsChangedNotification = Notification.Name("MikoGBSettingsChangedNotification")
    
    // Settings Keys
    private static let GenerateAudioKey = "GenerateAudioKey"
    private static let RespectMuteSwitchKey = "RespectMuteSwitchKey"
    
    private let defaults = UserDefaults.standard
    
    init() {
        defaults.register(defaults: [
            SettingsManager.GenerateAudioKey : true,
            SettingsManager.RespectMuteSwitchKey : true,
        ])
    }
    
    @UserDefaultsBacked(key: GenerateAudioKey) var shouldGenerateAudio: Bool
    @UserDefaultsBacked(key: RespectMuteSwitchKey) var shouldRespectMuteSwitch: Bool
    
    // in-memory
    var checkForStagingUpdates: Bool = false
}

@propertyWrapper struct UserDefaultsBacked<Value> {
    let key: String
    let defaults: UserDefaults = .standard
    
    var wrappedValue: Value {
        get {
            defaults.value(forKey: key) as! Value
        }
        set {
            defaults.set(newValue, forKey: key)
            DispatchQueue.main.async {
                NotificationCenter.default.post(name: SettingsManager.SettingsChangedNotification, object: nil)
            }
        }
    }
    
    init(key: String) {
        self.key = key
    }
}
