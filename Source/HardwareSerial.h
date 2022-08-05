//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"
#include "rs232/rs232.h"

// include device specific
#include "Devices/SupperwareInterface.h"

class HardwareSerial : public HardwareAbstract {
public:
    Orientation orientation;
    std::string currentDevice;
    std::vector<std::string> devices;
    int baudRate = 115200;
    int connectedSerialPortIndex;
    bool isConnected = false;
    juce::StringPairArray portlist;

    void setup() override {
        refreshDevices();
    }

    void update() override {
        if (isConnected){
            char readBuffer[128];
            comRead(connectedSerialPortIndex, readBuffer, 128);
            
            if(strlen(readBuffer) != 0) {
                juce::StringArray receivedSerialData = juce::StringArray::fromTokens(readBuffer, ",", "\"");
                if(receivedSerialData.size() == 4) {
                    M1OrientationQuat newOrientation;
                    newOrientation.w = receivedSerialData[0].getFloatValue();
                    newOrientation.x = receivedSerialData[1].getFloatValue();
                    newOrientation.y = receivedSerialData[2].getFloatValue();
                    newOrientation.z = receivedSerialData[3].getFloatValue();
                    orientation.setQuat(newOrientation);
                } else {
                    M1OrientationYPR newOrientation;
                    newOrientation.yaw = receivedSerialData[0].getFloatValue();
                    newOrientation.pitch = receivedSerialData[1].getFloatValue();
                    newOrientation.roll = receivedSerialData[2].getFloatValue();
                    orientation.setYPR(newOrientation);
                }
            }
        }
    }

    bool connect(int deviceIndex) {
        int port_state = comOpen(deviceIndex, baudRate);
        if (port_state == 1) {
            // Set global ref for device's index (used for disconnect)
            connectedSerialPortIndex = deviceIndex;
            isConnected = true;
            return true;
        } else {
            return false;
        }
    }
    
    void close() override {
        comClose(connectedSerialPortIndex);
        isConnected = false;
    }

    M1OrientationTrackingResult getOrientation() override {
        M1OrientationTrackingResult result;
        result.currentOrientation = orientation;
        result.success = true;
        return result;
    }

    void refreshDevices() override {
        int port_number = comEnumerate();
        for(int port_index=0; port_index < port_number; port_index++) {
            std::cout << "[Serial] Found device: " << comGetPortName(port_index) << std::endl;
            portlist.set(comGetInternalName(port_index),comGetPortName(port_index));
            //TODO: create an easy to maintain string search list instead of listing them all here
            // push back name, path, index, type
        }
    }

    std::vector<std::string> getDevicesNames() override {
        return devices;
    }

    void startTrackingUsingDevice(std::string device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        currentDevice = device;
        statusCallback(true, "ok");
    }

    std::string getCurrentDevice() override {
        return currentDevice;
    }

};
