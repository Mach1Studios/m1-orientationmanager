#pragma once

#include <JuceHeader.h>
#include "M1OrientationHardwareAbstract.h"

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


class M1OrientationHardwareSerial : public M1OrientationHardwareAbstract {

    Orientation orientation;

    std::string currentDevice;
    std::vector<std::string> devices;

public:
    void setup() override {

    }

    void update() override {
        // generate random angles
        float yaw = 180 * sin(juce::Time::currentTimeMillis() / 100000.0);
        float pitch = 180 * cos(juce::Time::currentTimeMillis() / 100000.0 - 0.3);
        float roll = 180 * sin(juce::Time::currentTimeMillis() / 100000.0 + 0.1);

        orientation.setYPR({ yaw, pitch, roll });
    }

    void close() override {

    }

    M1OrientationTrackingResult getOrientation() override {
        M1OrientationTrackingResult result;
        result.currentOrientation = orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
        devices = {
            "serial 1",
            "serial 2",
        };
    }

    std::vector<std::string> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(std::string device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        currentDevice = device;
        statusCallback(true, "ok");
    }

    std::string getCurrentDevice() override {
        return currentDevice;
    }

};