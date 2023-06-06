//
//  M1-OrientationManager
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
    Orientation orientation;
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
	float yaw = 0;
	float pitch = 0;
	float roll = 0; 
	bool isConnected = false;

    int setup() override {
        refreshDevices();
        return 1;
    }

    int update() override {
        if (isConnected){
			M1OrientationYPR newOrientation;
			newOrientation.yaw = yaw; 
			newOrientation.pitch = pitch; 
			newOrientation.roll = roll; 
			newOrientation.angleType = M1OrientationYPR::AngleType::DEGREES;
			orientation.setYPR(newOrientation);

			yaw = fmod((yaw + 0.1), 360);
            return 1;
        } else {
            // return for error handling
            return -1;
        }
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
        // clear device list
        devices.clear();
		devices.push_back({ "Emulator", M1OrientationDeviceType::M1OrientationManagerDeviceTypeEmulator, "" });
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback) override {
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
    
    // Callback update from the SupperwareInterface object
//    std::vector<float> trackerChanged(const HeadMatrix& headMatrix) override {
//
//    }
};
