//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"
#include "simpleble/SimpleBLE.h"

// include device specific
#include "Devices/NxTrackerInterface.h"
#include "Devices/MetaWearInterface.h"

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

#define SCAN_TIMEOUT_MS 2000

#define NORDIC_UART_SERVICE_UUID "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_UART_CHAR_RX      "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NORDIC_UART_CHAR_TX      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

class HardwareBLE : public HardwareAbstract {
public:
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    std::vector<SimpleBLE::Safe::Peripheral> discovered_ble_devices;

    Orientation orientation;

    // Device Interfaces
    MetaWearInterface metawearInterface;
    NxTrackerInterface nxtrackerInterface;
    bool isConnected = false;
    bool displayOnlyKnownIMUs = true;
    
    int setup() override {
        // Setup callback functions
        refreshDevices();
        return 1;
    }

    int update() override {
        if (isConnected){
            // Update RSSI value
            for (int i = 0; i < discovered_ble_devices.size(); ++i) {
                if (discovered_ble_devices[i].address() == connectedDevice.getDeviceAddress()) {
                    getConnectedDevice().signalStrength = discovered_ble_devices[i].rssi().value_or(0);
                }
            }
            
            if (getConnectedDevice().getDeviceName().find("Nx Tracker") != std::string::npos) {
                //std::cout << "[BLE] Nx Tracker device: " << getConnectedDevice().getDeviceName() << ", " << getConnectedDevice().getDeviceAddress() << ", " << std::endl;
                
                M1OrientationQuat newOrientation = nxtrackerInterface.getRotationQuat();
                orientation.setQuat(newOrientation);
                
                int b = nxtrackerInterface.getBatteryLevel();
                getConnectedDevice().batteryPercentage = b;
            }
            
            // TODO: Add magnometer activate function `bUseMagnoHeading`
            if (getConnectedDevice().getDeviceName().find("MetaWear") != std::string::npos || getConnectedDevice().getDeviceName().find("Mach1-M") != std::string::npos) {
                float* a = metawearInterface.getAngle(); // MMC = Y=0, P=2, R=1
                //std::cout << "[BLE] MetaWear device: " << a[0] << ", " << a[2] << ", " << a[1] << std::endl;
                M1OrientationYPR newOrientation;
                newOrientation.yaw = a[0];
                newOrientation.pitch = -a[2];
                newOrientation.roll = -a[1];
                newOrientation.angleType = M1OrientationYPR::DEGREES; // TODO: check this!
                orientation.setYPR(newOrientation);

                // Update battery percentage
                int b = metawearInterface.getBatteryLevel();
                getConnectedDevice().batteryPercentage = b;
            }
            return 1;
        } else {
            // return for error handling
            return -1;
        }
    }
    
    void calibrateDevice() override {
        
    }

    int close() override {
        for (int i = 0; i < discovered_ble_devices.size(); ++i) {
            if (discovered_ble_devices[i].address() == connectedDevice.getDeviceAddress()) {
                discovered_ble_devices[i].disconnect();
                
                auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(connectedDevice.getDeviceName()));
                if (matchedDevice != devices.end()) {
                    //TODO: should we trigger a new search to update the list again?
                    //matchedDevice->state = M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable;
                    connectedDevice = *matchedDevice;
                }
                isConnected = false;
            }
        }
        //TODO: improve this, i dont think this works
        //ble.scan_stop();
        return 1;
    }

    M1OrientationTrackingResult getOrientation() override {
        M1OrientationTrackingResult result;
        result.currentOrientation = orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
		std::vector<M1OrientationDeviceInfo> devices;
		std::vector<SimpleBLE::Safe::Peripheral> discovered_ble_devices;

        auto ble_list = SimpleBLE::Safe::Adapter::get_adapters();
        
        if (!ble_list.has_value() || ble_list->empty()) {
            std::cout << "[BLE] No adapter was found." << std::endl;
            return;
        }

        // grab the first adapter
        // TODO: add error handling?
        SimpleBLE::Safe::Adapter& adapter = ble_list->at(0);

        adapter.set_callback_on_scan_start([]() { std::cout << "[BLE] Scan started." << std::endl; });
        adapter.set_callback_on_scan_stop([]() { std::cout << "[BLE] Scan stopped." << std::endl; });
        adapter.set_callback_on_scan_found([&](SimpleBLE::Safe::Peripheral peripheral) {
            std::cout << "[BLE] Found device: " << peripheral.identifier().value_or("UNKNOWN") << " [" << peripheral.address().value_or("UNKNOWN") << "] " << peripheral.rssi().value_or(0) << " dBm" << std::endl;
            discovered_ble_devices.push_back(peripheral);
        });
        adapter.scan_for(SCAN_TIMEOUT_MS);
        
        // TODO: create a switch UI for filtering only known IMU devices vs showing all BLE
        for (int i = 0; i < discovered_ble_devices.size(); ++i){
            if (!displayOnlyKnownIMUs){
                /// SHOW ALL CONNECTABLE BLE DEVICES
                if (discovered_ble_devices[i].is_connectable()) {
                    devices.push_back({ discovered_ble_devices[i].identifier().value_or("UNKNOWN"), M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE, discovered_ble_devices[i].address().value_or("UNKNOWN"), discovered_ble_devices[i].rssi().value_or(0) });
                }
            } else {
                /// SHOW KNOWN BLE DEVICES USING DEVICE NAME FILTERS

                // SHOW NX TRACKER BLE
                if (discovered_ble_devices[i].identifier()->find("Nx Tracker") != std::string::npos) {
                    devices.push_back({ discovered_ble_devices[i].identifier().value_or("UNKNOWN"), M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE, discovered_ble_devices[i].address().value_or("UNKNOWN"), discovered_ble_devices[i].rssi().value_or(0) });
                    nxtrackerInterface.set_peripheral_device(discovered_ble_devices[i]);
                }
                
                // SHOW METAWEAR/METAMOTION/MACH1-M BLE
                if (discovered_ble_devices[i].identifier()->find("MetaWear") != std::string::npos ||  discovered_ble_devices[i].identifier()->find("Mach1-M") != std::string::npos) {
                    devices.push_back({ discovered_ble_devices[i].identifier().value_or("UNKNOWN"), M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE, discovered_ble_devices[i].address().value_or("UNKNOWN"), discovered_ble_devices[i].rssi().value_or(0) });
                    // Setup and construct MetaWearInterface device with pointer to peripheral
                    metawearInterface.set_peripheral_device(discovered_ble_devices[i]);
                }
            }
        }

		lock();
		this->devices = devices;
		this->discovered_ble_devices = discovered_ble_devices;
		unlock();
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }
    
    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            for (int i = 0; i < discovered_ble_devices.size(); ++i) {
                if (discovered_ble_devices[i].address() == matchedDevice->getDeviceAddress()) {
                    
                    discovered_ble_devices[i].connect();
                    
                    // IF NX TRACKER
                    if (matchedDevice->getDeviceName().find("Nx Tracker") != std::string::npos) {
                        nxtrackerInterface.sendStartCommand();
                        // Report to the manager that it's connected
                        statusCallback(true, "BLE: Nx Tracker Device Connected", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());

                    // IF METAMOTION
                    } else if (matchedDevice->getDeviceName().find("MetaWear") != std::string::npos || matchedDevice->getDeviceName().find("Mach1-M") != std::string::npos) {
                        // setup meta motion
                        MblMwBtleConnection btleConnection;
                        btleConnection.context = &metawearInterface;
                        btleConnection.write_gatt_char = metawearInterface.write_gatt_char;
                        btleConnection.read_gatt_char = metawearInterface.read_gatt_char;
                        btleConnection.enable_notifications = metawearInterface.enable_char_notify;
                        btleConnection.on_disconnect = metawearInterface.on_disconnect;
                        metawearInterface.board = mbl_mw_metawearboard_create(&btleConnection);

                        auto dev_info = mbl_mw_metawearboard_get_device_information(metawearInterface.board);
                        std::cout << "firmware revision number = " << dev_info->firmware_revision << std::endl;
                        std::cout << "model = " << mbl_mw_metawearboard_get_model(metawearInterface.board) << std::endl;
                        
                        // context?
                        mbl_mw_metawearboard_initialize(metawearInterface.board, &metawearInterface, [](void* context, MblMwMetaWearBoard* board, int32_t status) -> void {
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
                        statusCallback(true, "BLE: MetaMotion Device Connected", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
                        
                    } else { // NOT ANY FILTERED/KNOWN IMU DEVICES
                        // TODO: return message if any error
                        statusCallback(false, "BLE: Device "+matchedDevice->getDeviceName()+" is not yet supported", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
                    }
                    connectedDevice = *matchedDevice;
                    isConnected = true;
                    return;
                }
            }
        }
        statusCallback(false , "BLE: Not connected", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
    }
     
    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }
};
