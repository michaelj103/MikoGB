//
//  RingBuffer.swift
//  GBCoreTester
//
//  Created by Michael Brandt on 4/29/20.
//  Copyright © 2020 Michael Brandt. All rights reserved.
//

import Foundation

struct RingBuffer<T> {
    
    private var _buffer: [T]
    private var _readIdx: Int = 0
    private var _writeIdx: Int = 0
    
    var capacity: Int {
        get {
            return _buffer.count
        }
    }
    
    private(set) var count: Int = 0 {
        didSet {
            isFull = (count == _buffer.count)
        }
    }
    private(set) var isFull: Bool = false
    
    init(repeating: T, count: Int) {
        _buffer = [T](repeating: repeating, count: count)
    }
    
    mutating func write(_ value: T) {
        _buffer[_writeIdx] = value
        _writeIdx = (_writeIdx + 1) % _buffer.count
        if (isFull) {
            //already full, move the read pointer to match write
            _readIdx = _writeIdx
        } else {
            count += 1
        }
    }
    
    mutating func write(_ values: [T]) {
        //simple implementation. Revisit if batch writes are actually needed
        //Or rewrite in C/C++ like some kind of animal
        for val in values {
            write(val)
        }
    }
    
    mutating func read() throws -> T {
        guard count > 0 else {
            throw SimpleError("Reading from an empty ring buffer")
        }
        let tmp = _buffer[_readIdx]
        _readIdx = (_readIdx + 1) % _buffer.count;
        count -= 1
        return tmp
    }
    
    mutating func read<MC: MutableCollection>(into buf: inout MC) throws
        where MC.Element == T, MC.Index == Int
    {
        try read(into: &buf, count: buf.count)
    }
    
    mutating func read<MC: MutableCollection>(into buf: inout MC, count: Int) throws
        where MC.Element == T, MC.Index == Int
    {
        let readSize = min(count, buf.count)
        for i in 0..<readSize {
            buf[i] = try read()
        }
    }
}

