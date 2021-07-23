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
    
    private var displayLink: CVDisplayLink?
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
    
    private func _startDisplayLink() {
        if let link = displayLink {
            if !CVDisplayLinkIsRunning(link) {
                let cvError = CVDisplayLinkStart(link)
                if cvError == kCVReturnSuccess {
                    state = .Playing
                } else {
                    print("Failed to restart display link")
                    state = .Error
                }
            } else {
                print("Game view is already running")
                state = .Playing
            }
            
        } else {
            var cvError = CVDisplayLinkCreateWithActiveCGDisplays(&displayLink)
            if (cvError != kCVReturnSuccess) {
                print("Failed to create display link")
                displayLink = nil
            }
            
            guard let resolvedLink = displayLink else {
                state = .Error
                return
            }
            
            //if still successful, set the display link output callback
            if (cvError == kCVReturnSuccess) {
                cvError = CVDisplayLinkSetOutputCallback(resolvedLink, {
                    (link : CVDisplayLink, currentTime : UnsafePointer<CVTimeStamp>, outputTime : UnsafePointer<CVTimeStamp>, _ : CVOptionFlags, _ : UnsafeMutablePointer<CVOptionFlags>, sourceUnsafeRaw : UnsafeMutableRawPointer?) -> CVReturn in
                    
                    if let sourceUnsafeRaw = sourceUnsafeRaw {
                        let viewUnmanaged = Unmanaged<GameView>.fromOpaque(sourceUnsafeRaw)
                        let timestamp: CFTimeInterval = Double(currentTime.pointee.videoTime) / Double(currentTime.pointee.videoTimeScale)
                        viewUnmanaged.takeUnretainedValue()._handleTimer(timestamp)
                    }
                    
                    return kCVReturnSuccess
                    }, Unmanaged.passUnretained(self).toOpaque())
                if (cvError != kCVReturnSuccess) {
                    print("Failed to set display link output callback")
                    displayLink = nil
                }
            }
            
            //if still successful, start the display link
            if (cvError == kCVReturnSuccess) {
                cvError = CVDisplayLinkStart(resolvedLink)
                if (cvError != kCVReturnSuccess) {
                    print("Failed to start display link")
                    displayLink = nil
                }
            }
            
            state = cvError == kCVReturnSuccess ? .Playing : .Error
        }
    }
    
    func pause() {
        engine.desiredRunnable = false
    }
    
    private func _pauseDisplayLink() {
        guard let link = displayLink else {
            print("Cannot pause game view with no display link")
            state = .Error
            return
        }
        
        if state == .Playing {
            if CVDisplayLinkIsRunning(link) {
                let cvResult = CVDisplayLinkStop(link)
                if cvResult == kCVReturnSuccess {
                    state = .Paused
                } else {
                    print("Failed to stop running display link")
                }
            } else {
                assertionFailure("State is playing but display link isn't running")
            }
        } else {
            print("Pause of a non-running game view does nothing")
        }
    }
    
    func engine(_ engine: GBEngine, receivedFrame frame: CGImage) {
        self.layer?.contents = frame
    }
    
    func engine(_ engine: GBEngine, runnableDidChange isRunnable: Bool) {
        if isRunnable {
            _startDisplayLink()
        } else {
            _pauseDisplayLink()
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
