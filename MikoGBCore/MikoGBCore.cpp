//
//  MikoGBCore.cpp
//  MikoGB
//
//  Created on 5/4/21.
//

#include <iostream>
#include "MikoGBCore.hpp"
#include "MikoGBCorePriv.hpp"

void MikoGBCore::HelloWorld(const char * s)
{
    MikoGBCorePriv *theObj = new MikoGBCorePriv;
    theObj->HelloWorldPriv(s);
    delete theObj;
};

void MikoGBCorePriv::HelloWorldPriv(const char * s) 
{
    std::cout << s << std::endl;
};

