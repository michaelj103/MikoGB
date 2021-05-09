//
//  ViewController.swift
//  MikoGB
//
//  Created on 5/4/21.
//

import Cocoa

class ViewController: NSViewController, NoROMViewDelegate {

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
//    private var audioController: AudioController!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let emptyView = NoROMView()
        emptyView.delegate = self
        contentView = emptyView
        self.view.addSubview(emptyView)
        
        engine = GBEngine()
//        audioController = AudioController(engine: engine)
    }
    
    override func viewDidLayout() {
        super.viewDidLayout()
        contentView?.frame = self.view.bounds
    }
    
    private func loadROM(url: URL) {
        if (engine.loadROM(url)) {
            let gameView = GameView(engine: engine)
            contentView = gameView
            gameView.start()
//            audioController.startAudioEngine()
        } else {
            //TODO: present an alert
        }
    }
    
    func didSelectChooseRom(view: NoROMView) {
//        let openPanel = NSOpenPanel()
//        openPanel.canChooseFiles = true
//        openPanel.canChooseDirectories = false
//        openPanel.allowsMultipleSelection = false
//        openPanel.begin { (response) in
//            if response == NSApplication.ModalResponse.OK {
//                let fileURL = openPanel.urls[0]
//                self.loadROM(url: fileURL)
//            }
//        }
        
        let gameView = GameView(engine: engine)
        contentView = gameView
        gameView.start()
    }
    
    override var acceptsFirstResponder: Bool {
        return true
    }
}

