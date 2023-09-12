//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <cctype>

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
    CalcResult decoded;
    bool anythingNewDetected = false;

    void setDeviceTrailingPoints(int trailingpoints);
    static CalcResult decode3PieceString(std::string stringToDecode, char symbolY, char symbolP, char symbolR, int numberOfDecimalPlaces);
    CalcResult DecodeIMUData(unsigned char chrTemp[]); // legacy direct decode of witmotion
    bool getNewDataFromQueue(std::string queueString, float & Y, float & P, float & R);

    float* updateOrientation(std::string queueString, std::vector<unsigned char> queueBuffer);
};
