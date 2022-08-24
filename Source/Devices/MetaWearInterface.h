//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <csignal>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

#ifndef WIN32
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#endif

#include "metawear/core/metawearboard.h"
#include "metawear/core/module.h"
#include "metawear/core/settings.h"

#include "metawear/core/status.h"
#include "metawear/core/debug.h"
#include "metawear/core/logging.h"
#include "metawear/core/macro.h"
#include "metawear/peripheral/led.h"

#include "metawear/core/data.h"
#include "metawear/core/datasignal.h"
#include "metawear/core/types.h"
#include "metawear/sensor/sensor_common.h"
#include "metawear/sensor/sensor_fusion.h"

#define SCAN_TIMEOUT_MS 3000

#define NORDIC_UART_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_UART_CHAR_RX      "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_UART_CHAR_TX      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

#define METAMOTION_READ_SERVICE_UUID    "0000180a-0000-1000-8000-00805f9b34fb"
#define METAMOTION_READ_UUID            "00002a26-0000-1000-8000-00805f9b34fb"
#define METAMOTION_NOTIFY_SERVICE_UUID  "326a9000-85cb-9195-d9dd-464cfbbae75a"
#define METAMOTION_NOTIFY_UUID          "326a9006-85cb-9195-d9dd-464cfbbae75a"
#define METAMOTION_WRITE_SERVICE_UUID   "326a9000-85cb-9195-d9dd-464cfbbae75a"
#define METAMOTION_WRITE_UUID           "326a9001-85cb-9195-d9dd-464cfbbae75a"

bool bUseMagnoHeading = true;
float outputEuler[4];
float* getAngle();
float angle[3];
float angle_shift[3];
int battery_level;
const char* module_name;

MblMwMetaWearBoard * board;

void resetOrientation();
void recenter();

void data_printer(void* context, const MblMwData* data) {
    // Print data as 2 digit hex values
    uint8_t* data_bytes = (uint8_t*) data->value;
    std::string bytes_str("[");
    char buffer[5];
    for (uint8_t i = 0; i < data->length; i++) {
        if (i) {
            bytes_str += ", ";
        }
        //sprintf(buffer, "0x%02x", data_bytes[i]);
        bytes_str += buffer;
    }
    bytes_str += "]";

    // Format time as YYYYMMDD-HH:MM:SS.LLL
    std::chrono::time_point<std::chrono::system_clock> then(std::chrono::milliseconds(data->epoch));
    auto time_c = std::chrono::system_clock::to_time_t(then);
    auto rem_ms= data->epoch % 1000;

    std::cout << "{timestamp: " << std::put_time(localtime(&time_c), "%Y%m%d-%T") << "." << rem_ms << ", "
        << "type_id: " << data->type_id << ", "
        << "bytes: " << bytes_str.c_str() << "}"
        << std::endl;
}

void configure_sensor_fusion(MblMwMetaWearBoard* board) {
    // set fusion mode to ndof (n degress of freedom)
    mbl_mw_sensor_fusion_set_mode(board, MBL_MW_SENSOR_FUSION_MODE_NDOF);
    // set acceleration range to +/-16G, note accelerometer is configured here
    mbl_mw_sensor_fusion_set_acc_range(board, MBL_MW_SENSOR_FUSION_ACC_RANGE_16G);
    // set gyro range to 2000 DPS
    mbl_mw_sensor_fusion_set_gyro_range(board, MBL_MW_SENSOR_FUSION_GYRO_RANGE_2000DPS);
    // write changes to the board
    mbl_mw_sensor_fusion_write_config(board);
    
    // set the tx power as high as allowed
    if (mbl_mw_metawearboard_get_model(board) == MBL_MW_MODEL_METAMOTION_S){
        mbl_mw_settings_set_tx_power(board, 8);
    } else {
        mbl_mw_settings_set_tx_power(board, 4);
    }
}

void get_battery_percentage(MblMwMetaWearBoard* board) {
    auto battery_signal = mbl_mw_settings_get_battery_state_data_signal(board);
//    mbl_mw_datasignal_subscribe(battery_signal, this, [](void* context, const MblMwData* data) -> void {
//        auto state = (MblMwBatteryState*) data->value;
//        battery_level = state->charge;
//        //printf("{voltage: %dmV, charge: %d}\n", state->voltage, state->charge);
//    });
    mbl_mw_datasignal_read(battery_signal);
}

void get_ad_name(MblMwMetaWearBoard* board){
    // This function is for calling the name via metamotion
    module_name = mbl_mw_metawearboard_get_model_name(board);
}

void enable_fusion_sampling(MblMwMetaWearBoard* board) {
    // Write the config to the sensor
    configure_sensor_fusion(board);
    
    auto fusion_signal = mbl_mw_sensor_fusion_get_data_signal(board, MBL_MW_SENSOR_FUSION_DATA_EULER_ANGLE);
//    mbl_mw_datasignal_subscribe(fusion_signal, this, [](void* context, const MblMwData* data) -> void {
//
//        auto euler = (MblMwEulerAngles*)data->value;
//        if (bUseMagnoHeading){ // externally set use of magnometer to correct IMU or not
//            outputEuler[0] = euler->heading;
//            outputEuler[3] = euler->yaw;
//        } else {
//            outputEuler[0] = euler->yaw;
//            outputEuler[3] = euler->heading;
//        }
//        outputEuler[1] = euler->pitch;
//        outputEuler[2] = euler->roll;
//        //printf("(%.3f, %.3f, %.3f)\n", euler->yaw, euler->pitch, euler->roll);
//    });
    
    // Start
    mbl_mw_sensor_fusion_enable_data(board, MBL_MW_SENSOR_FUSION_DATA_EULER_ANGLE);
    mbl_mw_sensor_fusion_start(board);
    //enable_led(board);
}

void enable_led(MblMwMetaWearBoard* board) {
    MblMwLedPattern pattern;
    pattern = { 8, 0, 250, 250, 250, 5000, 0, 0 };
    mbl_mw_led_write_pattern(board, &pattern, MBL_MW_LED_COLOR_RED);
    mbl_mw_led_play(board);
}

void disable_led(MblMwMetaWearBoard* board) {
    // stop the LED pattern from playing
    mbl_mw_led_stop_and_clear(board);
}

void disable_fusion_sampling(MblMwMetaWearBoard* board) {
    auto fusion_signal = mbl_mw_sensor_fusion_get_data_signal(board, MBL_MW_SENSOR_FUSION_DATA_EULER_ANGLE);
    mbl_mw_datasignal_unsubscribe(fusion_signal);
    mbl_mw_sensor_fusion_stop(board);
}

void calibration_mode(MblMwMetaWearBoard* board) {
    
}

void resetOrientation() {
    for(int i=0; i< 3; i++){
        angle_shift[i] = 0;
    }
}

void recenter() {
    float* swpAngle = getAngle();
    angle_shift[0] = angle_shift[0] - swpAngle[0];
    angle_shift[1] = angle_shift[1] - swpAngle[1];
    angle_shift[2] = angle_shift[2] - swpAngle[2];
}

float* getAngle() {
    float* calculated_angle = new float[3];
        
    calculated_angle[0] = angle[0] + angle_shift[0];
    calculated_angle[1] = angle[1] + angle_shift[1];
    calculated_angle[2] = angle[2] + angle_shift[2];
    
    return calculated_angle;
}

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

void read_gatt_char(void *context, const void *caller, const MblMwGattChar *characteristic, MblMwFnIntVoidPtrArray handler) {
    auto *wrapper = static_cast<SimpleBLE::Peripheral *>(context);
    auto readByteArray = wrapper->read(HighLow2Uuid(characteristic->service_uuid_high, characteristic->service_uuid_low), HighLow2Uuid(characteristic->uuid_high, characteristic->uuid_low));
                                                 
    handler(caller, (uint8_t*)readByteArray.data(), readByteArray.length());
}

void write_gatt_char(void *context, const void *caller, MblMwGattCharWriteType writeType, const MblMwGattChar *characteristic, const uint8_t *value, uint8_t length){
    auto *wrapper = static_cast<SimpleBLE::Peripheral *>(context);
    wrapper->write_command(HighLow2Uuid(characteristic->service_uuid_high, characteristic->service_uuid_low), HighLow2Uuid(characteristic->uuid_high, characteristic->uuid_low), std::string((char*)value, int(length)));
}

void enable_char_notify(void *context, const void *caller, const MblMwGattChar *characteristic, MblMwFnIntVoidPtrArray handler, MblMwFnVoidVoidPtrInt ready) {
    auto *wrapper = static_cast<SimpleBLE::Peripheral *>(context);
    wrapper->notify(HighLow2Uuid(characteristic->service_uuid_high, characteristic->service_uuid_low), HighLow2Uuid(characteristic->uuid_high, characteristic->uuid_low), [&,handler,caller](SimpleBLE::ByteArray payload) {
        handler(caller,(uint8_t*)payload.data(),payload.length());
    });
    ready(caller, MBL_MW_STATUS_OK);
}

void on_disconnect(void *context, const void *caller, MblMwFnVoidVoidPtrInt handler) {
    // call this handler everytime connection is lost, use 0 for 'value' parameter
    handler(caller, 0);
}
