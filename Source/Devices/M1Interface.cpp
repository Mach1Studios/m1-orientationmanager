//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "M1Interface.h"

M1Interface::M1Interface()
{
}

M1Interface::~M1Interface()
{
}

void M1Interface::setDeviceTrailingPoints(int trailingpoints){
    trailingPoints = trailingpoints;
}

M1Interface::CalcResult M1Interface::DecodeIMUData(unsigned char chrTemp[])
{
    switch(chrTemp[1])
    {
        case 0x53:
        Angle[0] = ((short) (chrTemp[3]<<8|chrTemp[2]))/32768.0*180;
        Angle[1] = ((short) (chrTemp[5]<<8|chrTemp[4]))/32768.0*180;
        Angle[2] = ((short)(chrTemp[7]<<8|chrTemp[6]))/32768.0*180;
        T = ((short)(chrTemp[9]<<8|chrTemp[8]))/340.0+36.25;
        printf("Angle = %4.2f\t%4.2f\t%4.2f\tT=%4.2f\r\n",Angle[0],Angle[1],Angle[2],T);

        M1Interface::CalcResult success;
        success.y = (float)Angle[2];
        success.gotY = true;
        success.p = (float)Angle[1] + 90;
        success.gotP = true;
        success.r = 0;
        success.gotR = true;
        return success;
    }
    M1Interface::CalcResult fail;
    return fail;
}

M1Interface::CalcResult M1Interface::decode3PieceString(std::string stringToDecode, char symbolY, char symbolP, char symbolR, int numberOfDecimalPlaces) {
    int cursor = 0;
    
    struct utility {
        static bool decodeNumberAtPoint(std::string str, int startIndex, int minDigitsAfterDot, std::string & decodedNumber) {
            int cursor = startIndex + 1;
            std::string numberString = "";
            bool stringFinished = false;
            int afterDotCounter = -99999;
            
            while ((cursor < str.length()) && (!stringFinished)) {
                if ((std::isdigit(str[cursor])) || (str[cursor] == '.') || (str[cursor] == '-'))
                    numberString += str[cursor];
                else stringFinished = true;
                
                if (str[cursor] == '.')
                    afterDotCounter = 0;
                else {
                    if (std::isdigit(str[cursor]))
                        afterDotCounter++;
                }
                cursor++;
            }
            decodedNumber = numberString;
            
            if (afterDotCounter >= minDigitsAfterDot) {
                return true;
            } else return false;
        }
    };
    
    M1Interface::CalcResult success;
    
    while (cursor < stringToDecode.size()) {
        const char currentChar = stringToDecode.at(cursor);
        std::string decodedNumber = "";
        
        if (currentChar == symbolY) {
            if (!utility::decodeNumberAtPoint(stringToDecode, cursor, numberOfDecimalPlaces, decodedNumber)) {
            } else { // succeeded getting the number and there is enough decimals
                success.gotY = true;
                success.y = std::stof(decodedNumber);
            }
        }
        if (currentChar == symbolP) {
            if (!utility::decodeNumberAtPoint(stringToDecode, cursor, numberOfDecimalPlaces, decodedNumber)) {
            } else { // succeeded getting the number and there is enough decimals
                success.gotP = true;
                success.p = std::stof(decodedNumber);
            }
        }
        if (currentChar == symbolR) {
            if (!utility::decodeNumberAtPoint(stringToDecode, cursor, numberOfDecimalPlaces, decodedNumber)) {
            } else { // succeeded getting the number and there is enough decimals
                success.gotR = true;
                success.r = std::stof(decodedNumber);
            }
        }
        cursor++;
    }
    return success;
}

bool M1Interface::getNewDataFromQueue(float & Y, float & P, float & R) {
    auto decoded = decode3PieceString(queueString, 'Y', 'P', 'R', trailingPoints);
    bool anythingNewDetected = false;
    
    if (decoded.gotY) {
        Y = decoded.y;
        anythingNewDetected = true;
    }
    
    if (decoded.gotP) {
        P = decoded.p;
        anythingNewDetected = true;
    }
    
    if (decoded.gotR) {
        R = decoded.r;
        anythingNewDetected = true;
    }
    return anythingNewDetected;
}
