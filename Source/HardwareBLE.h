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
    std::string currentDevice;
    std::vector<M1OrientationDevice> bleDeviceList;
    std::vector<SimpleBLE::Peripheral> discovered_ble_devices;
    SimpleBLE::Adapter ble;
    std::vector<SimpleBLE::Adapter> ble_list;
    bool isConnected = false;
    
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
            std::cout << "No adapter was found." << std::endl;
            return;
        }
        
        try {
            SimpleBLE::Adapter adapter = ble_list[0];

            adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });
            adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });
            adapter.set_callback_on_scan_found([this](SimpleBLE::Peripheral peripheral) {
                std::cout << "Found device: " << peripheral.identifier() << " [" << peripheral.address() << "] " << peripheral.rssi() << " dBm" << std::endl;
                discovered_ble_devices.push_back(peripheral);
            });
            adapter.scan_for(SCAN_TIMEOUT_MS);
        } catch (...) {
            throw;
        }
        
        // TODO: create a switch UI for filtering only known IMU devices vs showing all BLE
        for (int i = 0; i < discovered_ble_devices.size(); ++i){
            // SHOW ALL BLE
            if (discovered_ble_devices[i].is_connectable()) {
                M1OrientationDevice newDevice;
                newDevice.type = M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE;
                newDevice.name = discovered_ble_devices[i].identifier();
                newDevice.path = discovered_ble_devices[i].address();
                newDevice.rssi = discovered_ble_devices[i].rssi();
                newDevice.state = discovered_ble_devices[i].is_connectable() ? M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable :  M1OrientationStatusType::M1OrientationManagerStatusTypeNotConnectable;
                bleDeviceList.push_back(newDevice);
            }
            // SHOW BLE WITH "IMU" IN NAME
            else if (discovered_ble_devices[i].identifier().find("IMU") != std::string::npos) {
                M1OrientationDevice newDevice;
                newDevice.type = M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE;
                newDevice.name = discovered_ble_devices[i].identifier();
                newDevice.path = discovered_ble_devices[i].address();
                newDevice.rssi = discovered_ble_devices[i].rssi();
                newDevice.state = discovered_ble_devices[i].is_connectable() ? M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable :  M1OrientationStatusType::M1OrientationManagerStatusTypeNotConnectable;
                bleDeviceList.push_back(newDevice);
            }
                
            // SHOW METAWEAR BLE ONLY
             else if (discovered_ble_devices[i].identifier().find("MetaWear") != std::string::npos || discovered_ble_devices[i].identifier().find("Mach1-M") != std::string::npos) {
                M1OrientationDevice newDevice;
                newDevice.type = M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE;
                newDevice.name = discovered_ble_devices[i].identifier();
                newDevice.path = discovered_ble_devices[i].address();
                newDevice.rssi = discovered_ble_devices[i].rssi();
                newDevice.state = discovered_ble_devices[i].is_connectable() ? M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable :  M1OrientationStatusType::M1OrientationManagerStatusTypeNotConnectable;
                bleDeviceList.push_back(newDevice);
            }
        }
    }

    std::vector<std::string> getDevicesNames() override {
        std::vector<std::string> nameList;
        for (int i = 0; i < bleDeviceList.size(); ++i){
            nameList.push_back(bleDeviceList[i].name);
        }
        return nameList;
    }
    
    void startTrackingUsingDevice(std::string device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        auto matchedBLEDevice = std::find_if(bleDeviceList.begin(), bleDeviceList.end(), M1OrientationDevice::find_id(device));
        currentDevice = matchedBLEDevice->name;
        statusCallback(true, "ok");
    }

    bool connect(int deviceIndex) {
        for (int i = 0; i < discovered_ble_devices.size(); ++i) {
            if (discovered_ble_devices[i].address() == bleDeviceList[deviceIndex].path){
                discovered_ble_devices[i].connect();
                bleDeviceList[deviceIndex].state = M1OrientationStatusType::M1OrientationManagerStatusTypeConnected;
                currentDevice = bleDeviceList[deviceIndex].name;
                isConnected = true;
                return true;
            }
        }
        isConnected = false;
        return false;
    }
    
    void disconnect(int deviceIndex) {
        for (int i = 0; i < discovered_ble_devices.size(); ++i) {
            if (discovered_ble_devices[i].address() == bleDeviceList[deviceIndex].path){
                discovered_ble_devices[i].disconnect();
                bleDeviceList[deviceIndex].state = M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable;
                currentDevice = "";
                isConnected = false;
            }
        }
    }
    
    std::string getCurrentDevice() override {
        return currentDevice;
    }
};