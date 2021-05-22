//
//  GBImageUtilities.h
//  MikoGB
//
//  Created on 5/19/21.
//

#import <Foundation/Foundation.h>
#import "PixelBuffer.hpp"
#import <ImageIO/ImageIO.h>

NS_ASSUME_NONNULL_BEGIN

void writePNG(const MikoGB::PixelBuffer &pixelBuffer, NSURL * _Nullable directoryURL, NSString *filename);

NS_ASSUME_NONNULL_END
