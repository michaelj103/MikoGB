//
//  GBRateLimitTimer.m
//  MikoGB
//
//  Created by Michael Brandt on 3/6/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#import "GBRateLimitTimer.h"
#import <os/lock.h>

@implementation GBRateLimitTimer {
    BOOL _isWaiting;
    BOOL _isCancelled;
    CFAbsoluteTime _lastEventTime;
    os_unfair_lock _stateLock;
}

- (instancetype)initWithDelay:(NSTimeInterval)delay targetQueue:(dispatch_queue_t)targetQueue eventBlock:(dispatch_block_t)eventBlock {
    self = [super init];
    if (self) {
        _delay = delay;
        _targetQueue = targetQueue;
        _eventBlock = [eventBlock copy];
        _stateLock = OS_UNFAIR_LOCK_INIT;
    }
    return self;
}

- (void)_handleTimer {
    CFAbsoluteTime currentTime = CFAbsoluteTimeGetCurrent();
    CFAbsoluteTime lastTime = 0.0;
    BOOL isCancelled = NO;
    os_unfair_lock_lock(&_stateLock);
    lastTime = _lastEventTime;
    isCancelled = _isCancelled;
    os_unfair_lock_unlock(&_stateLock);
    
    if (isCancelled) {
        return;
    }
    
    NSTimeInterval delaySeconds = [self delay];
    NSTimeInterval difference = currentTime - lastTime;
    if (difference >= delaySeconds) {
        // fire event!
        os_unfair_lock_lock(&_stateLock);
        _isWaiting = NO;
        os_unfair_lock_unlock(&_stateLock);
        [self eventBlock]();
    } else {
        // we need to wait a little longer
        dispatch_queue_t targetQueue = [self targetQueue];
        NSTimeInterval furtherDelay = delaySeconds - difference;
        __weak typeof(self) weakSelf = self;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(furtherDelay * NSEC_PER_SEC)), targetQueue, ^{
            [weakSelf _handleTimer];
        });
    }
}

- (void)input {
    BOOL isWaiting = NO;
    BOOL isCancelled = NO;
    CFAbsoluteTime eventTime = CFAbsoluteTimeGetCurrent();
    os_unfair_lock_lock(&_stateLock);
    isWaiting = _isWaiting;
    _lastEventTime = eventTime;
    // either already waiting or we will be momentarily. Need to toggle now to avoid race conditions
    _isWaiting = YES;
    isCancelled = _isCancelled;
    os_unfair_lock_unlock(&_stateLock);
    
    if (isCancelled) {
        return;
    }
    
    NSTimeInterval delaySeconds = [self delay];
    dispatch_queue_t targetQueue = [self targetQueue];
    if (!isWaiting) {
        __weak typeof(self) weakSelf = self;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delaySeconds * NSEC_PER_SEC)), targetQueue, ^{
            [weakSelf _handleTimer];
        });
    }
}

- (void)cancel {
    os_unfair_lock_lock(&_stateLock);
    _isCancelled = YES;
    os_unfair_lock_unlock(&_stateLock);
}

@end
