#pragma once

#include <JuceHeader.h>
#include "M1OrientationHardwareAbstract.h"

class M1OrientationHardwareBluetooth : public M1OrientationHardwareAbstract {
    
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
		    "ble 1", 
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
  
