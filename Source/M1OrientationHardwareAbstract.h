#pragma once

#include <JuceHeader.h>
#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"

class M1OrientationHardwareAbstract {
public:
    virtual void setup() = 0;
    virtual void update() = 0;
    virtual void close() = 0;

    virtual M1OrientationTrackingResult getOrientation() = 0;

    virtual void refreshDevices() = 0;
    virtual std::vector<std::string> getDevices() = 0;

    virtual std::string getCurrentDevice() = 0;
    virtual void startTrackingUsingDevice(std::string device) = 0;
};
