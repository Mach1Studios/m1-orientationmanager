//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"

class HardwareOSC : public HardwareAbstract {
public:
    Orientation orientation;
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    bool isConnected = false;

    void setup() override {
        refreshDevices();
    }

    void update() override {
        if (isConnected){
            // generate random angles
            float yaw = 180 * sin(juce::Time::currentTimeMillis() / 100000.0);
            float pitch = 180 * cos(juce::Time::currentTimeMillis() / 100000.0 - 0.3);
            float roll = 180 * sin(juce::Time::currentTimeMillis() / 100000.0 + 0.1);

            orientation.setYPR({ yaw, pitch, roll });
        }
    }
    
    void close() override {
        isConnected = false;
    }

    M1OrientationTrackingResult getOrientation() override {
        M1OrientationTrackingResult result;
        result.currentOrientation = orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
        //devices.push_back({ "test 1", M1OrientationDeviceType::M1OrientationManagerDeviceTypeOSC, "test 1" });
        //devices.push_back({ "test 2", M1OrientationDeviceType::M1OrientationManagerDeviceTypeOSC, "test 2" });
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            connectedDevice = *matchedDevice;
            isConnected = true;

            statusCallback(true, "ok");
            return;
        }
        statusCallback(false, "not found");
    }

    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }

};
