//
//  ServerConfiguration-template.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/5/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

internal struct ServerConfiguration {
    public static let Hostname = URL(string: "https://www.example.com")!
    public static let APIKey = "<KeyHere>"
    
    static func createURL(resourcePath: String, queryItems: [URLQueryItem]? = nil) -> URL? {
        var components = URLComponents()
        components.host = Hostname
        components.scheme = "https"
        components.path = resourcePath
        components.queryItems = queryItems
        return components.url
    }
}
