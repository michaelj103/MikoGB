//
//  AudioController.swift
//  GBCoreTester
//
//  Created by Michael Brandt on 4/29/20.
//  Copyright © 2020 Michael Brandt. All rights reserved.
//

import Foundation
import AVFoundation

class AudioController : NSObject, GBEngineAudioDestination {
    
    let engine: GBEngine
    private var leftAudioSampleBuffer: RingBuffer<Float32>
    private var rightAudioSampleBuffer: RingBuffer<Float32>
    private let bufferQueue = DispatchQueue(label: "AudioControllerBufferQueue")
    private var audioEngine: AVAudioEngine?
    
    private static let bufferSize = 16384
    
    init(engine: GBEngine) {
        self.engine = engine
        self.leftAudioSampleBuffer = RingBuffer<Float32>(repeating: 0, count: AudioController.bufferSize)
        self.rightAudioSampleBuffer = RingBuffer<Float32>(repeating: 0, count: AudioController.bufferSize)
        super.init()
        self.engine.audioDestination = self
    }
    
    private func _initializeAudioEngine() {
        let engine = AVAudioEngine()
        let mainMixer = engine.mainMixerNode
        let output = engine.outputNode
        let outputFormat = output.inputFormat(forBus: 0)
        // Use output format for input but reduce channel count to 1
        let inputFormat = AVAudioFormat(commonFormat: .pcmFormatFloat32,
                                        sampleRate: outputFormat.sampleRate,
                                        channels: 2,
                                        interleaved: outputFormat.isInterleaved)
        
        // TODO: weak self?
        let srcNode = AVAudioSourceNode { isSilence, _, frameCount, audioBufferList -> OSStatus in
            let ablPointer = UnsafeMutableAudioBufferListPointer(audioBufferList)
            var leftBuf: UnsafeMutableBufferPointer<Float32> = UnsafeMutableBufferPointer(ablPointer[0])
            var rightBuf: UnsafeMutableBufferPointer<Float32> = UnsafeMutableBufferPointer(ablPointer[1])
            self.bufferQueue.sync {
                let maxCount = Int(frameCount)
                let leftReadCount = min(self.leftAudioSampleBuffer.count, maxCount)
                let rightReadCount = min(self.rightAudioSampleBuffer.count, maxCount)
//                print("MJB: requested \(maxCount) leftover \(self.leftAudioSampleBuffer.count)")
                try! self.leftAudioSampleBuffer.read(into: &leftBuf, count: leftReadCount)
                try! self.rightAudioSampleBuffer.read(into: &rightBuf, count: rightReadCount)
            }
            return noErr
        }
        
        engine.attach(srcNode)
        
        engine.connect(srcNode, to: mainMixer, format: inputFormat)
        engine.connect(mainMixer, to: output, format: outputFormat)
        mainMixer.outputVolume = 0.5
        
        self.audioEngine = engine
    }
    
    func startAudioEngine() {
        _initializeAudioEngine()
        do {
            try self.audioEngine?.start()
        } catch {
            print("Failed to start audio engine \(error)")
        }
    }
    
    func stopAudioEngine() {
        audioEngine?.stop()
        audioEngine = nil
    }
    
    func engine(_ engine: GBEngine, receivedAudioSampleLeft left: Int16, right: Int16) {
        bufferQueue.sync {
            let leftFloat = Float32(Float(left)/32768.0)
            let rightFloat = Float32(Float(right)/32768.0)
            leftAudioSampleBuffer.write(leftFloat)
            rightAudioSampleBuffer.write(rightFloat)
        }
    }
}
