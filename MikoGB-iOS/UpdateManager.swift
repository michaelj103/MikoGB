//
//  UpdateManager.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 7/29/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class UpdateManager {
    private static let BuildVersion = 2
    private static let BuildVersionKey = "BuildVersion"
    
    static func migrateIfNecessary() {
        let currentVersion = UserDefaults.standard.integer(forKey: BuildVersionKey)
        // migration logic for versions < BuildVersion
        if currentVersion < BuildVersion {
            _runMigration(from: currentVersion, to: BuildVersion)
            // Migration complete
            UserDefaults.standard.set(BuildVersion, forKey: BuildVersionKey)
        }
    }
    
    private static func _runMigration(from fromVersion: Int, to toVersion: Int) {
        
    }
    
    private static var LastUpdateCheckTime: Date?
    
    static func checkForUpdate(_ completion: @escaping (Bool, String)->()) {
        guard let url = URL(string: "https://***REMOVED***/currentVersionInfo") else {
            print("Failed to construct update check URL")
            return
        }
        let currentTime = Date()
        if let lastCheckTime = LastUpdateCheckTime {
            let timeSinceLast = currentTime.timeIntervalSince(lastCheckTime)
            if timeSinceLast < 3600.0 {
                // Don't check all the time, just once per hour
                print("Skipping update check, it's only been \(timeSinceLast) seconds")
                return
            }
        }
        
        LastUpdateCheckTime = currentTime
        
        let networkManager = NetworkManager.sharedNetworkManager
        networkManager.makeDataRequest(url) { result in
            switch result {
            case .success(let data):
                do {
                    let updateInfo = try _parseUpdateData(data)
                    print("Fetched update info \(updateInfo)")
                    let hasUpdate = updateInfo.build > BuildVersion
                    DispatchQueue.main.async {
                        completion(hasUpdate, updateInfo.version)
                    }
                } catch {
                    print("Failed to parse update data with error: \(error)")
                }
            case .failure(let error):
                print("Failed to get update data with error: \(error)")
            }
        }
    }
    
    private static func _parseUpdateData(_ data: Data) throws -> UpdateInfo {
        guard let updateDictionary = try JSONSerialization.jsonObject(with: data) as? [String:Any] else {
            throw SimpleError("Failed to serialize JSON data")
        }
        guard let info = UpdateInfo(updateDictionary) else {
            throw SimpleError("Failed to parse update JSON data")
        }
        return info
    }
    
    static func updateURL() -> URL? {
        guard let url = URL(string: "https://***REMOVED***") else {
            print("Failed to construct update URL")
            return nil
        }
        return url
    }
}

fileprivate struct UpdateInfo: CustomStringConvertible {
    private static let BuildNumberKey = "CurrentBuild"
    private static let VersionStringKey = "CurrentVersionString"
    
    let build: Int
    let version: String
    
    init?(_ dictionary: [String: Any]) {
        guard let buildNum = dictionary[Self.BuildNumberKey] as? Int else {
            print("No build number found in update data")
            return nil
        }
        guard let versionString = dictionary[Self.VersionStringKey] as? String else {
            print("No version string found in update data")
            return nil
        }
        build = buildNum
        version = versionString
    }
    
    var description: String {
        return "Build: \(build) Version: \(version)"
    }
}
