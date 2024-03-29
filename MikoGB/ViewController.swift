//
//  ViewController.swift
//  MikoGB
//
//  Created on 5/4/21.
//

import Cocoa

class ViewController: NSViewController, NoROMViewDelegate, NSMenuItemValidation, GBEngineSaveDestination {

    private var gameView: GameView?
    private var contentView: NSView? {
        didSet {
            oldValue?.removeFromSuperview()
            if let cv = contentView {
                cv.frame = self.view.bounds;
                self.view.addSubview(cv)
            }
        }
    }
    
    private var engine: GBEngine!
    private var audioController: AudioController!
    private var persistenceManager: PersistenceManager!
    private var linkManager: LinkSessionManager!
    private var romFeatures: ROMFeatureSupport?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let emptyView = NoROMView()
        emptyView.delegate = self
        contentView = emptyView
        self.view.addSubview(emptyView)
        
        engine = AppStateManager.sharedInstance.engine
        audioController = AudioController(engine: engine)
        persistenceManager = AppStateManager.sharedInstance.persistenceManager
        linkManager = AppStateManager.sharedInstance.linkSessionManager
        engine.saveDestination = self
        
        NotificationCenter.default.addObserver(forName: TerminationNotification, object: nil, queue: nil) { [weak self] _ in
            self?._prepareForTermination()
        }
    }
    
    override func viewDidLayout() {
        super.viewDidLayout()
        contentView?.frame = self.view.bounds
    }
    
    private func loadROM(url: URL) {
        engine.loadROM(url) { [self] (success, featureSupport) in
            _romDidLoad(url, success: success, featureSupport: featureSupport)
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
            //TODO: present an alert
            preconditionFailure("failure to load ROM data is unhandled")
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
            //TODO: present an alert
            preconditionFailure("failure to load save data is unhandled")
        }
    }
    
    private func _clockDataDidLoad(_ success: Bool) {
        if success {
            _startEmulation()
        } else {
            //TODO: present an alert
            preconditionFailure("failure to load clock data is unhandled")
        }
    }
    
    private func _startEmulation() {
        let gameView = GameView(engine: engine)
        self.contentView = gameView
        self.gameView = gameView
        gameView.start()
        audioController.startAudioEngine()
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
    
    private func _prepareForTermination() {
        if engine.isSaveDataStale {
            print("Writing stale save data on quit")
            let saveData = engine.synchronousGetSaveData()
            _persistSaveData(saveData)
            engine.staleSaveDataHandled()
        }
        if engine.isClockDataStale {
            print("Writing stale clock data on quit")
            let clockData = engine.synchronousGetClockData()
            _persistClockData(clockData)
            engine.staleClockDataHandled()
        }
    }
    
    private func writeDisplayState(to url: URL) {
        engine.writeDisplayState(toDirectory: url) { (success) in
            //TODO: present an alert on failure
        }
    }
    
    private var isChoosingROM = false
    func didSelectChooseRom(view: NoROMView) {
        if isChoosingROM {
            return
        }
        isChoosingROM = true
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = true
        openPanel.canChooseDirectories = false
        openPanel.allowsMultipleSelection = false
        openPanel.begin { [weak self] (response) in
            self?.isChoosingROM = false
            if response == NSApplication.ModalResponse.OK {
                let fileURL = openPanel.urls[0]
                self?.loadROM(url: fileURL)
            }
        }
    }
    
    override var acceptsFirstResponder: Bool {
        return true
    }
    
    func validateMenuItem(_ menuItem: NSMenuItem) -> Bool {
        var valid = true;
        if menuItem.action == #selector(self.a_pauseEmulation) {
            if gameView?.state == .Playing {
                menuItem.title = "Pause Emulation"
            } else {
                menuItem.title = "Resume Emulation"
            }
        } else if menuItem.action == #selector(self.a_dumpDisplayState) {
            valid = (gameView?.state == .Paused)
        } else if menuItem.action == #selector(self.a_stepFrame) {
            valid = (gameView?.state == .Paused)
        }
        
        return valid;
    }
    
    @objc func a_pauseEmulation(_ sender: AnyObject) {
        guard let gv = gameView else {
            return
        }
        
        if gv.state == .Playing {
            gv.pause()
        } else {
            gv.start()
        }
    }
    
    @objc func a_dumpDisplayState(_ sender: AnyObject) {
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = false
        openPanel.canChooseDirectories = true
        openPanel.allowsMultipleSelection = false
        openPanel.begin { (response) in
            if response == NSApplication.ModalResponse.OK {
                let dirURL = openPanel.urls[0]
                self.writeDisplayState(to: dirURL)
            }
        }
    }
    
    @objc func a_stepFrame(_ sender: AnyObject) {
        guard let gv = gameView else {
            return
        }
        
        gv.engine.stepFrame()
    }
}

