//
//  GameView.swift
//  MikoGB
//
//  Created on 5/8/21.
//

import Cocoa

enum GameViewState {
    case Initial
    case Playing
    case Paused
    case Error
}

class GameView : NSView, GBEngineImageDestination, GBEngineObserver {
    
    private var dispatchTimer: DispatchSourceTimer?
    let engine: GBEngine
    var state: GameViewState
    
    init(engine: GBEngine) {
        self.engine = engine
        self.state = .Initial
        super.init(frame: NSZeroRect)
        self.wantsLayer = true
        self.engine.imageDestination = self
        self.engine.register(self)
        
        self.layer?.backgroundColor = NSColor.blue.cgColor
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    private func _handleTimer(_ timestamp: CFTimeInterval) {
        self.engine.emulateFrame()
    }
    
    func start() {
        engine.desiredRunnable = true
    }
    
    private func _startTimer() {
        if let timer = dispatchTimer {
            timer.resume()
        } else {
            let timer = DispatchSource.makeTimerSource(flags: .strict, queue: DispatchQueue.main)
            let framerate = 1.0 / 60.0
            timer.schedule(deadline: DispatchTime.now() + framerate, repeating: framerate)
            timer.setEventHandler { [weak self] in
                let time = CFAbsoluteTimeGetCurrent()
                self?._handleTimer(time)
            }
            timer.activate()
            dispatchTimer = timer
        }
        state = .Playing
    }
    
    private func _pauseTimer() {
        guard let timer = dispatchTimer else {
            print("Cannot pause game view with no timer")
            state = .Error
            return
        }
        
        if state == .Playing {
            timer.suspend()
            state = .Paused
        } else {
            print("Pause of a non-running game view does nothing")
        }
    }
    
    
    func pause() {
        engine.desiredRunnable = false
    }
        
    func engine(_ engine: GBEngine, receivedFrame frame: CGImage) {
        self.layer?.contents = frame
    }
    
    func engine(_ engine: GBEngine, runnableDidChange isRunnable: Bool) {
        if isRunnable {
            _startTimer()
        } else {
            _pauseTimer()
        }
    }
    
    override var acceptsFirstResponder: Bool {
        return true
    }
    
    override func keyDown(with event: NSEvent) {
        if !event.isARepeat {
            switch event.keyCode {
            case 0x24:
                engine.setKeyDown(.start)
            case 0x30:
                engine.setKeyDown(.select)
            case 0x7B:
                engine.setKeyDown(.left)
            case 0x7C:
                engine.setKeyDown(.right)
            case 0x7D:
                engine.setKeyDown(.down)
            case 0x7E:
                engine.setKeyDown(.up)
            case 0x06:
                engine.setKeyDown(.A)
            case 0x07:
                engine.setKeyDown(.B)
            default:
                break
            }
        }
    }
    
    override func keyUp(with event: NSEvent) {
        if !event.isARepeat {
            switch event.keyCode {
            case 0x24:
                engine.setKeyUp(.start)
            case 0x30:
                engine.setKeyUp(.select)
            case 0x7B:
                engine.setKeyUp(.left)
            case 0x7C:
                engine.setKeyUp(.right)
            case 0x7D:
                engine.setKeyUp(.down)
            case 0x7E:
                engine.setKeyUp(.up)
            case 0x06:
                engine.setKeyUp(.A)
            case 0x07:
                engine.setKeyUp(.B)
            default:
                break
            }
        }
    }
}
