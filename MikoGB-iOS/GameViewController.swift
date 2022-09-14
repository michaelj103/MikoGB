//
//  GameViewController.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit
import UniformTypeIdentifiers
import AVFoundation

class GameViewController: UIViewController, DPadDelegate, GBEngineSaveDestination, UIDocumentPickerDelegate {

    private var gameView: GameView!
    private var engine: GBEngine
    private var audioController: AudioController!
    private let romURL: URL
    private var romFeatures: ROMFeatureSupport?
    private let persistenceManager: PersistenceManager
    private var linkSessionManager: LinkSessionManager
    
    private var dPadView: DPadView!
    private var aButton: UIButton!
    private var bButton: UIButton!
    private var startButton: UIButton!
    private var selectButton: UIButton!
    private var fastForwardButton: UIBarButtonItem!
    
    private var terminationObserver: NSObjectProtocol?
    private var backgroundObserver: NSObjectProtocol?
    private var settingsObserver: NSObjectProtocol?
    private var connectionObserver: NSObjectProtocol?
    
    private var desiredAudioState: DesiredAudioState = .stopped {
        didSet {
            _updateAudioEngine()
        }
    }
    private enum DesiredAudioState {
        case stopped
        case playing
    }
    
    init(_ rom: URL, persistenceManager: PersistenceManager) {
        self.romURL = rom
        self.persistenceManager = persistenceManager
        let engine = GBEngine()
        self.engine = engine
        self.linkSessionManager = LinkSessionManager(engine)
        super.init(nibName: nil, bundle: nil)
        
        self.navigationItem.hidesBackButton = true
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        _updateAudioSession()
        
        gameView = GameView(engine: engine)
        audioController = AudioController(engine: engine)
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

        let menuButton = _createMenuButton()
        let fastForwardImage = UIImage(systemName: "forward")
        let ffAction = UIAction { [weak self] _ in
            self?._toggleSpeedMode()
        }
        fastForwardButton = UIBarButtonItem(title: nil, image: fastForwardImage, primaryAction: ffAction, menu: nil)
        fastForwardButton.tintColor = .white
        navigationItem.rightBarButtonItems = [menuButton, fastForwardButton]
        
        terminationObserver = NotificationCenter.default.addObserver(forName: UIApplication.willTerminateNotification, object: nil, queue: nil) { [weak self] _ in
            self?._persistSaveDataImmediatelyIfNeeded()
        }
        
        backgroundObserver = NotificationCenter.default.addObserver(forName: UIApplication.didEnterBackgroundNotification, object: nil, queue: nil) { [weak self] _ in
            self?._persistSaveDataImmediatelyIfNeeded()
        }
        
        settingsObserver = NotificationCenter.default.addObserver(forName: SettingsManager.SettingsChangedNotification, object: nil, queue: nil) { [weak self] _ in
            self?._settingsChanged()
        }
        
        connectionObserver = NotificationCenter.default.addObserver(forName: LinkSessionManager.HasConnectionNotification, object: nil, queue: OperationQueue.main, using: { [weak self] _ in
            self?._updateSpeedModeForConnection()
        })
        
        _loadROM()
    }
    
    deinit {
        if let terminationObserver = terminationObserver {
            NotificationCenter.default.removeObserver(terminationObserver)
        }
        if let backgroundObserver = backgroundObserver {
            NotificationCenter.default.removeObserver(backgroundObserver)
        }
        if let settingsObserver = settingsObserver {
            NotificationCenter.default.removeObserver(settingsObserver)
        }
        if let connectionObserver = connectionObserver {
            NotificationCenter.default.removeObserver(connectionObserver)
        }
    }
    
    private func _loadROM() {
        let url = romURL
        engine.saveDestination = self
        engine.loadROM(url) { [weak self] success, featureSupport in
            self?._romDidLoad(url, success: success, featureSupport: featureSupport)
        }
    }
    
    private func _restart() {
        if self.presentedViewController != nil {
            // in case we have a link session VC up
            self.dismiss(animated: true)
        }
        engine = GBEngine()
        linkSessionManager = LinkSessionManager(engine)
        gameView.engine = engine
        desiredAudioState = .stopped
        audioController = AudioController(engine: engine)
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
    
    private func _createMenuButton() -> UIBarButtonItem {
        //TODO: only include save actions if there's a save entry
        let exportSaveItem = UIAction(title: "Export Save", image: nil, identifier: nil, discoverabilityTitle: nil, attributes: [], state: .off) { [weak self] _ in
            self?._exportSave()
        }
        let importSaveItem = UIAction(title: "Import Save", image: nil, identifier: nil, discoverabilityTitle: nil, attributes: [], state: .off) { [weak self] _ in
            self?._importSave()
        }
        let settingsImage = UIImage(systemName: "gearshape")
        let showSettingsItem = UIAction(title: "Settings", image: settingsImage, identifier: nil, discoverabilityTitle: nil, attributes: [], state: .off) { [weak self] _ in
            self?._showSettings()
        }
        let linkSessionImage = UIImage(systemName: "cable.connector.horizontal")
        let linkSessionItem = UIAction(title: "Link Session", image: linkSessionImage, identifier: nil, discoverabilityTitle: nil, attributes: [], state: .off) { [weak self] _ in
            self?._showLinkSessionManager()
        }
        let menu = UIMenu(title: "", image: nil, identifier: nil, options: .displayInline, children: [
        showSettingsItem, linkSessionItem, exportSaveItem, importSaveItem])
        
        let buttonImage = UIImage(systemName: "ellipsis.circle")
        let menuButton = UIBarButtonItem(title: nil, image: buttonImage, primaryAction: nil, menu: menu)
        menuButton.tintColor = .white
        return menuButton
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
        let screenSize = gameView.sizeThatFits(availableBounds.size)
        let midX = availableBounds.origin.x + ((availableBounds.size.width - screenSize.width) / 2.0)
        let screenFrame = CGRect(x: midX, y: availableBounds.origin.y, width: screenSize.width, height: screenSize.height)
        gameView.frame = screenFrame
        
        let leftArea = CGRect(x: availableBounds.minX, y: availableBounds.minY, width: screenFrame.minX - availableBounds.minX, height: availableBounds.height)
        let leftTopArea = CGRect(x: leftArea.minX, y: leftArea.minY, width: leftArea.width, height: leftArea.height * 0.75)
        let leftBottomArea = CGRect(x: leftArea.minX, y: leftTopArea.maxY, width: leftArea.width, height: leftArea.height - leftTopArea.height)
        
        let dPadFrame = leftTopArea.fitRect(1.0)
        dPadView.frame = dPadFrame
        
        let selectAspect = selectButton.intrinsicContentSize.width / selectButton.intrinsicContentSize.height
        let selectButtonFrame = leftBottomArea.fitRect(selectAspect)
        selectButton.frame = selectButtonFrame
        
        let rightArea = CGRect(x: screenFrame.maxX, y: availableBounds.minY, width: availableBounds.maxX - screenFrame.maxX, height: availableBounds.height)
        let rightTopArea = CGRect(x: rightArea.minX, y: rightArea.minY, width: rightArea.width, height: rightArea.height * 0.75)
        let rightBottomArea = CGRect(x: rightArea.minX, y: rightTopArea.maxY, width: rightArea.width, height: rightArea.height - rightTopArea.height)
        
        let buttonsTopHalf = CGRect(x: rightTopArea.minX, y: rightTopArea.minY, width: rightTopArea.width, height: rightTopArea.height * 0.5)
        let aButtonAspect = aButton.intrinsicContentSize.width / aButton.intrinsicContentSize.height
        let aButtonFrame = buttonsTopHalf.fitRect(aButtonAspect)
        aButton.frame = aButtonFrame
        
        let buttonsBottomHalf = CGRect(x: rightTopArea.minX, y: buttonsTopHalf.maxY, width: rightTopArea.width, height: rightTopArea.height * 0.5)
        let bButtonAspect = bButton.intrinsicContentSize.width / bButton.intrinsicContentSize.height
        let bButtonFrame = buttonsBottomHalf.fitRect(bButtonAspect)
        bButton.frame = bButtonFrame
        
        let startAspect = startButton.intrinsicContentSize.width / startButton.intrinsicContentSize.height
        let startButtonFrame = rightBottomArea.fitRect(startAspect)
        startButton.frame = startButtonFrame
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
    
    private func _updateAudioEngine() {
        let shouldPlay: Bool
        let allowAudio = SettingsManager.sharedInstance.shouldGenerateAudio
        switch desiredAudioState {
        case .stopped:
            shouldPlay = false
        case .playing:
            shouldPlay = allowAudio
        }
        
        if shouldPlay && !audioController.isPlaying {
            audioController.startAudioEngine()
        } else if !shouldPlay && audioController.isPlaying {
            audioController.stopAudioEngine()
        }
    }
    
    private func _updateAudioSession() {
        // Set audio category to play when in silent mode
        let shouldRespectMute = SettingsManager.sharedInstance.shouldRespectMuteSwitch
        let category = shouldRespectMute ? AVAudioSession.Category.ambient : AVAudioSession.Category.playback
        do {
            try AVAudioSession.sharedInstance().setCategory(category)
        } catch {
            print("Failed to set audio category with error \(error)")
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
    private func _romDidLoad(_ url: URL, success: Bool, featureSupport: ROMFeatureSupport) {
        if success {
            self.romFeatures = featureSupport
            if featureSupport.supportsSave.boolValue {
                _loadSaveData(url) { _saveDataDidLoad($0) }
            } else {
                loadedSaveDataEntry = nil
                _startEmulation()
            }
        } else {
            let alert = UIAlertController(title: "Failed to Load ROM", message: "Either because the file failed to load or the MBC (chip on cartridge) is unsupported", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { [weak self] _ in
                self?.navigationController?.popViewController(animated: true)
            }))
            self.present(alert, animated: true)
        }
    }
    
    private func _saveDataDidLoad(_ success: Bool) {
        if success {
            if self.romFeatures!.supportsTimer.boolValue {
                _loadClockData { _clockDataDidLoad($0) }
            } else {
                _startEmulation()
            }
        } else {
            let alert = UIAlertController(title: "Failed to Load Save Data", message: "Unable to load save data from the file. It may not be valid for this ROM", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { [weak self] _ in
                self?.navigationController?.popViewController(animated: true)
            }))
            self.present(alert, animated: true)
        }
    }
    
    private func _loadClockData(completion: (Bool) -> Void) {
        guard let saveEntry = loadedSaveDataEntry else {
            // no data is a valid case (e.g. starting for the first time)
            completion(true)
            return
        }
        
        if let data = try! persistenceManager.loadClockData(saveEntry) {
            engine.loadClockData(data) { [weak self] success in
                self?._clockDataDidLoad(success)
            }
        } else {
            // no data is a valid case (e.g. starting for the first time)
            completion(true)
        }
    }
    
    private func _clockDataDidLoad(_ success: Bool) {
        if success {
            _startEmulation()
        } else {
            let alert = UIAlertController(title: "Failed to Load Clock Data", message: "Unable to load clock data from the file", preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { [weak self] _ in
                self?.navigationController?.popViewController(animated: true)
            }))
            self.present(alert, animated: true)
        }
    }
    
    private func _startEmulation() {
        gameView.start()
        desiredAudioState = .playing
    }
    
    func engineIsReady(toPersistSaveData engine: GBEngine) {
        engine.getSaveData { (data) in
            // capture strong reference
            self._persistSaveData(data)
        }
    }
    
    func engineIsReady(toPersistClockData engine: GBEngine) {
        engine.getClockData { (data) in
            // capture strong reference
            self._persistClockData(data)
        }
    }
    
    private func _persistSaveData(_ data: Data?) {
        guard let data = data, let entry = loadedSaveDataEntry else {
            return
        }
        
        // TODO: catch errors and handle gracefully
        try! persistenceManager.writeSaveData(data, for: entry)
    }
    
    private func _persistClockData(_ data: Data?) {
        guard let data = data, let entry = loadedSaveDataEntry else {
            return
        }
        
        // TODO: catch errors and handle gracefully
        try! persistenceManager.writeClockData(data, for: entry)
    }
    
    private func _persistSaveDataImmediatelyIfNeeded() {
        if engine.isSaveDataStale {
            print("Writing stale save data immediately")
            let data = engine.synchronousGetSaveData()
            _persistSaveData(data)
            engine.staleSaveDataHandled()
        }
        if engine.isClockDataStale {
            print("Writing stale clock data on quit")
            let clockData = engine.synchronousGetClockData()
            _persistClockData(clockData)
            engine.staleClockDataHandled()
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
    
    private func _settingsChanged() {
        _updateAudioEngine()
        _updateAudioSession()
    }
    
    // MARK: - Actions
    
    private func _toggleSpeedMode() {
        if linkSessionManager.hasConnection {
            print("Can't toggle speed mode with an active connection")
            return
        }
        gameView.toggleSpeedMode()
        _updateSpeedModeButton()
    }
    
    private func _updateSpeedModeButton() {
        let image: UIImage?
        if gameView.isInSpeedMode {
            image = UIImage(systemName: "forward.fill")
        } else {
            image = UIImage(systemName: "forward")
        }
        fastForwardButton.image = image
    }
    
    private func _updateSpeedModeForConnection() {
        if linkSessionManager.hasConnection {
            if gameView.isInSpeedMode {
                gameView.toggleSpeedMode()
                let alert = UIAlertController(title: "Speed Mode Disabled", message: "Speed mode isn't available while a link session is active", preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: "OK", style: .default))
                self.presentOverPresentedViewController(alert, animated: true)
            }
            
            fastForwardButton.isEnabled = false
        } else {
            fastForwardButton.isEnabled = true
        }
        _updateSpeedModeButton()
    }
    
    private func _exportSave() {
        guard let loadedSaveDataEntry = loadedSaveDataEntry else {
            return
        }
        
        _persistSaveDataImmediatelyIfNeeded()
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
        let types: [UTType]
        if let type = UTType(filenameExtension: "sav") {
            types = [type]
        } else {
            types = []
        }
        let documentPicker = UIDocumentPickerViewController(forOpeningContentTypes: types)
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
    
    private func _showSettings() {
        SettingsTableViewController.presentModal(on: self)
    }
    
    private func _showLinkSessionManager() {
        LinkSessionViewController.presentModal(with: self.linkSessionManager, on: self)
    }
    
    // MARK: UIDocumentPickerDelegate
    
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

extension UIViewController {
    func presentOverPresentedViewController(_ viewController: UIViewController, animated: Bool) {
        if let presented = self.presentedViewController {
            presented.presentOverPresentedViewController(viewController, animated: animated)
        } else {
            self.present(viewController, animated: animated)
        }
    }
}
