//
//  Breakpoint.cpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/18/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#include "Breakpoint.hpp"

using namespace std;
using namespace MikoGB;

void BreakpointManager::removeBreakpoint(size_t id) {
    auto it = _lineBreakpoints.begin();
    while (it != _lineBreakpoints.end()) {
        if (it->id == id) {
            _lineBreakpoints.erase(it);
            break;
        }
        it++;
    }
    
    if (_lineBreakpoints.empty()) {
        _hasBreakpoints = false;
    }
}

void BreakpointManager::addLineBreakpoint(int bank, uint16_t address) {
    size_t id = _getNextID();
    LineBreakpoint bp = { id, bank, address };
    _lineBreakpoints.insert(bp);
    _hasBreakpoints = true;
}

bool BreakpointManager::hasLineBreakpoint(int bank, uint16_t address) const {
    LineBreakpoint bp = { SIZE_T_MAX, bank, address };
    auto it = _lineBreakpoints.find(bp);
    return it != _lineBreakpoints.end();
}
