//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <stdio.h>
#include <string>

class M1Interface {
public:
    M1Interface();
    ~M1Interface();

    struct CalcResult {
        bool gotY = false;
        bool gotP = false;
        bool gotR = false;
        float y, p, r;
    };
    
    static CalcResult decode3PieceString(std::string stringToDecode, char symbolY, char symbolP, char symbolR, int numberOfDecimalPlaces);
};
