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

    virtual M1OrientationTrackingResult getOrientation() = 0;
    
    virtual void refreshDevices() = 0;
    virtual std::vector<M1OrientationDevice> getDevices() = 0;

    virtual M1OrientationDevice getCurrentDevice() = 0;
    virtual void startTrackingUsingDevice(M1OrientationDevice device, std::function<void(bool success, std::string errorMessage)> callback) = 0;
};
