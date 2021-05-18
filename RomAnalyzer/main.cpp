//
//  main.cpp
//  RomAnalyzer
//
//  Created on 4/26/20.
//

#include <iostream>
#include <fstream>
#include <cassert>
#include "CartridgeHeader.hpp"

using namespace std;
using namespace MikoGB;

static const size_t SixteenKB = 16384;

int main(int argc, const char * argv[]) {
    string filename;
    if (argc >= 2) {
        filename = argv[1];
    } else {
        cout << "Enter filename: " << endl;
        cin >> filename;
    }
    
    ifstream rom(filename);
    
    //first 16KB from the rom data. This is the permanently mapped region
    //and the region that contains the nintendo logo and cartridge header
    char *romBytes = new char[SixteenKB]();
    rom.read(romBytes, SixteenKB);
    
    CartridgeHeader header;
    header.readHeaderData((uint8_t *)romBytes);
    cout << header << endl;
    
    delete [] romBytes;
    return 0;
}
