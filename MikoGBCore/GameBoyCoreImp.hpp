//
//  GameBoyCoreImp.hpp
//  MikoGB
//
//  Created on 5/4/21.
//

#ifndef GameBoyCoreImp_hpp
#define GameBoyCoreImp_hpp

#include "GameBoyCore.hpp"
#include "CPUCore.hpp"
#include "GPUCore.hpp"

namespace MikoGB {

class GameBoyCoreImp {
public:
    GameBoyCoreImp();
    ~GameBoyCoreImp();
    
    void step();
    
private:
    CPUCore *_cpu;
    GPUCore *_gpu;
    
    friend class GameBoyCore;
};

}

#endif /* GameBoyCoreImp_hpp */
