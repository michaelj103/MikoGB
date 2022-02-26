//
//  GBEngine.m
//  MikoGB
//
//  Created on 5/8/21.
//

#import "GBEngine.h"

#import <iostream>
#import "GameBoyCore.hpp"
#import <os/lock.h>
#import "GBImageUtilities.h"
#import "GBAudioWriter.h"

static const size_t GBBytesPerLine = 160 * 4;
static const size_t GBBytesPerImage = GBBytesPerLine * 144;

static const size_t NumKeys = 8; //Number of gameboy keys

static MikoGB::JoypadButton _ButtonForCode(GBEngineKeyCode code) {
    switch (code) {
        case GBEngineKeyCodeRight:
            return MikoGB::JoypadButton::Right;
        case GBEngineKeyCodeLeft:
            return MikoGB::JoypadButton::Left;
        case GBEngineKeyCodeUp:
            return MikoGB::JoypadButton::Up;
        case GBEngineKeyCodeDown:
            return MikoGB::JoypadButton::Down;
        case GBEngineKeyCodeA:
            return MikoGB::JoypadButton::A;
        case GBEngineKeyCodeB:
            return MikoGB::JoypadButton::B;
        case GBEngineKeyCodeSelect:
            return MikoGB::JoypadButton::Select;
        case GBEngineKeyCodeStart:
            return MikoGB::JoypadButton::Start;
    }
}

@implementation GBEngine {
    MikoGB::GameBoyCore *_core;
    uint8_t *_imageBuffer;
    CGContextRef _cgContext;
    
    os_unfair_lock _keyLock;
    BOOL _keyPressedStates[NumKeys];
    BOOL _keyPressedChanged[NumKeys];
    BOOL _hasKeyChange;
    
    dispatch_queue_t _emulationQueue;
    os_unfair_lock _frameLock;
    BOOL _isProcessingFrame;
    BOOL _isRunnable;
    
    NSHashTable<id<GBEngineObserver>> *_observers;
//    GBAudioWriter *_audioWriter;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        __weak typeof(self) weakSelf = self;
        void (^scanlineBlock)(const MikoGB::PixelBuffer &, size_t) = ^void(const MikoGB::PixelBuffer &scanline, size_t line) {
            [weakSelf _handleScanline:scanline line:line];
        };
        
        void (^audioBlock)(int16_t, int16_t) = ^void(int16_t l, int16_t r) {
            [weakSelf _handleAudioSampleLeft:l right:r];
        };
        
        _core = new MikoGB::GameBoyCore();
        _core->setScanlineCallback([scanlineBlock](const MikoGB::PixelBuffer &scanline, size_t line) {
            scanlineBlock(scanline, line);
        });
        _core->setAudioSampleCallback([audioBlock](int16_t l, int16_t r){
            audioBlock(l, r);
        });
        
        void (^runnableBlock)(bool) = ^void(bool isRunnable) {
            [weakSelf _handleRunnableChange:isRunnable];
        };
        _core->setRunnableChangedCallback([runnableBlock](bool isRunnable) {
            runnableBlock(isRunnable);
        });
        _isRunnable = _core->isRunnable() ? YES : NO;
        
        dispatch_queue_attr_t attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, 0);
        _emulationQueue = dispatch_queue_create("EmulationQueue", attr);
        _frameLock = OS_UNFAIR_LOCK_INIT;
        
        const size_t width = 160;
        const size_t height = 144;
        const size_t bitsPerComponent = 8;
        const size_t numComponents = 4;
        const size_t bytesPerRow = numComponents * width;
        _imageBuffer = (uint8_t *)calloc(GBBytesPerImage, 1);
        CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
        _cgContext = CGBitmapContextCreate(_imageBuffer, width, height, bitsPerComponent, bytesPerRow, colorspace, kCGImageAlphaNoneSkipLast);
        CGColorSpaceRelease(colorspace);
        
        _keyLock = OS_UNFAIR_LOCK_INIT;
        for (size_t i = 0; i < NumKeys; ++i) {
            _keyPressedStates[i] = NO;
            _keyPressedChanged[i] = NO;
        }
        _observers = [NSHashTable weakObjectsHashTable];
        
//        _audioWriter = [[GBAudioWriter alloc] init];
//        self.audioDestination = _audioWriter;
    }
    return self;
}

- (void)dealloc {
    delete _core;
    free(_imageBuffer);
    CGContextRelease(_cgContext);
}

- (void)loadROM:(NSURL *)url completion:(void (^)(BOOL))completion {
    NSError *readErr = nil;
    NSData *data = [NSData dataWithContentsOfURL:url options:NSDataReadingMappedIfSafe error:&readErr];
    if (!data) {
        NSLog(@"Failed to read data from URL \'%@\': %@", url, readErr);
        if (completion) {
            completion(NO);
        }
        return;
    }
    
    dispatch_async(_emulationQueue, ^{
        bool success = self->_core->loadROMData(data.bytes, data.length);
        if (completion) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(success ? YES : NO);
            });
        }
    });
}

- (void)writeDisplayStateToDirectory:(NSURL *)directoryURL completion:(void (^)(BOOL))completion {
    NSFileManager *fm = NSFileManager.defaultManager;
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:directoryURL.path isDirectory:&isDir] || !isDir) {
        if (completion) {
            completion(NO);
        }
        return;
    }
    
    dispatch_async(_emulationQueue, ^{
        [self _writeOutTileMapAndBackground:directoryURL];
    });
}

- (void)_updateKeyStatesIfNeeded {
    os_unfair_lock_lock(&_keyLock);
    if (_hasKeyChange) {
        _hasKeyChange = NO;
        for (size_t i = 0; i < NumKeys; ++i) {
            if (_keyPressedChanged[i]) {
                _keyPressedChanged[i] = NO;
                _keyPressedStates[i] = !_keyPressedStates[i];
                bool state = _keyPressedStates[i] ? true : false;
                _core->setButtonPressed(_ButtonForCode((GBEngineKeyCode)i), state);
            }
        }
    }
    os_unfair_lock_unlock(&_keyLock);
}

- (void)_writeOutTileMapAndBackground:(NSURL *)dirURL {
    void (^tileMapBlock)(const MikoGB::PixelBuffer &) = ^void(const MikoGB::PixelBuffer &pixelBuffer) {
        writePNG(pixelBuffer, dirURL, @"tileMap.png");
    };
    
    _core->getTileMap([tileMapBlock](const MikoGB::PixelBuffer &pixelBuffer){
        tileMapBlock(pixelBuffer);
    });
    
    void (^backgroundBlock)(const MikoGB::PixelBuffer &) = ^void(const MikoGB::PixelBuffer &pixelBuffer) {
        writePNG(pixelBuffer, dirURL, @"background.png");
    };
    
    _core->getBackground([backgroundBlock](const MikoGB::PixelBuffer &pixelBuffer){
        backgroundBlock(pixelBuffer);
    });
}

- (void)emulateFrame {
    // Thread safe to allow more freedom for clients in terms of timer mechanisms
    BOOL canRun = NO;
    BOOL dropped = NO;
    os_unfair_lock_lock(&_frameLock);
    dropped = _isProcessingFrame;
    canRun = !dropped && _isRunnable;
    os_unfair_lock_unlock(&_frameLock);
    
    if (canRun) {
        dispatch_async(_emulationQueue, ^{
            [self _emulationQueue_emulateFrame];
        });
    } else if (dropped) {
        // Dropped a frame if the previous frame has not finished processing when the next one starts
        NSLog(@"Dropped a frame");
    }
}

- (void)_emulationQueue_emulateFrame {
    os_unfair_lock_lock(&_frameLock);
    _isProcessingFrame = YES;
    os_unfair_lock_unlock(&_frameLock);
    
    [self _updateKeyStatesIfNeeded];
    self->_core->emulateFrame();
    
    os_unfair_lock_lock(&_frameLock);
    _isProcessingFrame = NO;
    os_unfair_lock_unlock(&_frameLock);
}

- (void)step:(NSInteger)stepCount {
    dispatch_async(_emulationQueue, ^{
        [self _emulationQueue_step:stepCount];
    });
}

- (void)_emulationQueue_step:(NSInteger)stepCount {
    if (_core->isRunnable()) {
        // must be in a paused state to manually step
        NSLog(@"Manual step requested but emulation is runnable. Must be paused");
        return;
    }
    for (NSInteger i = 0; i < stepCount; i++) {
        _core->step();
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [self _notifyObserversOfSuspendedStateChange];
    });
}

- (BOOL)isRunnable {
    BOOL isRunnable = NO;
    os_unfair_lock_lock(&_frameLock);
    isRunnable = _isRunnable;
    os_unfair_lock_unlock(&_frameLock);
    return isRunnable;
}

- (void)setDesiredRunnable:(BOOL)desiredRunnable completion:(void (^_Nullable)(void))completion {
    // don't check if the desire has changed because the desire may be overridden by internal state
    _desiredRunnable = desiredRunnable;
    
    dispatch_async(_emulationQueue, ^{
        bool r = desiredRunnable ? true : false;
        self->_core->setRunnable(r);
        if (completion) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completion();
            });
        }
    });
}

- (void)setDesiredRunnable:(BOOL)desiredRunnable {
    [self setDesiredRunnable:desiredRunnable completion:nil];
}

- (void)_setDesiredState:(BOOL)desired forKeyCode:(GBEngineKeyCode)code {
    os_unfair_lock_lock(&_keyLock);
    if (_keyPressedStates[code] == desired) {
        //Already in that state so undo pending changes (if any)
        _keyPressedChanged[code] = NO;
    } else {
        //New state, mark as pending for next frame update
        _hasKeyChange = YES;
        _keyPressedChanged[code] = YES;
    }
    os_unfair_lock_unlock(&_keyLock);
}

- (void)setKeyDown:(GBEngineKeyCode)keyCode {
    [self _setDesiredState:YES forKeyCode:keyCode];
}

- (void)setKeyUp:(GBEngineKeyCode)keyCode {
    [self _setDesiredState:NO forKeyCode:keyCode];
}

static void _AddInstruction(const MikoGB::DisassembledInstruction &instruction, NSMutableArray<NSString *> *array) {
    NSString *addressString = nil;
    if (instruction.romBank == -1) {
        addressString = [NSString stringWithFormat:@"RAM: 0x%X", instruction.addr];
    } else {
        addressString = [NSString stringWithFormat:@"%d: 0x%X", instruction.romBank, instruction.addr];
    }
    [array addObject:addressString];
    
    const char *cStr = instruction.description.c_str();
    NSString *desc = [NSString stringWithCString:cStr encoding:NSASCIIStringEncoding];
    [array addObject:desc];
}

- (NSArray<NSString *> *)disassembledInstructions:(size_t *)currentIndex {
    NSMutableArray<NSString *> *disassembledInstructions = [NSMutableArray array];
    __block size_t startIndex = 0;
    dispatch_sync(_emulationQueue, ^{
        size_t tmpIdx = 0;
        std::vector<MikoGB::DisassembledInstruction> instructions = _core->getDisassembledInstructions(10, 10, &tmpIdx);
        startIndex = tmpIdx;
        for (const MikoGB::DisassembledInstruction &instruction : instructions) {
            _AddInstruction(instruction, disassembledInstructions);
        }
    });
    
    if (currentIndex) {
        *currentIndex = startIndex;
    }
    return disassembledInstructions;
}

- (GBRegisterState)registerState {
    __block GBRegisterState state;
    dispatch_sync(_emulationQueue, ^{
        MikoGB::RegisterState rState = _core->getRegisterState();
        state.B = rState.B;
        state.C = rState.C;
        state.D = rState.D;
        state.E = rState.E;
        state.H = rState.H;
        state.L = rState.L;
        state.A = rState.A;
        state.ZFlag = (rState.ZFlag == true);
        state.NFlag = (rState.NFlag == true);
        state.HFlag = (rState.HFlag == true);
        state.CFlag = (rState.CFlag == true);
    });
    return state;
}

- (uint8_t)readByte:(uint16_t)addr {
    __block uint8_t byte = 0;
    dispatch_sync(_emulationQueue, ^{
        byte = _core->readMem(addr);
    });
    return byte;
}

- (BOOL)addLineBreakpointForBank:(int)romBank address:(uint16_t)address {
    __block BOOL success = NO;
    dispatch_sync(_emulationQueue, ^{
        bool couldSet = _core->setLineBreakpoint(romBank, address);
        success = couldSet ? YES : NO;
    });
    return success;
}

- (void)_deliverFrameImage {
    CGImageRef image = CGBitmapContextCreateImage(_cgContext);
    dispatch_async(dispatch_get_main_queue(), ^{
        [self _mainQueue_deliverFrameImage:image];
        CGImageRelease(image);
    });
}

- (void)_mainQueue_deliverFrameImage:(CGImageRef)image {
    [self.imageDestination engine:self receivedFrame:image];
}

// Expected on emulation queue
- (void)_handleScanline:(const MikoGB::PixelBuffer &)scanline line:(size_t)line {
    size_t bufferOffset = GBBytesPerLine * line;
    uint8_t *buffer = _imageBuffer;
    for (size_t pIdx = 0, bIdx = bufferOffset; pIdx < 160; ++pIdx, bIdx += 4) {
        const MikoGB::Pixel &px = scanline.pixels[pIdx];
        // RGBA format
        buffer[bIdx] = px.red;
        buffer[bIdx + 1] = px.green;
        buffer[bIdx + 2] = px.blue;
        buffer[bIdx + 3] = 0xFF;
    }
    
    if (line >= 143) {
        [self _deliverFrameImage];
    }
}

- (void)_handleAudioSampleLeft:(int16_t)left right:(int16_t)right {
    [self.audioDestination engine:self receivedAudioSampleLeft:left right:right];
}

// Expected on emulation queue
- (void)_handleRunnableChange:(bool)isRunnable {
    dispatch_assert_queue(_emulationQueue);
    
    BOOL b_isRunnable = isRunnable ? YES : NO;
    os_unfair_lock_lock(&_frameLock);
    _isRunnable = b_isRunnable;
    os_unfair_lock_unlock(&_frameLock);
        
    dispatch_async(dispatch_get_main_queue(), ^{
        [self _notifyObserversOfRunnable:b_isRunnable];
    });
}

#pragma mark - Observation

- (void)registerObserver:(id<GBEngineObserver>)observer {
    [_observers addObject:observer];
}

- (void)unregisterObserver:(id<GBEngineObserver>)observer {
    [_observers removeObject:observer];
}

- (void)_notifyObserversOfRunnable:(BOOL)isRunnable {
    NSHashTable<id<GBEngineObserver>> *observersCopy = [_observers copy];
    for (id<GBEngineObserver> observer in observersCopy) {
        [observer engine:self runnableDidChange:isRunnable];
    }
}

- (void)_notifyObserversOfSuspendedStateChange {
    NSHashTable<id<GBEngineObserver>> *observersCopy = [_observers copy];
    for (id<GBEngineObserver> observer in observersCopy) {
        [observer didUpdateSuspendedStateForEngine:self];
    }
}

@end
