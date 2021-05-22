//
//  GBEngine.h
//  MikoGB
//
//  Created on 5/8/21.
//

#import <Foundation/Foundation.h>

@protocol GBEngineImageDestination;
@protocol GBEngineAudioDestination;

typedef NS_ENUM(NSInteger, GBEngineKeyCode) {
    GBEngineKeyCodeRight,
    GBEngineKeyCodeLeft,
    GBEngineKeyCodeUp,
    GBEngineKeyCodeDown,
    GBEngineKeyCodeA,
    GBEngineKeyCodeB,
    GBEngineKeyCodeSelect,
    GBEngineKeyCodeStart,
};

NS_ASSUME_NONNULL_BEGIN

@interface GBEngine : NSObject

@property (nonatomic, weak) id<GBEngineImageDestination> imageDestination;
@property (nonatomic, weak) id<GBEngineAudioDestination> audioDestination;

- (void)loadROM:(NSURL *)url completion:(void (^_Nullable)(BOOL))completion;
- (void)writeDisplayStateToDirectory:(NSURL *)directoryURL completion:(void (^_Nullable)(BOOL))completion;

- (void)emulateFrame;

- (void)setKeyDown:(GBEngineKeyCode)keyCode;
- (void)setKeyUp:(GBEngineKeyCode)keyCode;

@end


@protocol GBEngineImageDestination <NSObject>
- (void)engine:(GBEngine *)engine receivedFrame:(CGImageRef)frame;
@end

NS_ASSUME_NONNULL_END
