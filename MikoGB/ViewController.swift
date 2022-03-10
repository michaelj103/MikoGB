//
//  ViewController.swift
//  MikoGB
//
//  Created on 5/4/21.
//

import Cocoa

class ViewController: NSViewController, NoROMViewDelegate, NSMenuItemValidation {

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
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let emptyView = NoROMView()
        emptyView.delegate = self
        contentView = emptyView
        self.view.addSubview(emptyView)
        
        engine = AppStateManager.sharedInstance.engine
        audioController = AudioController(engine: engine)
        persistenceManager = AppStateManager.sharedInstance.persistenceManager
    }
    
    override func viewDidLayout() {
        super.viewDidLayout()
        contentView?.frame = self.view.bounds
    }
    
    private func loadROM(url: URL) {
        engine.loadROM(url) { [self] (success, supportsSaveData) in
            _romDidLoad(url, success: success, supportsSaveData: supportsSaveData)
        }
    }
    
    private func _loadSaveData(_ url: URL) {
        // TODO: MJB: appropriate fallbacks instead of crash
        let entry = try! persistenceManager.loadSaveEntry(url)
        if let data = try! persistenceManager.loadSaveData(entry) {
            // now, do something with the data
        }
    }
    
    private func _romDidLoad(_ url: URL, success: Bool, supportsSaveData: Bool) {
        if success {
            if supportsSaveData {
                _loadSaveData(url)
            }
            let gameView = GameView(engine: engine)
            self.contentView = gameView
            self.gameView = gameView
            gameView.start()
            audioController.startAudioEngine()
        } else {
            //TODO: present an alert
        }
    }
    
    private func writeDisplayState(to url: URL) {
        engine.writeDisplayState(toDirectory: url) { (success) in
            //TODO: present an alert on failure
        }
    }
    
    func didSelectChooseRom(view: NoROMView) {
        let openPanel = NSOpenPanel()
        openPanel.canChooseFiles = true
        openPanel.canChooseDirectories = false
        openPanel.allowsMultipleSelection = false
        openPanel.begin { (response) in
            if response == NSApplication.ModalResponse.OK {
                let fileURL = openPanel.urls[0]
                self.loadROM(url: fileURL)
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
}

