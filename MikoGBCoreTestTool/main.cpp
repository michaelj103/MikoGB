//
//  main.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include <iostream>
#include <fstream>
#include "GameboyCore.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    MikoGB::GameBoyCore gbCore;
    while (gbCore.getPC() < 0xfa) {
        gbCore.step();
    }
    
    return 0;
}
