//
//  GameViewController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

class GameViewController: UIViewController {

    private var gameView: GameView!
    private let engine = GBEngine()
    private let romURL: URL
    private let persistenceManager: PersistenceManager
    
    init(_ rom: URL, persistenceManager: PersistenceManager) {
        self.romURL = rom
        self.persistenceManager = persistenceManager
        super.init(nibName: nil, bundle: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        gameView = GameView(engine: engine)
        self.view.addSubview(gameView)
        
        let url = romURL
        engine.loadROM(url) { [weak self] success, supportsSaves in
            self?._romDidLoad(url, success: success, supportsSaveData: supportsSaves)
        }
    }
    
    override func viewWillLayoutSubviews() {
        super.viewWillLayoutSubviews()
        
        let availableBounds = self.view.bounds.inset(by: self.view.safeAreaInsets)
        let isLandscape = availableBounds.size.width > availableBounds.size.height
        
        if isLandscape {
            
        } else {
            let size = gameView.sizeThatFits(availableBounds.size)
            let frame = CGRect(x: availableBounds.origin.x, y: availableBounds.origin.y, width: size.width, height: size.height)
            gameView.frame = frame
        }
    }
    
    private func _loadSaveData(_ url: URL, completion: (Bool)->()) {
        // TODO: MJB: appropriate fallbacks instead of crash
        let entry = try! persistenceManager.loadSaveEntry(url)
        loadedSaveDataEntry = entry
        if let data = try! persistenceManager.loadSaveData(entry) {
            // now, do something with the data
            engine.loadSave(data) { [weak self] (success) in
                self?._saveDataDidLoad(success)
            }
        } else {
            // there's no data, which is a valid case (e.g. starting a game for the first time or after deleting save data)
            completion(true)
        }
    }
    
    private var loadedSaveDataEntry: SaveEntry?
    private func _romDidLoad(_ url: URL, success: Bool, supportsSaveData: Bool) {
        if success {
            if supportsSaveData {
                _loadSaveData(url) { _saveDataDidLoad($0) }
            } else {
                loadedSaveDataEntry = nil
                _startEmulation()
            }
        } else {
            //TODO: present an alert
            preconditionFailure("failure to load ROM data is unhandled")
        }
    }
    
    private func _saveDataDidLoad(_ success: Bool) {
        if success {
            _startEmulation()
        } else {
            //TODO: present an alert
            preconditionFailure("failure to load save data is unhandled")
        }
    }
    
    private func _startEmulation() {
        gameView.start()
//        audioController.startAudioEngine()
    }

}
