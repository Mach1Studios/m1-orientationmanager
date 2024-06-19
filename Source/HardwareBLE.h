//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"
#include "m1_mathematics/Orientation.h"

// include device specific
#include "Devices/NxTrackerInterface.h"
#include "Devices/MetaWearInterface.h"
#include "BLEDeviceMap.h"

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

#define NORDIC_UART_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_UART_CHAR_RX      "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_UART_CHAR_TX      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

class HardwareBLE : public HardwareAbstract, Mach1::BLEDeviceMap::Listener {
public:

    Mach1::BLEDeviceMap m_device_map;
    Mach1::Orientation m_orientation;

    // Device Interfaces
    MetaWearInterface meta_wear_iface;
    NxTrackerInterface nx_tracker_iface;

    std::chrono::steady_clock::time_point m_last_battery_update_time;

    int setup() override {
        // Setup callback functions
        m_device_map.RegisterListener(this);
        refreshDevices();
        return 1;
    }

    int update() override {
        if (!m_device_map.IsConnected()) {
            return -1; // return for error handling
        }

        m_device_map.UpdateConnectedDevice();
        return 1;
    }

    void calibrateDevice() override {

    }

    int close() override {
        m_device_map.Disconnect();
        return 1;
    }

    M1OrientationTrackingResult getOrientation() override {
        M1OrientationTrackingResult result;
        result.currentOrientation = m_orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
        m_device_map.Refresh();
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return m_device_map.GetCurrentDevices();
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, TrackingCallback statusCallback) override {
        // TODO: The method signature is misleading. The implication is that tracking is to be performed
        //  with the given M1OrientationDeviceInfo, when in actuality it's an M1OrientationDeviceInfo
        //  that is on record, with the same name. Refactor this method to simply take in a name and address.

        m_device_map.ConnectDevice(device.getDeviceName(), device.getDeviceType(), device.getDeviceAddress(), statusCallback);
    }
     
    M1OrientationDeviceInfo getConnectedDevice() override {
        return m_device_map.GetConnectedDeviceInfo();
    }

    void recenter() override {
        m_orientation.Recenter();
    }

    // =================================================================================================================
    // ========================================= NX TRACKER-SPECIFIC CALLBACKS =========================================
    // =================================================================================================================

    void OnNxTrackerDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) override {

    }

    void OnNxTrackerConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral, TrackingCallback trackingCallback) override {
        nx_tracker_iface.set_peripheral_device(peripheral);
        nx_tracker_iface.sendStartCommand();

        // Report to the manager that it's connected
        trackingCallback(true, "BLE: Nx Tracker Device Connected", info.getDeviceName(), (int)info.getDeviceType(), info.getDeviceAddress());
    }

    void OnNxTrackerUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) override {
        m_orientation.SetRotation(nx_tracker_iface.getRotationQuat());

        // The procedure for getting the battery level is pretty expensive, so do it only once in a while for NX.
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        auto d = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - m_last_battery_update_time).count();
        if (d > 60000) {
            info.batteryPercentage = nx_tracker_iface.getBatteryLevel();
            m_last_battery_update_time = current_time;
        }
    }

    void OnNxTrackerDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral &peripheral) override {

    }

    // =================================================================================================================
    // ========================================== METAWEAR-SPECIFIC CALLBACKS ==========================================
    // =================================================================================================================

    void OnMetaWearDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) override {

    }

    void OnMetaWearConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral, TrackingCallback trackingCallback) override {
        meta_wear_iface.set_peripheral_device(peripheral);

        // setup meta motion
        MblMwBtleConnection btleConnection;
        btleConnection.context = &meta_wear_iface;
        btleConnection.write_gatt_char = meta_wear_iface.write_gatt_char;
        btleConnection.read_gatt_char = meta_wear_iface.read_gatt_char;
        btleConnection.enable_notifications = meta_wear_iface.enable_char_notify;
        btleConnection.on_disconnect = meta_wear_iface.on_disconnect;
        meta_wear_iface.board = mbl_mw_metawearboard_create(&btleConnection);

        auto dev_info = mbl_mw_metawearboard_get_device_information(meta_wear_iface.board);
        std::cout << "firmware revision number = " << dev_info->firmware_revision << std::endl;
        std::cout << "model = " << mbl_mw_metawearboard_get_model(meta_wear_iface.board) << std::endl;

        // context?
        mbl_mw_metawearboard_initialize(meta_wear_iface.board, &meta_wear_iface, [](void* context, MblMwMetaWearBoard* board, int32_t status) -> void {
            if (status != 0) {
                printf("Error initializing board: %d\n", status);
            } else {
                printf("Board initialized\n");
            }
            auto dev_info = mbl_mw_metawearboard_get_device_information(board);
            auto *wrapper = static_cast<MetaWearInterface *>(context);

            while (!mbl_mw_metawearboard_is_initialized(board)){
                // Wait for async initialization finishes
            }
            if (mbl_mw_metawearboard_is_initialized(board) == 1) {
                std::cout << "firmware revision number = " << dev_info->firmware_revision << std::endl;
                std::cout << "model = " << dev_info->model_number << std::endl;
                std::cout << "model = " << mbl_mw_metawearboard_get_model(board) << std::endl;
                std::cout << "model = " << mbl_mw_metawearboard_get_model_name(board) << std::endl;
                wrapper->enable_fusion_sampling(board);
                wrapper->get_current_power_status(board);
                wrapper->get_battery_percentage(board);
                wrapper->get_ad_name(board);
            }
        });

        // Report to the manager that it's connected
        trackingCallback(true, "BLE: MetaMotion Device Connected", info.getDeviceName(), (int)info.getDeviceType(), info.getDeviceAddress());
    }

    void OnMetaWearUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) override {
        float *a = meta_wear_iface.getAngle(); // MMC = Y=0, P=2, R=1; values are in degrees
        float yaw = -a[2];
        float pitch = a[0];
        float roll = a[1];

        m_orientation.SetRotation(Mach1::Float3{yaw, pitch, roll}.EulerRadians());
        info.batteryPercentage = meta_wear_iface.getBatteryLevel();
    }

    void OnMetaWearDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral &peripheral) override {

    }

    // =================================================================================================================
    // =========================================== MACH1-M-SPECIFIC CALLBACKS ==========================================
    // =================================================================================================================

    void OnMach1MDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) override {

    }

    void OnMach1MConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral, TrackingCallback trackingCallback) override {
        OnMetaWearConnected(info, peripheral, trackingCallback);
    }

    void OnMach1MUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) override {
        OnMetaWearUpdated(info, peripheral);
    }

    void OnMach1MDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral &peripheral) override {
        OnMetaWearDisconnected(info, peripheral);
    }

    // =================================================================================================================
    // ============================================ UNKNOWN DEVICE CALLBACKS ===========================================
    // =================================================================================================================

    void OnUnknownDeviceDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral &peripheral) override {

    }

    void OnUnknownDeviceConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral &peripheral,
                                  TrackingCallback trackingCallback) override {

    }

    void OnUnknownDeviceUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral &peripheral) override {

    }

    void OnUnknownDeviceDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral &peripheral) override {

    }
    
    void setAdditionalDeviceSettings(std::string settingsChange) override {
        // Fill device specific instructions here...
    }
};
