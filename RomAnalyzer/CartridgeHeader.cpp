//
//  CartridgeHeader.hpp
//  MikoGB
//
//  Created on 4/26/20.
//

#include "CartridgeHeader.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace MikoGB;

static bool _PrintableAscii(uint8_t byte) {
    return byte == 0 || (byte >= 0x20 && byte < 0x7F);
}

static bool _ValidateLogoHeader(uint8_t *bytes) {
    static const size_t LogoSize = 48;
    static const uint8_t LogoHeader[LogoSize] = { 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E };
    
    //Logo header is in 0x104 - 0x133
    for (size_t i = 0; i < LogoSize; ++i) {
        if (bytes[0x104 + i] != LogoHeader[i]) {
            return false;
        }
    }
    return true;
}

static CartridgeType _CartridgeTypeFromByte(uint8_t byte) {
    switch (byte) {
        case 0x00:
            return CartridgeType::ROM_Only;
        case 0x01:
            //e.g. Super Mario Land
            return CartridgeType::MBC1;
        case 0x13:
            //e.g. Pokemon Red/Blue
            return CartridgeType::MBC3_RAM_BATT;
        case 0x1B:
            //e.g. Pokemon Yellow
            return CartridgeType::MBC5_RAM_BATT;
        default:
            return CartridgeType::Unsupported;
    }
}

static string _CartridgeTypeDescription(uint8_t byte) {
    CartridgeType type = _CartridgeTypeFromByte(byte);
    switch (type) {
        case CartridgeType::ROM_Only:
            return "ROM ONLY";
        case CartridgeType::MBC1:
            //e.g. Super Mario Land
            return "MBC1";
        case CartridgeType::MBC3_RAM_BATT:
            //e.g. Pokemon Red/Blue
            return "MBC3+RAM+BATTERY";
        case CartridgeType::MBC5_RAM_BATT:
            //e.g. Pokemon Yellow
            return "MBC5+RAM+BATTERY";
        case CartridgeType::Unsupported:
            return "Unsupported " + to_string((int)byte);
    }
}

static CartridgeROMSize _CartridgeROMSizeFromByte(uint8_t byte) {
    switch (byte) {
        case 0x00:
            return CartridgeROMSize::BANKS_0;
        case 0x01:
            return CartridgeROMSize::BANKS_4;
        case 0x02:
            return CartridgeROMSize::BANKS_8;
        case 0x03:
            return CartridgeROMSize::BANKS_16;
        case 0x04:
            return CartridgeROMSize::BANKS_32;
        case 0x05:
            return CartridgeROMSize::BANKS_64;
        case 0x06:
            return CartridgeROMSize::BANKS_128;
        case 0x07:
            return CartridgeROMSize::BANKS_256;
        case 0x08:
            return CartridgeROMSize::BANKS_512;
        case 0x52: // 1.1 MiB / 72 Banks
        case 0x53: // 1.2 MiB / 80 Banks
        case 0x42: // 1.5 MiB / 96 Banks
        default:
            return CartridgeROMSize::Unsupported;
    }
}

static string _CartridgeROMSizeDescription(uint8_t byte) {
    CartridgeROMSize size = _CartridgeROMSizeFromByte(byte);
    switch (size) {
        case CartridgeROMSize::BANKS_0:
            return "32 KiB (No Bank Switching)";
        case CartridgeROMSize::BANKS_4:
            return "64 KiB (4 Banks)";
        case CartridgeROMSize::BANKS_8:
            return "128 KiB (8 Banks)";
        case CartridgeROMSize::BANKS_16:
            return "256 KiB (16 Banks)";
        case CartridgeROMSize::BANKS_32:
            return "512 KiB (32 Banks)";
        case CartridgeROMSize::BANKS_64:
            return "1 MiB (64 Banks)";
        case CartridgeROMSize::BANKS_128:
            return "2 MiB (128 Banks)";
        case CartridgeROMSize::BANKS_256:
            return "4 MiB (256 Banks)";
        case CartridgeROMSize::BANKS_512:
            return "8 MiB (512 Banks)";
        case CartridgeROMSize::Unsupported:
            return "Unsupported " + to_string((int)byte);
    }
}

static CartridgeRAMSize _CartridgeRAMSizeFromByte(uint8_t byte) {
    switch (byte) {
        case 0x00:
            //Random quirk: could also mean MBC2
            return CartridgeRAMSize::RAM0;
        case 0x01:
            return CartridgeRAMSize::RAM2KB;
        case 0x02:
            return CartridgeRAMSize::RAM8KB;
        case 0x03:
            return CartridgeRAMSize::RAM32KB;
        //Yes, the last two are out of order for some reason
        case 0x04:
            return CartridgeRAMSize::RAM128KB;
        case 0x05:
            return CartridgeRAMSize::RAM64KB;
        default:
            return CartridgeRAMSize::Unsupported;
    }
}

static string _CartridgeRAMSizeDescription(uint8_t byte) {
    CartridgeRAMSize ramSize = _CartridgeRAMSizeFromByte(byte);
    switch (ramSize) {
        case CartridgeRAMSize::RAM0:
            return "None";
        case CartridgeRAMSize::RAM2KB:
            return "2 KiB";
        case CartridgeRAMSize::RAM8KB:
            return "8 KiB";
        case CartridgeRAMSize::RAM32KB:
            return "32 KiB";
        case CartridgeRAMSize::RAM64KB:
            return "64 KiB";
        case CartridgeRAMSize::RAM128KB:
            return "128 KiB";
        case CartridgeRAMSize::Unsupported:
            return "Unsupported " + to_string((int)byte);
    }
}

CartridgeHeader::CartridgeHeader(uint8_t *romData) {
    // Check that the logo is in the correct spot
    _validLogo = _ValidateLogoHeader(romData);
    
    // Determine GameBoy Color support
    uint8_t CGBByte = romData[0x143];
    _cgbSupported = (CGBByte & 0x80) != 0;
    _cgbExclusive = (CGBByte & 0xC0) != 0;
    
    // Determine Super GameBoy support
    uint8_t SGBByte = romData[0x146];
    _sgbSupported = (SGBByte == 0x03);
    
    // Get Title and Manufacturer Code
    if (_cgbSupported) {
        //On CGB, title is 11 characters, uppercase ASCII
        char titleChar[12];
        titleChar[11] = '\0';
        for (size_t i = 0; i < 11; ++i) {
            uint8_t byte = romData[0x134 + i];
            assert(_PrintableAscii(byte));
            titleChar[i] = byte;
        }
        _title = string(titleChar);
        char mfgChar[5];
        mfgChar[4] = '\0';
        for (size_t i = 0; i < 4; ++i) {
            uint8_t byte = romData[0x13F + i];
            assert(_PrintableAscii(byte));
            mfgChar[i] = byte;
        }
        _mfgCode = string(mfgChar);
    } else {
        //If not CGB, title is 16 characters, uppercase ASCII
        char titleChar[17];
        titleChar[16] = '\0';
        for (size_t i = 0; i < 16; ++i) {
            uint8_t byte = romData[0x134 + i];
            assert(_PrintableAscii(byte));
            titleChar[i] = byte;
        }
        _title = string(titleChar);
    }
    
    // Publisher Code
    uint8_t oldPublisherCode = romData[0x14B];
    // An old publisher code of 0x33 indicates use of the new codes
    if (oldPublisherCode == 0x33) {
        _publisherCode += romData[0x144];
        _publisherCode += romData[0x145];
    } else {
        // Treat old code as string from hex value of publisher code byte
        ostringstream oss;
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)oldPublisherCode;
        _publisherCode = oss.str();
    }
    
    // Cartridge storage info
    _cartridgeType = romData[0x147];
    _romSize = romData[0x148];
    _ramSize = romData[0x149];
    _destinationCode = romData[0x14A];
    _version = romData[0x14C];
    
    // Header checksum
    uint8_t checksum = 0;
    for (size_t i = 0x134; i <= 0x14C; ++i) {
        checksum = checksum - romData[i] - 1;
    }
    _headerChecksum = (romData[0x14D] == (checksum & 0xFF));
    
    // TODO: Global checksum is 0x14E-0x14F, but that's not checked here (maybe later?)
}

bool CartridgeHeader::isSupported() const {
    bool isSupported = getType() != CartridgeType::Unsupported;
    isSupported = isSupported && getROMSize() != CartridgeROMSize::Unsupported;
    isSupported = isSupported && getRAMSize() != CartridgeRAMSize::Unsupported;
    isSupported = isSupported && (_destinationCode <= 0x01);
    return isSupported;
}

CartridgeType CartridgeHeader::getType() const {
    return _CartridgeTypeFromByte(_cartridgeType);
}

CartridgeROMSize CartridgeHeader::getROMSize() const {
    return _CartridgeROMSizeFromByte(_romSize);
}

CartridgeRAMSize CartridgeHeader::getRAMSize() const {
    return _CartridgeRAMSizeFromByte(_ramSize);
}

namespace MikoGB {
std::ostream &operator<<(std::ostream &os, const CartridgeHeader &cartridge) {
    cout << "Cartridge Info:\n=======================";
    cout << "\nTitle: " << cartridge._title;
    cout << "\nMFG Code: " << (cartridge._mfgCode.size() > 0 ? cartridge._mfgCode : "(N/A)");
    cout << "\nRegion: " << (cartridge._destinationCode == 0x00 ? "Japanese" : "Non-Japanese");
    cout << "\nVersion: " << (int)cartridge._version;
    cout << "\nHeader Checks: " << (cartridge._validLogo && cartridge._headerChecksum ? "Valid" : "Invalid");
    cout << "\nSGB Support: " << (cartridge._sgbSupported ? "Supported" : "Unsupported");
    cout << "\nCGB Support: " << (cartridge._cgbExclusive ? (cartridge._cgbExclusive ? "Required" : "Supported") : "Unsupported");
    cout << "\nPublisher Code: " << cartridge._publisherCode;
    cout << "\nCartridge Type: " << _CartridgeTypeDescription(cartridge._cartridgeType);
    cout << "\nROM Size: " << _CartridgeROMSizeDescription(cartridge._romSize);
    cout << "\nRAM Size: " << _CartridgeRAMSizeDescription(cartridge._ramSize);
    
    return os;
}
}


