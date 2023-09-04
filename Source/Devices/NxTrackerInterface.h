//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include "m1_orientation_client/M1OrientationTypes.h"
#include "simpleble/SimpleBLE.h"

#define NXTRACKER_GAP_ADVERTISE_SERVICE_UUID "A010"
#define NXTRACKER_ORIENTATION_DATA_GATT_SERVICE_UUID "0000a010-5761-7665-7341-7564696f4c74"
#define NXTRACKER_ORIENTATION_DATA_GATT_CHARATERISTIC_UUID "0000a015-5761-7665-7341-7564696f4c74"
#define NXTRACKER_START_WRITE_GATT_CHARACTERISTIC_UUID "0000a011-5761-7665-7341-7564696f4c74"
#define NXTRACKER_START_COMMAND "0x32 0x00 0x00 0x00 0x01"

class NxTrackerInterface {
public:
    NxTrackerInterface();
    ~NxTrackerInterface();
    
    // ble
    SimpleBLE::Safe::Peripheral* deviceInterface = nullptr;
    bool set_peripheral_device(SimpleBLE::Peripheral&);
    
    void sendStartCommand();
    M1OrientationQuat getRotationQuat();
    void recenter();
    int getBatteryLevel();
    int battery_level;

    void disconnect();
    
private:
    
    std::vector<float> parseQuatData(std::vector<uint8_t> data);
};
