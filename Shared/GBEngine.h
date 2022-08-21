//
//  GBEngine.h
//  MikoGB
//
//  Created on 5/8/21.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@protocol GBEngineImageDestination;
@protocol GBEngineAudioDestination;
@protocol GBEngineSaveDestination;
@protocol GBEngineSerialDestination;
@protocol GBEngineObserver;

NS_ASSUME_NONNULL_BEGIN

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

typedef struct _GBRegisterState {
    // registers
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint8_t A;
    
    // flags
    BOOL ZFlag;
    BOOL NFlag;
    BOOL HFlag;
    BOOL CFlag;
} GBRegisterState;

typedef void (^ROMLoadCompletion)(BOOL success, BOOL supportsSaveData);
typedef void (^RAMLoadCompletion)(BOOL success);
typedef void (^SaveDataCompletion)( NSData * _Nullable data);

@interface GBEngine : NSObject

@property (nonatomic, weak) id<GBEngineImageDestination> imageDestination;
@property (nonatomic, weak) id<GBEngineAudioDestination> audioDestination;
@property (nonatomic, weak) id<GBEngineSaveDestination> saveDestination;
@property (nonatomic, weak) id<GBEngineSerialDestination> serialDestination;

- (void)registerObserver:(id<GBEngineObserver>)observer;
- (void)unregisterObserver:(id<GBEngineObserver>)observer;

- (void)loadROM:(NSURL *)url completion:(nullable ROMLoadCompletion)completion;
- (void)loadSaveData:(NSData *)data completion:(nullable RAMLoadCompletion)completion;
- (void)getSaveData:(SaveDataCompletion)completion;
@property (readonly, nonatomic) BOOL isSaveDataStale;
- (void)staleSaveDataHandled;
- (nullable NSData *)synchronousGetSaveData;
- (void)writeDisplayStateToDirectory:(NSURL *)directoryURL completion:(void (^_Nullable)(BOOL))completion;

- (void)emulateFrame;
- (void)step:(NSInteger)stepCount; // step a given number of instructions

- (void)setDesiredRunnable:(BOOL)runnable completion:(void (^_Nullable)(void))completion;
@property (nonatomic) BOOL desiredRunnable;
@property (readonly, nonatomic, getter=isRunnable) BOOL runnable;

- (void)setKeyDown:(GBEngineKeyCode)keyCode;
- (void)setKeyUp:(GBEngineKeyCode)keyCode;

- (void)receivePulledSerialByte:(uint8_t)byte;
- (void)receivePushedSerialByte:(uint8_t)byte;

// Debugger
- (NSArray<NSString *> *)disassembledInstructions:(size_t *)currentIndex;
- (NSArray<NSString *> *)lastExecutedInstructions;
- (GBRegisterState)registerState;
- (uint8_t)readByte:(uint16_t)addr;
- (BOOL)addLineBreakpointForBank:(int)romBank address:(uint16_t)address;

@end


@protocol GBEngineImageDestination <NSObject>
- (void)engine:(GBEngine *)engine receivedFrame:(CGImageRef)frame;
@end

// Audio destination callbacks come in on the internal emulation queue because they happen so damn fast,
// it's not desirable to bounce around threads
@protocol GBEngineAudioDestination <NSObject>
- (void)engine:(GBEngine *)engine receivedAudioSampleLeft:(int16_t)left right:(int16_t)right;
@end

@protocol GBEngineSaveDestination <NSObject>
- (void)engineIsReadyToPersistSaveData:(GBEngine *)engine;
@end

/// Protocol for handling events originating from this emulator. Events are emitted on main queue
@protocol GBEngineSerialDestination <NSObject>
- (void)engine:(GBEngine *)engine pushByte:(uint8_t)byte;
- (void)engine:(GBEngine *)engine presentByte:(uint8_t)byte;
@end

@protocol GBEngineObserver <NSObject>
- (void)engine:(GBEngine *)engine runnableDidChange:(BOOL)isRunnable;
- (void)didUpdateSuspendedStateForEngine:(GBEngine *)engine;
@end


NS_ASSUME_NONNULL_END
