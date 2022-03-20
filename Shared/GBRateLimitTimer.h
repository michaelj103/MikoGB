//
//  GBRateLimitTimer.h
//  MikoGB
//
//  Created by Michael Brandt on 3/6/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface GBRateLimitTimer : NSObject

- (instancetype)initWithDelay:(NSTimeInterval)delay targetQueue:(dispatch_queue_t)targetQueue eventBlock:(dispatch_block_t)eventBlock;
@property (readonly, nonatomic) NSTimeInterval delay;
@property (readonly, nonatomic) dispatch_queue_t targetQueue;
@property (readonly, nonatomic, copy) dispatch_block_t eventBlock;

@property (readonly, nonatomic) BOOL isPending;

/// call when the rate limited event occurs
- (void)input;

/// no longer emit events. Once cancelled a timer can't be reused
- (void)cancel;

- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
