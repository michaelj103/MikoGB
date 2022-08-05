//
//  ServerConfiguration-template.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 8/5/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

internal struct ServerConfiguration {
    private static let URLBase = URL(string: "https://www.example.com")!
    
    static func createURL(resourcePath: String) -> URL? {
        return URL(string: resourcePath, relativeTo: ServerConfiguration.URLBase)
    }
}
