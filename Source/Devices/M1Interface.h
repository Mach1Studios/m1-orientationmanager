//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <stdio.h>
#include <string>
#include <vector>

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
    
    double a[3],w[3],Angle[3],T;
    int trailingPoints = 4;
    float lastY = 0, lastP = 0, lastR = 0;
    // can use this class to hold and manage queue if needed
    std::vector<unsigned char> queueBuffer;
    std::string queueString = "";
    
    void setDeviceTrailingPoints(int trailingpoints);
    static CalcResult decode3PieceString(std::string stringToDecode, char symbolY, char symbolP, char symbolR, int numberOfDecimalPlaces);
    CalcResult DecodeIMUData(unsigned char chrTemp[]); // legacy direct decode of witmotion
    bool getNewDataFromQueue(float & Y, float & P, float & R);
};
