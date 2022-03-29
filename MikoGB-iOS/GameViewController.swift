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
    
    private var dPadView: UIView!
    private var startSelectView: UIView!
    private var aButton: UIButton!
    private var bButton: UIButton!
    
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
        
        dPadView = UIView(frame: .zero)
        dPadView.backgroundColor = UIColor.blue
        self.view.addSubview(dPadView)
        startSelectView = UIView(frame: .zero)
        startSelectView.backgroundColor = UIColor.yellow
        self.view.addSubview(startSelectView)
        
        let aButtonUpImage = UIImage(named: "A_button_up", in: nil, with: nil)
        let aButtonDownImage = UIImage(named: "A_button_down", in: nil, with: nil)
        aButton = UIButton(frame: .zero)
        aButton.setImage(aButtonUpImage, for: .normal)
        aButton.setImage(aButtonDownImage, for: .highlighted)
        self.view.addSubview(aButton)
        let localEngine = self.engine
        let aButtonPushAction = UIAction { _ in
            localEngine.setKeyDown(.A)
        }
        let aButtonCancelAction = UIAction { _ in
            localEngine.setKeyUp(.A)
        }
        aButton.addAction(aButtonPushAction, for: .touchDown)
        aButton.addAction(aButtonCancelAction, for: .touchUpInside)
        aButton.addAction(aButtonCancelAction, for: .touchUpOutside)
        aButton.addAction(aButtonCancelAction, for: .touchDragExit)
        
        let bButtonUpImage = UIImage(named: "B_button_up", in: nil, with: nil)
        let bButtonDownImage = UIImage(named: "B_button_down", in: nil, with: nil)
        bButton = UIButton(frame: .zero)
        bButton.setImage(bButtonUpImage, for: .normal)
        bButton.setImage(bButtonDownImage, for: .highlighted)
        self.view.addSubview(bButton)
        let bButtonPushAction = UIAction { _ in
            localEngine.setKeyDown(.B)
        }
        let bButtonCancelAction = UIAction { _ in
            localEngine.setKeyUp(.B)
        }
        bButton.addAction(bButtonPushAction, for: .touchDown)
        bButton.addAction(bButtonCancelAction, for: .touchUpInside)
        bButton.addAction(bButtonCancelAction, for: .touchUpOutside)
        bButton.addAction(bButtonCancelAction, for: .touchDragExit)
        
        // TODO: themes
        // Teal GB
        self.view.backgroundColor = UIColor(red: 2.0/255.0, green: 183.0/255.0, blue: 212.0/255.0, alpha: 1.0)
        
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
            _layoutLandscape(availableBounds)
        } else {
            _layoutPortrait(availableBounds)
        }
    }
    
    private func _layoutLandscape(_ availableBounds: CGRect) {
        
    }
    
    private func _layoutPortrait(_ availableBounds: CGRect) {
        // screen
        let screenSize = gameView.sizeThatFits(availableBounds.size)
        let screenFrame = CGRect(x: availableBounds.origin.x, y: availableBounds.origin.y, width: screenSize.width, height: screenSize.height)
        gameView.frame = screenFrame
        
        // divide up the remaining space
        let totalArea = CGRect(x: availableBounds.minX, y: screenFrame.maxY, width: screenSize.width, height: availableBounds.height - screenSize.height)
        let topSection = CGRect(x: totalArea.minX, y: totalArea.minY, width: totalArea.size.width, height: totalArea.size.height * 0.75)
        let startSelectFrame = CGRect(x: topSection.minX, y: topSection.maxY, width: topSection.width, height: totalArea.size.height - topSection.height)
        let dPadFrame = CGRect(x: topSection.minX, y: topSection.minY, width: topSection.width * 0.5, height: topSection.height)
        let buttonsFrame = CGRect(x: dPadFrame.maxX, y: topSection.minY, width: topSection.width * 0.5, height: topSection.height)
        
        startSelectView.frame = startSelectFrame
        dPadView.frame = dPadFrame
        
        let buttonsTopHalf = CGRect(x: buttonsFrame.minX, y: buttonsFrame.minY, width: buttonsFrame.width, height: buttonsFrame.height / 2.0)
        let aButtonAspect = aButton.intrinsicContentSize.width / aButton.intrinsicContentSize.height
        let aButtonFrame = buttonsTopHalf.fitRect(aButtonAspect)
        aButton.frame = aButtonFrame
        
        let buttonsBottomHalf = CGRect(x: buttonsFrame.minX, y: buttonsTopHalf.maxY, width: buttonsFrame.width, height: buttonsFrame.height / 2.0)
        let bButtonAspect = bButton.intrinsicContentSize.width / bButton.intrinsicContentSize.height
        let bButtonFrame = buttonsBottomHalf.fitRect(bButtonAspect)
        bButton.frame = bButtonFrame
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
