//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "NxTrackerInterface.h"

//==============================================================================

NxTrackerInterface::NxTrackerInterface()
{
}

NxTrackerInterface::~NxTrackerInterface()
{
}

bool NxTrackerInterface::set_peripheral_device(SimpleBLE::Peripheral& peripheral) {
    if (peripheral.address() == "UNKNOWN"){
        return false;
    } else {
        this->deviceInterface = new SimpleBLE::Safe::Peripheral(peripheral);
        return true;
    }
}

float* NxTrackerInterface::getAngle() {
    
}

void NxTrackerInterface::recenter() {

}

int NxTrackerInterface::getBatteryLevel() {
    return battery_level;
}
