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
    private static let BuildVersion = 10
    private static let BuildVersionKey = "BuildVersion"
    private static let CurrentVersion = "v0.10"
    
    static func getCurrentVersionAndBuild() -> (String, Int) {
        return (CurrentVersion, BuildVersion)
    }
    
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
    
    static func checkForUpdate(_ completion: @escaping (Result<(Bool, String), Error>) -> Void) {
        checkForUpdate(type: .release, force: false, completion)
    }
    
    static func checkForUpdate(type: UpdateType, force: Bool, _ completion: @escaping (Result<(Bool, String), Error>) -> Void) {
        let queryItems = [URLQueryItem(name: "requestedType", value: "\(type.numberValue)")]
        guard let url = ServerConfiguration.createURL(resourcePath: "/api/currentVersionInfo", queryItems: queryItems) else {
            print("Failed to construct update check URL")
            return
        }
        let currentTime = Date()
        if let lastCheckTime = LastUpdateCheckTime {
            let timeSinceLast = currentTime.timeIntervalSince(lastCheckTime)
            if !force && timeSinceLast < 3600.0 {
                // Don't check all the time, just once per hour
                print("Skipping update check, it's only been \(timeSinceLast) seconds")
                return
            }
        }
        
        LastUpdateCheckTime = currentTime
        
        let request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData)
        let networkManager = NetworkManager.sharedNetworkManager
        networkManager.submitRequest(request) { result in
            let finalResult: Result<(Bool, String), Error>
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
                    finalResult = .success((hasUpdate, latestVersion))
                } catch {
                    print("Failed to parse update data with error: \(error)")
                    finalResult = .failure(error)
                }
            case .failure(let error):
                print("Failed to get update data with error: \(error)")
                finalResult = .failure(error)
            }
            
            DispatchQueue.main.async {
                completion(finalResult)
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
    
    enum UpdateType {
        case release, staging
        var numberValue: Int64 {
            get {
                switch self {
                case .release:
                    return VersionType.current.rawValue
                case .staging:
                    return VersionType.staging.rawValue
                }
            }
        }
    }
}

// MARK: - Network Packet Structures

extension CurrentVersionHTTPResponsePayload: CustomStringConvertible {
    public var description: String {
        return "Build: \(build) Version: \(versionName)"
    }
}


