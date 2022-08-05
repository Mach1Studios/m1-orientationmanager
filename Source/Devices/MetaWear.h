//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

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

//MblMwMetaWearBoard * board;
//void data_printer(void* context, const MblMwData* data);
//void get_current_power_status(MblMwMetaWearBoard* board);
//void get_battery_percentage(MblMwMetaWearBoard* board);
//void configure_sensor_fusion(MblMwMetaWearBoard* board);
//void enable_fusion_sampling(MblMwMetaWearBoard* board);
//void disable_fusion_sampling(MblMwMetaWearBoard* board);
//void calibration_mode(MblMwMetaWearBoard* board);
//void enable_led(MblMwMetaWearBoard* board);
//void disable_led(MblMwMetaWearBoard* board);
//void set_ad_name(MblMwMetaWearBoard* board);
//void get_ad_name(MblMwMetaWearBoard* board);
//
//static void read_gatt_char(void *context, const void *caller, const MblMwGattChar *characteristic,
//                              MblMwFnIntVoidPtrArray handler);
//
//static void write_gatt_char(void *context, const void *caller, MblMwGattCharWriteType writeType,
//                               const MblMwGattChar *characteristic, const uint8_t *value, uint8_t length);
//
//static void enable_char_notify(void *context, const void *caller, const MblMwGattChar *characteristic,
//                                  MblMwFnIntVoidPtrArray handler, MblMwFnVoidVoidPtrInt ready);
//
//static void on_disconnect(void *context, const void *caller, MblMwFnVoidVoidPtrInt handler);
//
