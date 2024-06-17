//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

// Concept:
//   Search for device and filter by device name to expected device Interface
//   Connect to any device freely (some devices have specific connection requirements like the Supperware)
//   Update devices and use name filtering again to do specific update routines

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h" 

class HardwareEmulator : public HardwareAbstract {
public:
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    bool isConnected = false;

    float yaw = 0;
    float pitch = 0;
    float roll = 0;
    Mach1::Orientation orientation;  // intended to hold a range between -1 and 1
    Mach1::Float3 current; // intended to hold euler degrees
    Mach1::Float3 previous; // intended to hold euler degrees
    
    int setup() override {
        refreshDevices();
        return 1;
    }

    int update() override {
        if (!isConnected){
            return -1;
        }

        // increment values
        yaw = std::fmod((yaw + 0.1), 360); // fmod 360 range
        if (yaw > 180) {
            yaw -= 360; // shift to -180
        }

        pitch = std::fmod((pitch + 0.1), 90); // fmod 90 range

        current = {pitch, yaw, roll};
        orientation.ApplyRotationDegrees(current - previous);

        // store previous value
        previous = current;

        return 1;
    }
    
    void calibrateDevice() override {
        yaw = 0;
        pitch = 0;
        roll = 0;
    }
    
    int close() override {
        isConnected = false;
        return 1;
    }

    M1OrientationTrackingResult getOrientation() override {
        M1OrientationTrackingResult result;
        result.currentOrientation = orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
		std::vector<M1OrientationDeviceInfo> devices;
		devices.push_back({ "Emulator", M1OrientationDeviceType::M1OrientationManagerDeviceTypeEmulator, "" });

		lock();
		this->devices = devices;
		unlock();
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, TrackingCallback statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            
            int comPort = matchedDevice - devices.begin(); // get the device index of all found serial devices
            
            if (matchedDevice->getDeviceName().find("Emulator") != std::string::npos) {
                connectedDevice = *matchedDevice;
                isConnected = true;
                statusCallback(true, "Emulator: Connected", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
                return;
              
            }
        }
        statusCallback(false, "Emulator: Not found", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
    }

    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }

    void recenter() override {
        orientation.Recenter();
    }
    
    void setAdditionalDeviceSettings(std::string settingsChange) override {
        // Fill device specific instructions here...
    }
};
