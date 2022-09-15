//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "M1Interface.h"

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
};
