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
    
    private static let bufferSize = 1 << 13
    private static let bufferDrop = bufferSize / 2
    
    // AUDIO_SAMPLE_RATE <- search for this and uncomment to track and print actual audio sample rate
//    private var lastCFTime: Double = 0.0
//    private var elapsedTime: Double = 0.0
//    private var weightedSampleRatio: Double = 1.0
//    private var receivedSamples: Int = 0
    
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
        
        let srcNode = AVAudioSourceNode { [weak self] _, _, frameCount, audioBufferList -> OSStatus in
            self?._audioCallback(frameCount: frameCount, audioBufferList: audioBufferList)
            return noErr
        }
        
        engine.attach(srcNode)
        
        engine.connect(srcNode, to: mainMixer, format: inputFormat)
        engine.connect(mainMixer, to: output, format: outputFormat)
        mainMixer.outputVolume = 0.5
        
        self.audioEngine = engine
    }
    
    func _audioCallback(frameCount: AVAudioFrameCount, audioBufferList: UnsafeMutablePointer<AudioBufferList>) {
        let ablPointer = UnsafeMutableAudioBufferListPointer(audioBufferList)
        var leftBuf: UnsafeMutableBufferPointer<Float32> = UnsafeMutableBufferPointer(ablPointer[0])
        var rightBuf: UnsafeMutableBufferPointer<Float32> = UnsafeMutableBufferPointer(ablPointer[1])
        
        self.bufferQueue.sync {
            // AUDIO_SAMPLE_RATE
//            let cfTime = CFAbsoluteTimeGetCurrent()
//            let diff = cfTime - lastCFTime
//            lastCFTime = cfTime
//            elapsedTime += diff
//            if elapsedTime >= 1.0 {
//                let actualSampleRate = Double(receivedSamples) / elapsedTime
//                let ratio = actualSampleRate / 44100.0
//                weightedSampleRatio = (0.95 * weightedSampleRatio) + (0.05 * ratio)
//                print("sample rate \(weightedSampleRatio * 44100.0)")
//                elapsedTime = 0.0
//                receivedSamples = 0
//            }
            
            let maxCount = Int(frameCount)
            let leftReadCount = min(leftAudioSampleBuffer.count, maxCount)
            let rightReadCount = min(rightAudioSampleBuffer.count, maxCount)
//            print("MJB: requested \(maxCount) leftover \(self.leftAudioSampleBuffer.count)")
            try! leftAudioSampleBuffer.read(into: &leftBuf, count: leftReadCount)
            try! rightAudioSampleBuffer.read(into: &rightBuf, count: rightReadCount)
        }
    }
    
    func startAudioEngine() {
        _initializeAudioEngine()
        do {
            // AUDIO_SAMPLE_RATE
//            lastCFTime = CFAbsoluteTimeGetCurrent()
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
            // AUDIO_SAMPLE_RATE
//            receivedSamples += 1
            let leftFloat = Float32(Float(left)/32768.0)
            let rightFloat = Float32(Float(right)/32768.0)
            leftAudioSampleBuffer.write(leftFloat)
            rightAudioSampleBuffer.write(rightFloat)
            if leftAudioSampleBuffer.isFull {
                print("Audio skip")
                leftAudioSampleBuffer.drop(AudioController.bufferDrop)
                rightAudioSampleBuffer.drop(AudioController.bufferDrop)
            }
        }
    }
}
