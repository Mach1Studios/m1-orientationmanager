//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "m1_orientation_client/M1OrientationTypes.h"
#include "m1_orientation_client/M1OrientationSettings.h"

class HardwareAbstract {
	std::mutex mtx;

public:
    virtual int setup() = 0;
    virtual int update() = 0;
    virtual int close() = 0;
    
    std::function<void()> orientationDeviceLostCallback = [](){};

	void lock() {
		mtx.lock();
	}

	void unlock() {
		mtx.unlock();
	}

    virtual std::vector<M1OrientationDeviceInfo> getDevices() = 0;
    virtual void refreshDevices() = 0;
    virtual M1OrientationDeviceInfo getConnectedDevice() = 0;
    virtual M1OrientationTrackingResult getOrientation() = 0;
    virtual void calibrateDevice() = 0;

    virtual void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> callback) = 0;
    
};
