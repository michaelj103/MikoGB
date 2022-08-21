//
//  ServerConfiguration-template.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/5/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

// Add this to ServerConfiguration.swift and don't commit
//internal struct ServerConfiguration {
//    public static let Hostname = URL(string: "https://www.example.com")!
//    public static let APIKey = "<KeyHere>"
//}

extension ServerConfiguration {
    static func createURL(resourcePath: String, queryItems: [URLQueryItem]? = nil) -> URL? {
        return createURL(hostname: Hostname, port: nil, scheme: "https", resourcePath: resourcePath, queryItems: queryItems)
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
}
