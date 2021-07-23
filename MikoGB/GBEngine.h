//
//  GBEngine.h
//  MikoGB
//
//  Created on 5/8/21.
//

#import <Foundation/Foundation.h>

@protocol GBEngineImageDestination;
@protocol GBEngineAudioDestination;
@protocol GBEngineObserver;

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

- (void)registerObserver:(id<GBEngineObserver>)observer;
- (void)unregisterObserver:(id<GBEngineObserver>)observer;

- (void)loadROM:(NSURL *)url completion:(void (^_Nullable)(BOOL))completion;
- (void)writeDisplayStateToDirectory:(NSURL *)directoryURL completion:(void (^_Nullable)(BOOL))completion;

- (void)emulateFrame;

- (void)setDesiredRunnable:(BOOL)runnable completion:(void (^_Nullable)(void))completion;
@property (nonatomic) BOOL desiredRunnable;
@property (readonly, nonatomic, getter=isRunnable) BOOL runnable;

- (void)setKeyDown:(GBEngineKeyCode)keyCode;
- (void)setKeyUp:(GBEngineKeyCode)keyCode;

@end


@protocol GBEngineImageDestination <NSObject>
- (void)engine:(GBEngine *)engine receivedFrame:(CGImageRef)frame;
@end

@protocol GBEngineObserver <NSObject>
- (void)engine:(GBEngine *)engine runnableDidChange:(BOOL)isRunnable;
@end

NS_ASSUME_NONNULL_END
