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

#include "simpleble/SimpleBLE.h"

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

class MetaWearInterface {
public:
    MetaWearInterface();
    ~MetaWearInterface();

    bool bUseMagnoHeading = true;
    float* getAngle();
    float outputEuler[4];
    float angle_shift[3];
    int getBatteryLevel();
    int battery_level;
    const char* module_name;
    
    // ble
    SimpleBLE::Safe::Peripheral* deviceInterface = nullptr;
    SimpleBLE::Safe::Peripheral* get_peripheral_device();
    bool set_peripheral_device(SimpleBLE::Peripheral&);
    
    MblMwMetaWearBoard * board;
    void data_printer(void* context, const MblMwData* data);
    void get_current_power_status(MblMwMetaWearBoard* board);
    void get_battery_percentage(MblMwMetaWearBoard* board);
    void configure_sensor_fusion(MblMwMetaWearBoard* board);
    void enable_fusion_sampling(MblMwMetaWearBoard* board);
    void disable_fusion_sampling(MblMwMetaWearBoard* board);
    void calibration_mode(MblMwMetaWearBoard* board);
    void enable_led(MblMwMetaWearBoard* board);
    void disable_led(MblMwMetaWearBoard* board);
    void set_ad_name(MblMwMetaWearBoard* board);
    void get_ad_name(MblMwMetaWearBoard* board);

    void resetOrientation();
    void recenter();
    
    static void read_gatt_char(void *context, const void *caller, const MblMwGattChar *characteristic, MblMwFnIntVoidPtrArray handler);
    static void write_gatt_char(void *context, const void *caller, MblMwGattCharWriteType writeType, const MblMwGattChar *characteristic, const uint8_t *value, uint8_t length);
    static void enable_char_notify(void *context, const void *caller, const MblMwGattChar *characteristic, MblMwFnIntVoidPtrArray handler, MblMwFnVoidVoidPtrInt ready);
    static void on_disconnect(void *context, const void *caller, MblMwFnVoidVoidPtrInt handler);
};
