//
//  HardwareDeviceInfo.h
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#ifndef HardwareDeviceInfo_h
#define HardwareDeviceInfo_h

/// Class for collecting devices
class HardwareDeviceInfo {
public:
    int index;
    std::string path;
    std::string name;
    M1OrientationStatusType state;
    int rssi;
    M1OrientationDeviceType type;
    
    HardwareDeviceInfo(std::string deviceName, std::string devicePath, int deviceIndex, M1OrientationStatusType deviceState, int deviceRssi, M1OrientationDeviceType deviceType){
        index = deviceIndex;
        path = devicePath;
        name = deviceName;
        state = deviceState;
        rssi = deviceRssi;
        type = deviceType;
    }
    HardwareDeviceInfo(){
        name = "undefined";
        path = "undefined";
        index = -1;
        state = M1OrientationStatusType::M1OrientationManagerStatusTypeUnknown;
        rssi = -1;
        type = M1OrientationDeviceType::M1OrientationManagerDeviceTypeNone;
    }
    std::string getDevicePath(){
        return path;
    }
    std::string getDeviceName(){
        return name;
    }
    int getDeviceID(){
        return index;
    }
    int getDeviceState(){
        return state;
    }
    int getDeviceRssi(){
        return rssi;
    }
    M1OrientationDeviceType getDeviceType(){
        return type;
    }
};

struct find_id : std::unary_function<HardwareDeviceInfo, bool> {
    std::string name;
    find_id(std::string name):name(name) { }
    bool operator()(HardwareDeviceInfo const& m) const {
        return m.name == name;
    }
};

#endif /* HardwareDeviceInfo_h */
