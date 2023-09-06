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

void NxTrackerInterface::sendStartCommand() {
    if (deviceInterface != nullptr) {
        std::string start_command = {0x32, 0x00, 0x00, 0x00, 0x01};
        deviceInterface->write_request(NXTRACKER_ORIENTATION_DATA_GATT_SERVICE_UUID, NXTRACKER_START_WRITE_GATT_CHARACTERISTIC_UUID, start_command);
    }
}

std::vector<float> NxTrackerInterface::parseQuatData(std::vector<uint8_t> data) {
    const float halfOfSignedShort = 16384.0;

    signed short val0 = (signed short)((data[1] << 8) | data[0]); //Q0
    signed short val2 = (signed short)((data[3] << 8) | data[2]); //Q2
    signed short val1 = (signed short)((data[5] << 8) | data[4]); //Q1
    signed short val3 = (signed short)((data[7] << 8) | data[6]); //Q3

    float n_q0 = static_cast<float>(val0) / halfOfSignedShort;
    float n_q1 = static_cast<float>(val1) / halfOfSignedShort;
    float n_q2 = static_cast<float>(val2) / halfOfSignedShort;
    float n_q3 = static_cast<float>(val3) / halfOfSignedShort;

    return {n_q0, n_q1, n_q2, n_q3};
}

M1OrientationQuat NxTrackerInterface::getRotationQuat() {
    if (deviceInterface->is_connected()) {
        // TODO: switch to a notify command to get data as soon as available by the rate dictated by device
        std::optional<SimpleBLE::ByteArray> rx_data = deviceInterface->read(NXTRACKER_ORIENTATION_DATA_GATT_SERVICE_UUID, NXTRACKER_ORIENTATION_DATA_GATT_CHARATERISTIC_UUID);

        if (rx_data.has_value()) {
            std::string str = rx_data.value();
            std::vector<uint8_t> data(str.begin(), str.end()); // convert incoming data string to bytes
            std::vector<float> read_quat = parseQuatData(data);
            
            M1OrientationQuat newQuat;
            newQuat.w = read_quat[0];
            newQuat.x = read_quat[1];
            newQuat.y = read_quat[2];
            newQuat.z = read_quat[3];
            return newQuat;
        }
    } else {
        // return error?
    }
}

void NxTrackerInterface::recenter() {
    // TODO: implement this
}

int NxTrackerInterface::getBatteryLevel() {
    std::optional<SimpleBLE::ByteArray> rx_data = deviceInterface->read("180f", "2a19"); // battery UUID
    if (rx_data.has_value()) {
        std::string batt_value_ascii = rx_data.value();
        const int length = batt_value_ascii.length();
        if (length > 0) {
            
            //battery_level = (int)batt_value_ascii; // cast to int to convert from ascii -> decimal
            //return battery_level;
        }
    }
}

void NxTrackerInterface::disconnect() {
    if (deviceInterface->is_connected()) {
        deviceInterface->disconnect();
    }
}
