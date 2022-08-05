//
//  UpdateManager.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 7/29/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation
import GBServerPayloads

class UpdateManager {
    private static let BuildVersion = 3
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
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/currentVersionInfo") else {
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
        
        let request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        let networkManager = NetworkManager.sharedNetworkManager
        networkManager.submitRequest(request) { result in
            switch result {
            case .success(let data):
                do {
                    let updateInfos = try JSONDecoder().decode([CurrentVersionHTTPResponsePayload].self, from: data)
                    print("Fetched update info \(updateInfos)")
                    let hasUpdate: Bool
                    let latestVersion: String
                    if let latestUpdateInfo = updateInfos.sorted(by: { $0.build > $1.build }).first {
                        hasUpdate = latestUpdateInfo.build > BuildVersion
                        latestVersion = latestUpdateInfo.versionName
                    } else {
                        hasUpdate = false
                        latestVersion = ""
                    }
                    DispatchQueue.main.async {
                        completion(hasUpdate, latestVersion)
                    }
                } catch {
                    print("Failed to parse update data with error: \(error)")
                }
            case .failure(let error):
                print("Failed to get update data with error: \(error)")
            }
        }
    }
    
    static func updateURL() -> URL? {
        guard let url = ServerConfiguration.createURL(resourcePath: "/") else {
            print("Failed to construct update URL")
            return nil
        }
        return url
    }
}

// MARK: - Network Packet Structures

extension CurrentVersionHTTPResponsePayload: CustomStringConvertible {
    public var description: String {
        return "Build: \(build) Version: \(versionName)"
    }
}


