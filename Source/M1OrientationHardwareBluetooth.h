#pragma once

#include <JuceHeader.h>
#include <juce_events/juce_events.h>

#include "m1_orientation_manager/M1OrientationHardwareAbstract.h"

class M1OrientationHardwareBluetooth : public M1OrientationHardwareAbstract {
    
    M1GlobalOrientation orientation;

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
        result.orientation = orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
        devices = { 
		    "device1", 
		    "device2",
        };
    }

    std::vector<std::string> getDevices() override {
        return devices;
    }
    
    void startTrackingUsingDevice(std::string device) override {
        currentDevice = device;
    }

    std::string getCurrentDevice() override {
        return currentDevice;
    }

};
  