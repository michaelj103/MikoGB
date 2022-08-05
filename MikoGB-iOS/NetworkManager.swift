//
//  NetworkManager.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 7/29/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

class NetworkManager {
    private let urlSession: URLSession
    static var sharedNetworkManager: NetworkManager = {
        let networkManager = NetworkManager()
        return networkManager
    }()
    
    init() {
        let config = URLSessionConfiguration.ephemeral
        urlSession = URLSession(configuration: config)
    }
    
    func makeDataRequest(_ url: URL, completion: @escaping (Result<Data, Swift.Error>)->()) {
        let request = URLRequest(url: url, cachePolicy: .reloadIgnoringLocalCacheData, timeoutInterval: 60.0)
        submitRequest(request, completion: completion)
    }
    
    func submitRequest(_ request: URLRequest, completion: @escaping (Result<Data, Swift.Error>)->()) {
        let dataTask = urlSession.dataTask(with: request) { data, response, error in
            let result: Result<Data, Swift.Error>
            if let data = data {
                if let response = response as? HTTPURLResponse {
                    let status = response.statusCode
                    if status == 200 {
                        result = .success(data)
                    } else {
                        let error = NetworkManager.Error("Failed with status code \(status)")
                        result = .failure(error)
                    }
                } else {
                    let error = NetworkManager.Error("Unable to parse response")
                    result = .failure(error)
                }
            } else if let error = error {
                result = .failure(error)
            } else {
                let unknownError = NetworkManager.Error("An unknown error occurred")
                result = .failure(unknownError)
            }
            
            completion(result)
        }
        dataTask.resume()
    }
    
    struct Error : Swift.Error, CustomStringConvertible {
        let description: String
        init(_ description: String) {
            self.description = description
        }
    }
}
