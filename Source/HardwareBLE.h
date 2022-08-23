//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"
#include "simpleble/SimpleBLE.h"

// include device specific
#include "Devices/MetaWear.h"

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

class HardwareBLE : public HardwareAbstract {
public:
    Orientation orientation;
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    std::vector<SimpleBLE::Peripheral> discovered_ble_devices;
    SimpleBLE::Adapter ble;
    std::vector<SimpleBLE::Adapter> ble_list;
    bool isConnected = false;
    bool displayOnlyKnownIMUs = true;
    
    void setup() override {
        // Setup callback functions
        refreshDevices();
    }

    void update() override {
        if (isConnected){
            //
        }
    }

    void close() override {
        for (int i = 0; i < discovered_ble_devices.size(); ++i) {
            if (discovered_ble_devices[i].address() == connectedDevice.getDeviceAddress()) {
                discovered_ble_devices[i].disconnect();
                
                auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(connectedDevice.getDeviceName()));
                if (matchedDevice != devices.end()) {
                    //TODO: should we trigger a new search to update the list again?
//                    matchedDevice->state = M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable;
                    connectedDevice = *matchedDevice;
                }

                isConnected = false;
            }
        }

        //TODO: improve this, i dont think this works
        ble.scan_stop();
    }

    M1OrientationTrackingResult getOrientation() override {
        M1OrientationTrackingResult result;
        result.currentOrientation = orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
        try {
            ble_list = SimpleBLE::Adapter::get_adapters();
        } catch (...) {
            throw;
        }
        
        if (ble_list.size() == 0) {
            std::cout << "[BLE] No adapter was found." << std::endl;
            return;
        }
        
        try {
            SimpleBLE::Adapter adapter = ble_list[0];

            adapter.set_callback_on_scan_start([]() { std::cout << "[BLE] Scan started." << std::endl; });
            adapter.set_callback_on_scan_stop([]() { std::cout << "[BLE] Scan stopped." << std::endl; });
            adapter.set_callback_on_scan_found([this](SimpleBLE::Peripheral peripheral) {
                std::cout << "[BLE] Found device: " << peripheral.identifier() << " [" << peripheral.address() << "] " << peripheral.rssi() << " dBm" << std::endl;
                discovered_ble_devices.push_back(peripheral);
            });
            adapter.scan_for(SCAN_TIMEOUT_MS);
        } catch (...) {
            throw;
        }
        
        devices.clear();

        // TODO: create a switch UI for filtering only known IMU devices vs showing all BLE
        for (int i = 0; i < discovered_ble_devices.size(); ++i){
            if (!displayOnlyKnownIMUs){
                // SHOW ALL CONNECTABLE BLE
                if (discovered_ble_devices[i].is_connectable()) {
                    devices.push_back({ discovered_ble_devices[i].identifier(), M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE, discovered_ble_devices[i].address(), (int)discovered_ble_devices[i].rssi()});
                }
            } else {
                if (discovered_ble_devices[i].identifier().find("MetaWear") != std::string::npos || discovered_ble_devices[i].identifier().find("IMU") != std::string::npos || discovered_ble_devices[i].identifier().find("Mach1-M") != std::string::npos) {
                    // SHOW METAWEAR/IMU/MACH1-M BLE ONLY
                    devices.push_back({ discovered_ble_devices[i].identifier(), M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE, discovered_ble_devices[i].address(), (int)discovered_ble_devices[i].rssi()});
                }
            }
        }
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }
    
    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            for (int i = 0; i < discovered_ble_devices.size(); ++i) {
                if (discovered_ble_devices[i].address() == matchedDevice->getDeviceAddress()) {
                    discovered_ble_devices[i].connect();
                    // TODO: return errorMessage if any error
                    
                    connectedDevice = *matchedDevice;
                    isConnected = true;
                    
                    statusCallback(true, "ok");
                    return;
                }
            }
        }

        statusCallback(false , "not connected");
    }
     
    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }
};
