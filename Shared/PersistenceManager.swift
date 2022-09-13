//
//  PersistenceManager.swift
//  MikoGB
//
//  Created by Michael Brandt on 3/6/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import Foundation
import CryptoKit

class PersistenceManager {
    
    // Directory names
    private static let baseDirectory = "Saves"
    private static let activeSavesDirectory = "Active"
    private static let backupSavesDirectory = "Backup"
    private static let romsDirectory = "ROMs"
    
    private static var documentsPath: String = getDocumentsPath()
    private static func getDocumentsPath() -> String {
        let paths = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)
        return paths[0]
    }
    
    private static var savesPath: String = getSavesPath()
    private static func getSavesPath() -> String {
        let savesPath = NSString(string: documentsPath).appendingPathComponent(baseDirectory)
        return savesPath
    }
    
    private static var activeSavesPath: String = getActiveSavesPath()
    private static func getActiveSavesPath() -> String {
        let activeSavesPath = NSString(string: savesPath).appendingPathComponent(activeSavesDirectory)
        return activeSavesPath
    }
    
    private static var backupPath: String = getBackupPath()
    private static func getBackupPath() -> String {
        let backupSavesPath = NSString(string: savesPath).appendingPathComponent(backupSavesDirectory)
        return backupSavesPath
    }
    
    private static var romsPath: String = getROMsPath()
    private static func getROMsPath() -> String {
        let romsPath = NSString(string: documentsPath).appendingPathComponent(romsDirectory)
        return romsPath
    }
    
    private static func _createDirectoryIfNeeded(_ path: String) throws {
        let fileManager = FileManager.default
        var isDir: ObjCBool = false
        if fileManager.fileExists(atPath: path, isDirectory: &isDir) {
            if !isDir.boolValue {
                throw SimpleError("File exists at \(path) that is not a directory")
            }
            return
        }
        
        try fileManager.createDirectory(atPath: path, withIntermediateDirectories: true, attributes: nil)
    }
    
    private static func _prepareDirectoryTree() throws {
        try _createDirectoryIfNeeded(savesPath)
        try _createDirectoryIfNeeded(activeSavesPath)
        try _createDirectoryIfNeeded(backupPath)
        try _createDirectoryIfNeeded(romsPath)
    }
    
    static func prepare() throws {
        try _prepareDirectoryTree()
        _prepareUserDefaults()
    }
    
    static func installROMs() throws {
        let fm = FileManager.default
        let contents = try fm.contentsOfDirectory(atPath: documentsPath)
        for file in contents {
            let pathExtension = NSString(string: file).pathExtension
            if pathExtension == "gb" || pathExtension == "gbc" {
                let currentPath = NSString(string: documentsPath).appendingPathComponent(file)
                let targetPath = NSString(string: romsPath).appendingPathComponent(file)
                try fm.moveItem(atPath: currentPath, toPath: targetPath)
            }
        }
    }
    
    static func installROM(_ url: URL) throws {
        let fm = FileManager.default
        let sourcePath = url.path
        let filename = url.lastPathComponent
        let targetPath = NSString(string: romsPath).appendingPathComponent(filename)
        try fm.moveItem(atPath: sourcePath, toPath: targetPath)
    }
    
    private static func getHash(_ url: URL) throws -> String {
        let data = try Data(contentsOf: url, options: .mappedIfSafe)
        let digest = SHA256.hash(data: data)
        let string = digest.stringRepresentation()
        return string
    }
    
    // User default keys
    private static let SaveInfoRootKey = "SaveInfoRoot"
    
    private static func _prepareUserDefaults() {
        let rootDictionary = [String:Any]()
        UserDefaults.standard.register(defaults: [SaveInfoRootKey : rootDictionary])
    }
    
    private var saveEntries = [String:SaveEntry]()
    init() {
        // TODO: this should probably all throw alerts and not crash if the data is improperly formatted
        if let rootInfo = UserDefaults.standard.object(forKey: PersistenceManager.SaveInfoRootKey) as! Dictionary<String,Any>? {
            for (hashString, entryInfo) in rootInfo {
                let entryDict = entryInfo as! Dictionary<String,String>
                let entry = SaveEntry(hashString: hashString, entryDict)
                saveEntries[hashString] = entry
            }
        }
    }
    
    private func _writeEntryToDefaults(_ entry: SaveEntry) {
        let dict = entry.dictionaryRepresentation
        // TODO: failure to save should probably not crash
        var rootInfo = UserDefaults.standard.object(forKey: PersistenceManager.SaveInfoRootKey) as! Dictionary<String,Any>
        rootInfo[entry.hashString] = dict
        UserDefaults.standard.set(rootInfo, forKey: PersistenceManager.SaveInfoRootKey)
    }
    
    func loadSaveEntry(_ url: URL) throws -> SaveEntry {
        let hashString = try PersistenceManager.getHash(url)
        
        if let existingEntry = saveEntries[hashString] {
            return existingEntry
        } else {
            // create an entry and save it to defaults
            let targetName = NSString(string: url.lastPathComponent).deletingPathExtension
            for (_, entry) in saveEntries {
                // TODO: handle the fallback more cleanly
                precondition(entry.name != targetName, "A different ROM with the same filename has previously created save data")
            }
            
            let entry = SaveEntry(hashString: hashString, name: targetName)
            _writeEntryToDefaults(entry)
            return entry
        }
    }
    
    private func _activeSavePath(for entry: SaveEntry) -> String {
        let saveFileName = NSString(string: entry.name).appendingPathExtension("sav")!
        let saveFilePath = NSString(string: PersistenceManager.activeSavesPath).appendingPathComponent(saveFileName)
        return saveFilePath
    }
    
    private func _activeClockSavePath(for entry: SaveEntry) -> String {
        let saveFileName = NSString(string: entry.name).appendingPathExtension("clock")!
        let saveFilePath = NSString(string: PersistenceManager.activeSavesPath).appendingPathComponent(saveFileName)
        return saveFilePath
    }
    
    func loadSaveData(_ entry: SaveEntry) throws -> Data? {
        return try _loadData(entry)
    }
    
    func loadClockData(_ entry: SaveEntry) throws -> Data? {
        return try _loadData(entry, isClock: true)
    }
    
    private func _loadData(_ entry: SaveEntry, isClock: Bool = false) throws -> Data? {
        let fileManager = FileManager.default
        let saveFilePath = isClock ? _activeClockSavePath(for: entry) : _activeSavePath(for: entry)
        var isDir: ObjCBool = false
        if fileManager.fileExists(atPath: saveFilePath, isDirectory: &isDir) {
            if isDir.boolValue {
                throw SimpleError("A directory exists at the expected save file path \"\(saveFilePath)\"")
            } else {
                let url = URL(fileURLWithPath: saveFilePath)
                let data = try Data(contentsOf: url, options: [])
                return data
            }
        } else {
            return nil
        }
    }
    
    func writeSaveData(_ data: Data, for entry: SaveEntry) throws {
        let saveFilePath = _activeSavePath(for: entry)
        let url = URL(fileURLWithPath: saveFilePath)
        try data.write(to: url)
        print("Wrote save data to \"\(saveFilePath)\"")
    }
    
    func writeClockData(_ data: Data, for entry: SaveEntry) throws {
        let saveFilePath = _activeClockSavePath(for: entry)
        let url = URL(fileURLWithPath: saveFilePath)
        try data.write(to: url)
        print("Wrote clock data to \"\(saveFilePath)\"")
    }
    
    func saveURLForEntry(_ entry: SaveEntry) -> URL {
        let saveFilePath = _activeSavePath(for: entry)
        let url = URL(fileURLWithPath: saveFilePath)
        return url
    }
    
    // returns list of names, path
    func getROMs() throws -> [(String, String)] {
        let fm = FileManager.default
        let contents = try fm.contentsOfDirectory(atPath: PersistenceManager.romsPath)
        let basePath = NSString(string: PersistenceManager.romsPath)
        
        var romsInfo = [(String, String)]()
        for filename in contents {
            let baseName = NSString(string: filename).deletingPathExtension
            let fullPath = basePath.appendingPathComponent(filename)
            romsInfo.append((baseName, fullPath))
        }
        return romsInfo
    }
}

extension UInt8 {
    func hexString() -> String {
        let digits = "0123456789ABCDEF"
        let highNibble = Int((self & 0xF0) >> 4)
        let lowNibble = Int(self & 0x0F)
        
        let high = digits[digits.index(digits.startIndex, offsetBy: highNibble)]
        let low = digits[digits.index(digits.startIndex, offsetBy: lowNibble)]
        return String([high, low])
    }
}

extension SHA256.Digest {
    func stringRepresentation() -> String {
        var iterator = makeIterator()
        var stringRep = ""
        while let next = iterator.next() {
            stringRep += next.hexString()
        }
        return stringRep
    }
}
