//
//  ServerConfiguration-template.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/5/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

// Add this to ServerConfiguration.swift and don't commit
//internal struct ServerConfiguration {
//    public static let Hostname = URL(string: "https://www.example.com")!
//    public static let APIKey = "<KeyHere>"
//}

extension ServerConfiguration {
    static let ServerConfigurationChangedNotification = NSNotification.Name(rawValue: "ServerConfigurationChangedNotification")
    
    static func createURL(resourcePath: String, queryItems: [URLQueryItem]? = nil) -> URL? {
        let (host, port, scheme) = _getHostAndPortAndScheme()
        return createURL(hostname: host, port: port, scheme: scheme, resourcePath: resourcePath, queryItems: queryItems)
    }
    
    static func createURL(hostname: String, port: Int?, scheme: String, resourcePath: String, queryItems: [URLQueryItem]? = nil) -> URL? {
        var components = URLComponents()
        components.host = hostname
        components.port = port
        components.scheme = scheme
        components.path = resourcePath
        components.queryItems = queryItems
        return components.url
    }
    
    private static var temporaryHostOverride: URL?
    static func setTemporaryHost(_ hostname: String) -> Bool {
        if let url = URL(string: hostname), url.host != nil {
            temporaryHostOverride = url
            NotificationCenter.default.post(name: ServerConfigurationChangedNotification, object: nil)
            return true
        } else {
            // try to prepend http://
            if let url = URL(string: "http://\(hostname)"), url.host != nil {
                temporaryHostOverride = url
                NotificationCenter.default.post(name: ServerConfigurationChangedNotification, object: nil)
                return true
            }
        }
        return false
    }
    
    static func stopTemporaryOverride() {
        if temporaryHostOverride != nil {
            temporaryHostOverride = nil
            NotificationCenter.default.post(name: ServerConfigurationChangedNotification, object: nil)
        }
    }
    
    static func hasHostOverride() -> Bool {
        return temporaryHostOverride != nil
    }
    
    private static func _getHostAndPortAndScheme() -> (String, Int?, String) {
        if let temporaryHostOverride = temporaryHostOverride {
            return (temporaryHostOverride.host!, temporaryHostOverride.port, temporaryHostOverride.scheme ?? "http")
        } else {
            return (ServerConfiguration.Hostname, nil, "https")
        }
    }
}
