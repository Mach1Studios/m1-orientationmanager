//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include "simpleble/SimpleBLE.h"

#define NXTRACKER_SERVICE_UUID ""
#define NXTRACKER_GAP_ADVERTISE_SERVICE_UUID ""
#define NXTRACKER_ORIENTATION_DATA_CHARATERISTIC_UUID ""
#define NXTRACKER_START_WRITE_CHARACTERISTIC_UUID ""
#define NXTRACKER_START_COMMAND ""

class NxTrackerInterface {
public:
    NxTrackerInterface();
    ~NxTrackerInterface();
    
    // ble
    SimpleBLE::Safe::Peripheral* deviceInterface = nullptr;
    SimpleBLE::Safe::Peripheral* get_peripheral_device();
    bool set_peripheral_device(SimpleBLE::Peripheral&);
    
    float* getAngle();
    void recenter();
    int getBatteryLevel();
    int battery_level;

    void disconnect();
};
