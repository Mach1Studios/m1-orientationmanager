//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "NxTrackerInterface.h"

//==============================================================================
std::string HighLow2Uuid(const uint64_t high, const uint64_t low){
    uint8_t *u_h = (uint8_t *)&(high);
    uint8_t *u_l = (uint8_t *)&(low);

    char UUID[38];
    std::sprintf(UUID, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            u_h[7], u_h[6], u_h[5], u_h[4], u_h[3], u_h[2], u_h[1], u_h[0],
            u_l[7], u_l[6], u_l[5], u_l[4], u_l[3], u_l[2], u_l[1], u_l[0]
            );
    
    return std::string(UUID);
}

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