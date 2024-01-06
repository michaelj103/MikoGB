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
#include <functional>
#include <cassert>

namespace MikoGB {

/// RGB8 Pixels. No colorspace since GB screens were before that. Wing it on the display side
struct Pixel {
    uint8_t red = 0;
    uint8_t green = 255; // Make uninitialized pixels bright green so bugs are visible
    uint8_t blue = 0;
    
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

    size_t indexOf(size_t x, size_t y) const {
        assert(x < width && y < height);
        size_t idx = (y * width) + x;
        return idx;
    }
};

using PixelBufferImageCallback = std::function<void(const PixelBuffer &)>;
using PixelBufferScanlineCallback = std::function<void(const PixelBuffer &, size_t lineNum)>;

}

#endif /* PixelBuffer_hpp */
