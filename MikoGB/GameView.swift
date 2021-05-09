//
//  GameView.swift
//  MikoGB
//
//  Created on 5/8/21.
//

import Cocoa

class GameView : NSView, GBEngineImageDestination {
    
    var displayLink: CVDisplayLink?
    let engine: GBEngine
    
    init(engine: GBEngine) {
        self.engine = engine
        super.init(frame: NSZeroRect)
        self.wantsLayer = true
        self.engine.imageDestination = self
        
        self.layer?.backgroundColor = NSColor.blue.cgColor
    }
    
    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    private func _handleTimer(_ timestamp: CFTimeInterval) {
        self.engine.emulateFrame()
    }
    
    func start() {
        if (displayLink == nil) {
            var cvError = CVDisplayLinkCreateWithActiveCGDisplays(&displayLink)
            if (cvError != kCVReturnSuccess) {
                displayLink = nil
            }
            
            guard let resolvedLink = displayLink else {
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
                    displayLink = nil;
                }
            }
            
            //if still successful, start the display link
            if (cvError == kCVReturnSuccess) {
                cvError = CVDisplayLinkStart(resolvedLink);
                if (cvError != kCVReturnSuccess) {
                    displayLink = nil
                }
            }
        }
    }
    
    func engine(_ engine: GBEngine, receivedFrame frame: CGImage) {
        DispatchQueue.main.async {
            self.layer?.contents = frame
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
