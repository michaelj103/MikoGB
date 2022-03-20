//
//  SaveEntry.swift
//  MikoGB
//
//  Created by Michael Brandt on 3/9/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation

struct SaveEntry {
    let hashString: String
    let name: String
    
    private static let NameKey = "Name"
    
    init(hashString: String, _ dict: [String:String]) {
        let name = dict[SaveEntry.NameKey]!
        self.name = name
        self.hashString = hashString
    }
    
    init(hashString: String, name: String) {
        self.hashString = hashString
        self.name = name
    }
    
    
    var dictionaryRepresentation: [String : String] {
        get {
            let dict = [
                SaveEntry.NameKey : self.name
            ]
            return dict
        }
    }
}
