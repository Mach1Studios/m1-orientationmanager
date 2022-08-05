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
    std::vector<HardwareDeviceInfo> bleDeviceList;
    std::vector<SimpleBLE::Peripheral> discovered_ble_devices;
    SimpleBLE::Adapter ble;
    std::vector<SimpleBLE::Adapter> ble_list;
    
    void setup() override {
        // Setup callback functions
        refreshDevices();
    }

    void update() override {
        // generate random angles
//        float yaw = 180 * sin(juce::Time::currentTimeMillis() / 100000.0);
//        float pitch = 180 * cos(juce::Time::currentTimeMillis() / 100000.0 - 0.3);
//        float roll = 180 * sin(juce::Time::currentTimeMillis() / 100000.0 + 0.1);
//        orientation.setYPR({ yaw, pitch, roll });
    }

    void close() override {
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
            // TODO: If we want to list all devices put logic here
            
            // SHOW METAWEAR BLE ONLY
            if (discovered_ble_devices[i].identifier().find("MetaWear") != std::string::npos || discovered_ble_devices[i].identifier().find("Mach1-M") != std::string::npos) {
                HardwareDeviceInfo newDevice;
                newDevice.type = M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE;
                newDevice.name = discovered_ble_devices[i].identifier();
                newDevice.path = discovered_ble_devices[i].address();
                newDevice.rssi = discovered_ble_devices[i].rssi();
                newDevice.state = discovered_ble_devices[i].is_connectable() ? M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable :  M1OrientationStatusType::M1OrientationManagerStatusTypeNotConnectable;
                newDevice.index = (int)bleDeviceList.size();
                
                M1OrientationDevice uiDeviceListing;
                uiDeviceListing.name = discovered_ble_devices[i].identifier();
                uiDeviceListing.type = newDevice.type;
                
                bleDeviceList.push_back(newDevice);
            }
        }
    }

    std::vector<std::string> getDevices() override {
        std::vector<std::string> nameList;
        for (int i = 0; i < bleDeviceList.size(); ++i){
            nameList.push_back(bleDeviceList[i].name);
        }
        return nameList;
    }
    
    void startTrackingUsingDevice(std::string device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        auto matchedBLEDevice = std::find_if(bleDeviceList.begin(), bleDeviceList.end(), find_id(device));
        currentDevice = matchedBLEDevice->name;
        statusCallback(true, "ok");
    }

    bool connect(int deviceIndex) {
//        bleDeviceList[deviceIndex].connect();
        bleDeviceList[deviceIndex].state = M1OrientationStatusType::M1OrientationManagerStatusTypeConnected;
        currentDevice = bleDeviceList[deviceIndex].name;
        return true;
    }
    
    void disconnect(int deviceIndex) {
//        bleDeviceList[deviceIndex].disconnect();
        bleDeviceList[deviceIndex].state = M1OrientationStatusType::M1OrientationManagerStatusTypeConnectable;
        currentDevice = "";
    }
    
    std::string getCurrentDevice() override {
        return currentDevice;
    }
};
  
