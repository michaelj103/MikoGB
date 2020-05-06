//
//  BitTwiddlingUtil.h
//  MikoGB
//
//  Created on 5/5/20.
//

#ifndef BitTwiddlingUtil_h
#define BitTwiddlingUtil_h

inline uint16_t word16(uint8_t lo, uint8_t hi) {
    uint16_t word = hi;
    word = (word << 8) | lo;
    return word;
}

#endif /* BitTwiddlingUtil_h */
