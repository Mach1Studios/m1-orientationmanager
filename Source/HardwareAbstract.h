//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "m1_orientation_client/M1OrientationTypes.h"
#include "m1_orientation_client/M1OrientationSettings.h"

class HardwareAbstract {
public:
    virtual void setup() = 0;
    virtual void update() = 0;
    virtual void close() = 0;
    
    std::function<void()> orientationDeviceLostCallback = [](){};

    virtual M1OrientationTrackingResult getOrientation() = 0;
    
    virtual void refreshDevices() = 0;
    virtual std::vector<M1OrientationDeviceInfo> getDevices() = 0;

    virtual M1OrientationDeviceInfo getConnectedDevice() = 0;
    virtual void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string errorMessage)> callback) = 0;
    
};
