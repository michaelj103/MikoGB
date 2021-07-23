//
//  GameBoyCoreTypes.h
//  MikoGB
//
//  Created on 5/26/21.
//

#ifndef GameBoyCoreTypes_h
#define GameBoyCoreTypes_h

namespace MikoGB {

enum class JoypadButton {
    Right = 0,
    Left = 1,
    Up = 2,
    Down = 3,
    A = 4,
    B = 5,
    Select = 6,
    Start = 7
};

using RunnableChangedCallback = std::function<void(bool isRunnable)>;

}

#endif /* GameBoyCoreTypes_h */
