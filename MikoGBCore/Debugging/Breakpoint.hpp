//
//  Breakpoint.hpp
//  MikoGBCore
//
//  Created by Michael Brandt on 2/18/22.
//  Copyright © 2022 Michael Brandt. All rights reserved.
//

#ifndef Breakpoint_hpp
#define Breakpoint_hpp

#include <set>

namespace MikoGB {

struct LineBreakpoint {
    size_t id;
    int romBank;
    uint16_t address;
};

class BreakpointManager {
public:
    bool hasBreakpoints() const { return _hasBreakpoints; }
    
    void removeBreakpoint(size_t id);
    
    void addLineBreakpoint(int bank, uint16_t address);
    bool hasLineBreakpoint(int bank, uint16_t address) const;
    
private:
    bool _hasBreakpoints = false;
    size_t _getNextID() { return _nextID++; }
    size_t _nextID = 0;
    std::set<LineBreakpoint> _lineBreakpoints;
};

inline bool operator<(const LineBreakpoint &a, const LineBreakpoint &b) {
    if (a.romBank == b.romBank) {
        return a.address < b.address;
    }
    return a.romBank < b.romBank;
}

}

#endif /* Breakpoint_hpp */
