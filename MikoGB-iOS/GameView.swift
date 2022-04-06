//
//  GameView.swift
//  MikoGB-iOS
//
//  Created by Michael Brandt on 3/19/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

import UIKit

enum GameViewState {
    case Initial
    case Playing
    case Paused
    case Error
}

class GameView : UIView, GBEngineImageDestination, GBEngineObserver {
    private var dispatchTimer: DispatchSourceTimer?
    var engine: GBEngine {
        didSet {
            _setupWithEngine(engine, oldEngine: oldValue)
        }
    }
    private(set) var state: GameViewState
    
    init(engine: GBEngine) {
        self.engine = engine
        self.state = .Initial
        super.init(frame: CGRect.zero)
        _setupWithEngine(self.engine, oldEngine: nil)
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    private func _setupWithEngine(_ engine: GBEngine, oldEngine: GBEngine?) {
        self.state = .Initial
        oldEngine?.imageDestination = nil
        oldEngine?.unregisterObserver(self)
        engine.imageDestination = self
        engine.register(self)
    }
    
    override func sizeThatFits(_ size: CGSize) -> CGSize {
        let widthScale = size.width / 160.0
        let heightScale = size.height / 144.0
        
        let scale = min(widthScale, heightScale)
        let size = CGSize(width: 160.0 * scale, height: 144.0 * scale)
        return size
    }
    
    private func _handleTimer(_ timestamp: CFTimeInterval) {
        self.engine.emulateFrame()
        // toggle speed mode if requested
        if _switchModes {
            // cancel and recreate the timer at the new desired framerate
            _switchModes = false
            if let timer = dispatchTimer {
                timer.cancel()
                dispatchTimer = nil
                _startTimer()
            }
        }
    }
    
    func start() {
        engine.desiredRunnable = true
    }
    
    private func _startTimer() {
        if let timer = dispatchTimer {
            timer.resume()
        } else {
            let timer = DispatchSource.makeTimerSource(flags: .strict, queue: DispatchQueue.main)
            let framerate = desiredFramerate
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
    
    private var _switchModes = false
    private(set) var isInSpeedMode = false
    private var desiredFramerate = 1.0 / 60.0
    func toggleSpeedMode() {
        // ignore multiple toggles during the same frame (fast fingers!)
        if !_switchModes {
            _switchModes = true
            isInSpeedMode = !isInSpeedMode
            if isInSpeedMode {
                // 90fps (1.5x speed)
                desiredFramerate = 1.0 / 90.0
            } else {
                // 60fps (normal)
                desiredFramerate = 1.0 / 60.0
            }
        }
    }
        
    func engine(_ engine: GBEngine, receivedFrame frame: CGImage) {
        self.layer.contents = frame
    }
    
    func engine(_ engine: GBEngine, runnableDidChange isRunnable: Bool) {
        if isRunnable {
            _startTimer()
        } else {
            _pauseTimer()
        }
    }
    
    func didUpdateSuspendedState(for engine: GBEngine) {
        // means the state moved via something other than the timer. nothing to do
    }
}
