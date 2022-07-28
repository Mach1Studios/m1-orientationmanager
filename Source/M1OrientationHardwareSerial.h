#include <JuceHeader.h>

/// Class for collecting devices
class SerialDeviceInfo {
public:
    std::string devicePath;
    std::string deviceName;
    // index for device, connection state for device
    int deviceID, deviceState;
    
    SerialDeviceInfo(std::string devicePathIn, std::string deviceNameIn, int deviceIDIn, int deviceStateIn){
        devicePath = devicePathIn;
        deviceName = deviceNameIn;
        deviceID = deviceIDIn;
        deviceState = deviceStateIn;
    }
    SerialDeviceInfo(){
        deviceName = "device undefined";
        deviceID = -1;
        deviceState = -1;
    }
    std::string getDevicePath(){
        return devicePath;
    }
    std::string getDeviceName(){
        return deviceName;
    }
    int getDeviceID(){
        return deviceID;
    }
    int getDeviceState(){
        return deviceState;
    }
};