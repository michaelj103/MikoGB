//
//  GameBoyCore.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GameBoyCore_hpp
#define GameBoyCore_hpp

#include <cstdlib>

namespace MikoGB {

class GameBoyCoreImp;

class GameBoyCore {
public:
    GameBoyCore();
    ~GameBoyCore();
    
    void step();
    
    uint16_t getPC() const;
    
private:
    GameBoyCoreImp *_imp;
};

}

#endif /* GameBoyCore_hpp */