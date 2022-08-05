//
//  GBAudioWriter.m
//  GBCoreTester
//
//  Created by Michael Brandt on 4/29/20.
//  Copyright © 2020 Michael Brandt. All rights reserved.
//

#import "GBAudioWriter.h"
#import "WavFile.h"

static const int DelayTime = 15;
static const int DesiredTime = 45;

@implementation GBAudioWriter {
    int _waitSamples;
    int _sampleIdx;
    WAVFile *_wavFile;
    WAVSound *_wavSound;
    int _desiredSamples;
    int _delaySamples;
}

+ (NSString *)documentsPath {
    NSArray<NSString *> *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return paths[0];
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _sampleIdx = 0;
        NSString *testPath = [[self.class documentsPath] stringByAppendingPathComponent:@"testSound.wav"];
        NSLog(@"Writing audio to %@", testPath);
        _wavFile = wavfile_open(testPath.UTF8String);
        _wavSound = wavfile_createSound(_wavFile, (double)DesiredTime);
        _desiredSamples = 44100 * DesiredTime;
        _delaySamples = 44100 * DelayTime;
    }
    return self;
}

- (void)engine:(nonnull GBEngine *)engine receivedAudioSampleLeft:(int16_t)left right:(int16_t)right {
    if (_waitSamples < _delaySamples) {
        _waitSamples++;
        return;
    }
    if (_sampleIdx < _desiredSamples) {
        wavSound_addSample(_wavSound, right, _sampleIdx++);
        if (_sampleIdx == _desiredSamples) {
            //time to stop
            wavfile_finalizeSound(_wavFile, _wavSound, 0.0);
            wavfile_close(_wavFile);
            
            _wavSound = NULL;
            _wavFile = NULL;
        }
    }
}

@end
