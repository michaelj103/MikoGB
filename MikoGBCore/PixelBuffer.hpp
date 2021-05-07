//
//  PixelBuffer.hpp
//  MikoGB
//
//  Created on 5/5/21.
//

#ifndef PixelBuffer_hpp
#define PixelBuffer_hpp

#include <cstdlib>
#include <vector>

namespace MikoGB {

/// RGB8 Pixels. No colorspace since GB screens were before that. Wing it on the display side
struct Pixel {
    uint8_t red;
    uint8_t green = 255; // Make uninitialized pixels bright green so bugs are visible
    uint8_t blue;
    
    Pixel() = default;
    
    /// Initialize with RGB values
    Pixel(uint8_t r, uint8_t g, uint8_t b): red(r), green(g), blue(b) {}
    
    /// Grayscale convenience
    Pixel(uint8_t white): red(white), green(white), blue(white) {}
};

struct PixelBuffer {
    size_t width;
    size_t height;
    std::vector<Pixel> pixels;
    
    PixelBuffer(size_t w, size_t h): width(w), height(h), pixels(w*h) {}
};

}

#endif /* PixelBuffer_hpp */
