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

static const size_t GBBytesPerLine = 160 * 4;
static const size_t GBBytesPerImage = GBBytesPerLine * 144;

static const size_t NumKeys = 8; //Number of gameboy keys

@implementation GBEngine {
    MikoGB::GameBoyCore *_core;
    uint8_t *_imageBuffer;
    CGContextRef _cgContext;
    
    os_unfair_lock _keyLock;
    BOOL _keyPressedStates[NumKeys];
    BOOL _keyPressedChanged[NumKeys];
    BOOL _hasKeyChange;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        __weak typeof(self) weakSelf = self;
        void (^scanlineBlock)(const MikoGB::PixelBuffer &, size_t) = ^void(const MikoGB::PixelBuffer &scanline, size_t line) {
            [weakSelf _handleScanline:scanline line:line];
        };
        
        _core = new MikoGB::GameBoyCore();
        _core->setScanlineCallback([scanlineBlock](const MikoGB::PixelBuffer &scanline, size_t line) {
            scanlineBlock(scanline, line);
        });
        
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
    }
    return self;
}

- (void)dealloc {
    delete _core;
    free(_imageBuffer);
    CGContextRelease(_cgContext);
}

- (BOOL)loadROM:(NSURL *)url {
    NSError *readErr = nil;
    NSData *data = [NSData dataWithContentsOfURL:url options:NSDataReadingMappedIfSafe error:&readErr];
    if (!data) {
        NSLog(@"Failed to read data from URL \'%@\': %@", url, readErr);
        return NO;
    }
    bool success = _core->loadROMData(data.bytes, data.length);
    return success ? YES : NO;
}

- (void)_updateKeyStatesIfNeeded {
    os_unfair_lock_lock(&_keyLock);
    if (_hasKeyChange) {
        _hasKeyChange = NO;
        for (size_t i = 0; i < NumKeys; ++i) {
            if (_keyPressedChanged[i]) {
                _keyPressedChanged[i] = NO;
                _keyPressedStates[i] = !_keyPressedStates[i];
                //TODO: set the pressed state of the key on the core
            }
        }
    }
    os_unfair_lock_unlock(&_keyLock);
}

- (void)emulateFrame {
    [self _updateKeyStatesIfNeeded];
    _core->emulateFrame();
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

- (void)_deliverFrameImage {
    CGImageRef image = CGBitmapContextCreateImage(_cgContext);
    [self.imageDestination engine:self receivedFrame:image];
    CGImageRelease(image);
}

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

@end
