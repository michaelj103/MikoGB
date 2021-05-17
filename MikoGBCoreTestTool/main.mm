//
//  main.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#import <Foundation/Foundation.h>
#import <ImageIO/ImageIO.h>
#include "GameboyCore.hpp"
#include <iostream>

using namespace std;

CGImageRef createImageWithPixelBuffer(const MikoGB::PixelBuffer &pixelBuffer) {
    size_t width = pixelBuffer.width;
    size_t height = pixelBuffer.height;
    size_t bitsPerComponent = 8;
    size_t bytesPerRow = pixelBuffer.width * 4;
    CGColorSpaceRef colorspace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    uint8_t *data = (uint8_t *)calloc(bytesPerRow * height, 1);
    CGContextRef context = CGBitmapContextCreate(data, width, height, bitsPerComponent, bytesPerRow, colorspace, kCGImageAlphaNoneSkipLast);
    CGColorSpaceRelease(colorspace);
    
    uint8_t *currentPtr = data;
    for (const MikoGB::Pixel &p : pixelBuffer.pixels) {
        currentPtr[0] = p.red;
        currentPtr[1] = p.green;
        currentPtr[2] = p.blue;
        currentPtr[3] = 0xFF;
        
        currentPtr += 4;
    }
    
    CGImageRef image = CGBitmapContextCreateImage(context);
    CGContextRelease(context);
    free(data);
    
    return image;
}

void writePNG(const MikoGB::PixelBuffer &pixelBuffer, NSString *filename) {
    CGImageRef image = createImageWithPixelBuffer(pixelBuffer);
    if (!image) {
        fprintf(stderr, "Failed to create image\n");
        return;
    }
        
    NSArray<NSString *> *paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES);
    NSString *desktopPath = paths.firstObject;
    if (!desktopPath) {
        fprintf(stderr, "Failed to get desktop\n");
    } else {
        NSString *destPath = [desktopPath stringByAppendingPathComponent:filename];
        NSURL *destURL = [NSURL fileURLWithPath:destPath];
        CGImageDestinationRef imageDest = CGImageDestinationCreateWithURL((__bridge CFURLRef)destURL, kUTTypePNG, 1, NULL);
        if (imageDest) {
            CGImageDestinationAddImage(imageDest, image, NULL);
            if (!CGImageDestinationFinalize(imageDest)) {
                fprintf(stderr, "Failed to finalize image destination\n");
            }
            CFRelease(imageDest);
        } else {
            fprintf(stderr, "Failed to create image destination\n");
        }
    }
    
    CGImageRelease(image);
}

int main(int argc, const char * argv[]) {
    MikoGB::GameBoyCore gbCore;
    int numFrames = 0;
    while (gbCore.getPC() < 0xfa) {
        gbCore.emulateFrame();
        numFrames++;
    }
    cout << "Emulated " << numFrames << " frames\n";
    
    void (^tileMapBlock)(const MikoGB::PixelBuffer &) = ^void(const MikoGB::PixelBuffer &pixelBuffer) {
        writePNG(pixelBuffer, @"tileMap.png");
    };
    
    gbCore.getTileMap([tileMapBlock](const MikoGB::PixelBuffer &pixelBuffer){
        tileMapBlock(pixelBuffer);
    });
    
    void (^backgroundBlock)(const MikoGB::PixelBuffer &) = ^void(const MikoGB::PixelBuffer &pixelBuffer) {
        writePNG(pixelBuffer, @"background.png");
    };
    
    gbCore.getBackground([backgroundBlock](const MikoGB::PixelBuffer &pixelBuffer){
        backgroundBlock(pixelBuffer);
    });
    
    return 0;
}
