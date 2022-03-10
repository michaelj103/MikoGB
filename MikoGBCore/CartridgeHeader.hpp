//
//  CartridgeHeader.hpp
//  MikoGB
//
//  Created on 4/26/20.
//

#ifndef CartridgeHeader_hpp
#define CartridgeHeader_hpp

#include <iostream>
#include <string>

namespace MikoGB {

enum class CartridgeType {
    Unsupported,
    ROM_Only,
    MBC1,
    MBC1_RAM,
    MBC1_RAM_BATT,
    MBC3_RAM_BATT,
    MBC5_RAM_BATT,
};

enum class CartridgeROMSize {
    Unsupported,
    BANKS_0, ///< 32 KiB
    BANKS_4, ///< 64 KiB
    BANKS_8, ///< 128 KiB
    BANKS_16, ///< 256 KiB
    BANKS_32, ///< 512 KiB
    BANKS_64, ///< 1 MiB
    BANKS_128, ///< 2 MiB
    BANKS_256, ///< 4 MiB
    BANKS_512, ///< 8 MiB
};

enum class CartridgeRAMSize {
    Unsupported,
    RAM0,
    RAM2KB,
    RAM8KB,
    RAM32KB,
    RAM64KB,
    RAM128KB,
};

class CartridgeHeader {
public:
    CartridgeHeader() = default;
    void readHeaderData(uint8_t *romData);
    
    bool isSupported() const;
    
    CartridgeType getType() const;
    CartridgeROMSize getROMSize() const;
    CartridgeRAMSize getRAMSize() const;
    bool hasBatteryBackup() const;
    
private:
    std::string _title;
    std::string _mfgCode;
    bool _validLogo;
    bool _cgbSupported;
    bool _cgbExclusive;
    bool _sgbSupported;
    std::string _publisherCode;
    uint8_t _cartridgeType;
    uint8_t _romSize;
    uint8_t _ramSize;
    uint8_t _destinationCode;
    uint8_t _version;
    bool _headerChecksum;
    
    friend std::ostream &operator<<(std::ostream &, const CartridgeHeader &);
};
}

#endif /* CartridgeHeader_hpp */
