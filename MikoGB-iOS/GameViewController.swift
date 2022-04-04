//
//  GameViewController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit
import UniformTypeIdentifiers

class GameViewController: UIViewController, DPadDelegate, GBEngineSaveDestination, UIDocumentPickerDelegate {

    private var gameView: GameView!
    private var engine = GBEngine()
    private let romURL: URL
    private let persistenceManager: PersistenceManager
    
    private var dPadView: DPadView!
    private var aButton: UIButton!
    private var bButton: UIButton!
    private var startButton: UIButton!
    private var selectButton: UIButton!
    
    init(_ rom: URL, persistenceManager: PersistenceManager) {
        self.romURL = rom
        self.persistenceManager = persistenceManager
        super.init(nibName: nil, bundle: nil)
        
        self.navigationItem.hidesBackButton = true
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        gameView = GameView(engine: engine)
        self.view.addSubview(gameView)
        
        dPadView = DPadView(frame: .zero)
        dPadView.delegate = self
        self.view.addSubview(dPadView)
        
        // for reference captures
        let localEngine = self.engine
        
        let startButtonUpImage = UIImage(named: "Start_up", in: nil, with: nil)
        let startButtonDownImage = UIImage(named: "Start_down", in: nil, with: nil)
        startButton = UIButton(frame: .zero)
        startButton.setImage(startButtonUpImage, for: .normal)
        startButton.setImage(startButtonDownImage, for: .highlighted)
        self.view.addSubview(startButton)
        let startButtonPushAction = UIAction { _ in
            localEngine.setKeyDown(.start)
        }
        let startButtonCancelAction = UIAction { _ in
            localEngine.setKeyUp(.start)
        }
        startButton.addAction(startButtonPushAction, for: .touchDown)
        startButton.addAction(startButtonCancelAction, for: .touchUpInside)
        startButton.addAction(startButtonCancelAction, for: .touchUpOutside)
        startButton.addAction(startButtonCancelAction, for: .touchDragExit)
        
        let selectButtonUpImage = UIImage(named: "Select_up", in: nil, with: nil)
        let selectButtonDownImage = UIImage(named: "Select_down", in: nil, with: nil)
        selectButton = UIButton(frame: .zero)
        selectButton.setImage(selectButtonUpImage, for: .normal)
        selectButton.setImage(selectButtonDownImage, for: .highlighted)
        self.view.addSubview(selectButton)
        let selectButtonPushAction = UIAction { _ in
            localEngine.setKeyDown(.select)
        }
        let selectButtonCancelAction = UIAction { _ in
            localEngine.setKeyUp(.select)
        }
        selectButton.addAction(selectButtonPushAction, for: .touchDown)
        selectButton.addAction(selectButtonCancelAction, for: .touchUpInside)
        selectButton.addAction(selectButtonCancelAction, for: .touchUpOutside)
        selectButton.addAction(selectButtonCancelAction, for: .touchDragExit)
        
        let aButtonUpImage = UIImage(named: "A_button_up", in: nil, with: nil)
        let aButtonDownImage = UIImage(named: "A_button_down", in: nil, with: nil)
        aButton = UIButton(frame: .zero)
        aButton.setImage(aButtonUpImage, for: .normal)
        aButton.setImage(aButtonDownImage, for: .highlighted)
        self.view.addSubview(aButton)
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

        _configureMenuButton()
        
        NotificationCenter.default.addObserver(forName: UIApplication.willTerminateNotification, object: nil, queue: nil) { [weak self] _ in
            self?._persistSaveDataImmediatelyIfNeeded()
        }
        
        NotificationCenter.default.addObserver(forName: UIApplication.didEnterBackgroundNotification, object: nil, queue: nil) { [weak self] _ in
            self?._persistSaveDataImmediatelyIfNeeded()
        }
        
        _loadROM()
    }
    
    private func _loadROM() {
        let url = romURL
        engine.saveDestination = self
        engine.loadROM(url) { [weak self] success, supportsSaves in
            self?._romDidLoad(url, success: success, supportsSaveData: supportsSaves)
        }
    }
    
    private func _restart() {
        engine = GBEngine()
        gameView.engine = engine
        _loadROM()
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        let action = UIAction { [weak self] _ in
            self?._persistSaveDataImmediatelyIfNeeded()
            self?.navigationController?.popViewController(animated: true)
        }
        
        let powerImage = UIImage(systemName: "power")
        let powerButton = UIBarButtonItem(title: nil, image: powerImage, primaryAction: action, menu: nil)
        powerButton.tintColor = .white
        self.navigationItem.leftBarButtonItem = powerButton
    }
    
    private func _configureMenuButton() {
        //TODO: only include save actions if there's a save entry
        let exportSaveItem = UIAction(title: "Export Save", image: nil, identifier: nil, discoverabilityTitle: nil, attributes: [], state: .off) { [weak self] _ in
            self?._exportSave()
        }
        let importSaveItem = UIAction(title: "Import Save", image: nil, identifier: nil, discoverabilityTitle: nil, attributes: [], state: .off) { [weak self] _ in
            self?._importSave()
        }
        let menu = UIMenu(title: "", image: nil, identifier: nil, options: .displayInline, children: [
        exportSaveItem, importSaveItem])
        
        let buttonImage = UIImage(systemName: "ellipsis.circle")
        let menuButton = UIBarButtonItem(title: nil, image: buttonImage, primaryAction: nil, menu: menu)
        menuButton.tintColor = .white
        navigationItem.rightBarButtonItems = [menuButton]
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
        
        dPadView.frame = dPadFrame.fitRect(1.0)
        
        let buttonsTopHalf = CGRect(x: buttonsFrame.minX, y: buttonsFrame.minY, width: buttonsFrame.width, height: buttonsFrame.height / 2.0)
        let aButtonAspect = aButton.intrinsicContentSize.width / aButton.intrinsicContentSize.height
        let aButtonFrame = buttonsTopHalf.fitRect(aButtonAspect)
        aButton.frame = aButtonFrame
        
        let buttonsBottomHalf = CGRect(x: buttonsFrame.minX, y: buttonsTopHalf.maxY, width: buttonsFrame.width, height: buttonsFrame.height / 2.0)
        let bButtonAspect = bButton.intrinsicContentSize.width / bButton.intrinsicContentSize.height
        let bButtonFrame = buttonsBottomHalf.fitRect(bButtonAspect)
        bButton.frame = bButtonFrame
        
        let selectRegion = CGRect(x: startSelectFrame.minX, y: startSelectFrame.minY, width: startSelectFrame.width / 2.0, height: startSelectFrame.height)
        let selectAspect = selectButton.intrinsicContentSize.width / selectButton.intrinsicContentSize.height
        let selectButtonFrame = selectRegion.fitRect(selectAspect)
        selectButton.frame = selectButtonFrame
        
        let startRegion = CGRect(x: selectRegion.maxX, y: startSelectFrame.minY, width: startSelectFrame.width / 2.0, height: startSelectFrame.height)
        let startAspect = startButton.intrinsicContentSize.width / startButton.intrinsicContentSize.height
        let startButtonFrame = startRegion.fitRect(startAspect)
        startButton.frame = startButtonFrame
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
    
    func engineIsReady(toPersistSaveData engine: GBEngine) {
        engine.getSaveData { (data) in
            // capture strong reference
            self._persistSaveData(data)
        }
    }
    
    private func _persistSaveData(_ data: Data?) {
        guard let data = data, let entry = loadedSaveDataEntry else {
            return
        }
        
        // TODO: catch errors and handle gracefully
        try! persistenceManager.writeSaveData(data, for: entry)
    }
    
    private func _persistSaveDataImmediatelyIfNeeded() {
        if engine.isSaveDataStale {
            print("Writing stale save data immediately")
            let data = engine.synchronousGetSaveData()
            _persistSaveData(data)
            engine.staleSaveDataHandled()
        }
    }
    
    private func _mapDPadToKey(_ direction: DPadDirection) -> GBEngineKeyCode? {
        switch direction {
        case .Neutral:
            return nil
        case .Up:
            return .up
        case .Down:
            return .down
        case .Left:
            return .left
        case .Right:
            return .right
        }
    }
    
    func directionChanged(_ newDirection: DPadDirection, oldDirection: DPadDirection) {
        if let oldKey = _mapDPadToKey(oldDirection) {
            engine.setKeyUp(oldKey)
        }
        if let newKey = _mapDPadToKey(newDirection) {
            engine.setKeyDown(newKey)
        }
    }
    
    // MARK: - Actions
    
    private func _exportSave() {
        guard let loadedSaveDataEntry = loadedSaveDataEntry else {
            return
        }

        let url = persistenceManager.saveURLForEntry(loadedSaveDataEntry)
        let fm = FileManager.default
        if !fm.fileExists(atPath: url.path) {
            let alert = UIAlertController(title: "No Save File", message: "There's currently no save file for this ROM", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default))
            self.present(alert, animated: true)
            return
        }
        
        var shouldResume = false
        let gameView = gameView!
        if gameView.state == .Playing {
            shouldResume = true
            gameView.pause()
        }
        let activityVC = UIActivityViewController(activityItems: [url], applicationActivities: nil)
        activityVC.completionWithItemsHandler = { _, _, _, _ in
            if shouldResume {
                gameView.start()
            }
        }
        self.present(activityVC, animated: true)
    }
    
    private var isImporting = false
    private var shouldResumePostImport = false
    private func _importSave() {
        if loadedSaveDataEntry == nil {
            return
        }
        
        shouldResumePostImport = false
        let gameView = gameView!
        if gameView.state == .Playing {
            shouldResumePostImport = true
            gameView.pause()
        }
        
        let alert = UIAlertController(title: "Warning", message: "Importing a save will overwrite any existing save", preferredStyle: .alert)
        let continueAction = UIAlertAction(title: "Continue", style: .default) { [weak self] _ in
            self?._actuallyImportSave()
        }
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel) { [weak self] _ in
            self?._cancelImportSave()
        }
        alert.addAction(continueAction)
        alert.addAction(cancelAction)
        self.present(alert, animated: true)
    }
    
    private func _actuallyImportSave() {
//        let documentPicker: UIDocumentPickerViewController
//        if #available(iOS 14, *) {
//            documentPicker = UIDocumentPickerViewController(forOpeningContentTypes: [])
//        } else {
//            documentPicker = UIDocumentPickerViewController(documentTypes: [], in: .import)
//        }
        // TODO: the one we're supposed to use now doesn't compile. This API may not get much love
        let documentPicker = UIDocumentPickerViewController(documentTypes: [UTType.data.identifier], in: .import)
        documentPicker.delegate = self
        documentPicker.shouldShowFileExtensions = true
        documentPicker.allowsMultipleSelection = false
        self.present(documentPicker, animated: true)
    }
    
    private func _cancelImportSave() {
        if shouldResumePostImport {
            gameView.start()
        }
    }
    
    func documentPickerWasCancelled(_ controller: UIDocumentPickerViewController) {
        _cancelImportSave()
    }
    
    func documentPicker(_ controller: UIDocumentPickerViewController, didPickDocumentAt url: URL) {
        guard let loadedSaveDataEntry = loadedSaveDataEntry else {
            preconditionFailure("no loaded save entry on picking a save to import - this should be unreachable")
        }
        let isSecureURL = url.startAccessingSecurityScopedResource()
        let coordinator = NSFileCoordinator()
        var error: NSError? = nil
        //TODO: handle these failable calls gracefully
        var data: Data?
        coordinator.coordinate(readingItemAt: url, error: &error) { accessedURL in
            data = try! Data(contentsOf: accessedURL)
        }
        
        if isSecureURL {
            url.stopAccessingSecurityScopedResource()
        }
        try! persistenceManager.writeSaveData(data!, for: loadedSaveDataEntry)
        _restart()
        
//        if url.startAccessingSecurityScopedResource() {
//            //TODO: handle these failable calls gracefully
//            let data = try! Data(contentsOf: url)
//            try! persistenceManager.writeSaveData(data, for: loadedSaveDataEntry)
//            url.stopAccessingSecurityScopedResource()
//            _restart()
//        } else {
//            let alert = UIAlertController(title: "Failed to Load", message: "Accessing secure document failed", preferredStyle: .alert)
//            alert.addAction(UIAlertAction(title: "OK", style: .default))
//            self.present(alert, animated: true)
//            _cancelImportSave()
//        }
    }

}