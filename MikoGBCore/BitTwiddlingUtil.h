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

inline void splitWord16(uint16_t word, uint8_t &lo, uint8_t &hi) {
    lo = (word & 0xFF);
    hi = (word & 0xFF00) >> 8;
}

inline bool isMaskSet(uint8_t byte, uint8_t mask) {
    bool isSet = ((byte & mask) == mask);
    return isSet;
}

#endif /* BitTwiddlingUtil_h */
