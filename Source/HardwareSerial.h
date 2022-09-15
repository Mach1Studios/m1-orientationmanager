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
#include "Devices/M1Interface.h"

class HardwareSerial : public HardwareAbstract/*, SupperwareInterface::Listener*/ {
public:
    Orientation orientation;
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    int baudRate = 115200;
    int connectedSerialPortIndex;
    bool isConnected = false;
    juce::StringPairArray portlist;
    bool displayOnlyKnownIMUs = true;

    // Device Interfaces
    M1Interface m1interface;
    SupperwareInterface supperwareInterface;

    void setup() override {
        // TODO: move the orientation update into this listener callback
        //supperwareInterface.setListener(this);
        refreshDevices();
    }

    void update() override {
        if (isConnected){
            char readBuffer[128];
            comRead(connectedSerialPortIndex, readBuffer, 128);
            
            std::vector<unsigned char> queueBuffer;
            std::string queueString = "";

            for (int i = 0; i < sizeof(readBuffer)/sizeof(int); i++) {
                queueBuffer.insert(queueBuffer.begin() + i, (int)readBuffer[i]);
                queueString.push_back(readBuffer[i]);
            }
            
            /// ORIENTATION UPDATE: SUPPERWARE
            if (getConnectedDevice().getDeviceName().find("Supperware HT IMU") != std::string::npos) {
                if (supperwareInterface.getTrackerDriver().isConnected()) {
                    if (supperwareInterface.currentOrientation.size() == 3) {
                        M1OrientationYPR newOrientation;
                        newOrientation.yaw = supperwareInterface.currentOrientation[0];
                        newOrientation.pitch = supperwareInterface.currentOrientation[1];
                        newOrientation.roll = supperwareInterface.currentOrientation[2];
                        newOrientation.angleType = M1OrientationYPR::AngleType::DEGREES;
                        orientation.setYPR(newOrientation);
                        return;
                    } else if (supperwareInterface.currentOrientation.size() == 4) {
                        M1OrientationQuat newOrientation;
                        newOrientation.wIn = supperwareInterface.currentOrientation[0];
                        newOrientation.xIn = supperwareInterface.currentOrientation[1];
                        newOrientation.yIn = supperwareInterface.currentOrientation[2];
                        newOrientation.zIn = supperwareInterface.currentOrientation[3];
                        orientation.setQuat(newOrientation);
                        return;
                    } else {
                        // error or do nothing
                    }
                } else {
                    // TODO: error for being abstractly connected by not literally
                }
                
            } else {
                /// ORIENTATION UPDATE: SERIAL
                while ((queueString.length() > 0) || (queueBuffer.size() > 0)) {
                    /// UPDATES FOR KNOWN MACH1 IMUs
                    if (getConnectedDevice().getDeviceName().find("Mach1-") != std::string::npos || getConnectedDevice().getDeviceName().find("HC-06-DevB") != std::string::npos || getConnectedDevice().getDeviceName().find("witDevice") != std::string::npos || getConnectedDevice().getDeviceName().find("m1YostDevice") != std::string::npos || getConnectedDevice().getDeviceName().find("usbmodem") != std::string::npos ||
                        getConnectedDevice().getDeviceName().find("usbmodem1434302") != std::string::npos || getConnectedDevice().getDeviceName().find("m1Device") != std::string::npos) {

                        bool anythingNewDetected = false;
                        auto decoded = M1Interface::decode3PieceString(queueString, 'Y', 'P', 'R', 4);
                        
                        if (decoded.gotY || decoded.gotP || decoded.gotR) {
                            anythingNewDetected = true;
                            queueBuffer.clear();
                        }
                        
                        if (anythingNewDetected) {
                            M1OrientationYPR newOrientation;
                            newOrientation.yaw = decoded.y;
                            newOrientation.pitch = decoded.p;
                            newOrientation.roll = decoded.r;
                            orientation.setYPR(newOrientation);
                            // cleanup
                            queueBuffer.clear();
                            queueString.clear();
                            return;
                        }
                    } else {
                        /// UPDATES FOR GENERIC DEVICES
                        juce::StringArray receivedSerialData = juce::StringArray::fromTokens(readBuffer, ",", "\""); // expect delimited characters of ,
                        // TODO: figure out how to ensure the buffer captures a specific number of chars
                        if(receivedSerialData.size() == 4) {
                            M1OrientationQuat newOrientation;
                            newOrientation.wIn = receivedSerialData[0].getFloatValue();
                            newOrientation.xIn = receivedSerialData[1].getFloatValue();
                            newOrientation.yIn = receivedSerialData[2].getFloatValue();
                            newOrientation.zIn = receivedSerialData[3].getFloatValue();
                            orientation.setQuat(newOrientation);
                            return;
                        } else {
                            M1OrientationYPR newOrientation;
                            newOrientation.yaw = receivedSerialData[0].getFloatValue();
                            newOrientation.pitch = receivedSerialData[1].getFloatValue();
                            newOrientation.roll = receivedSerialData[2].getFloatValue();
                            orientation.setYPR(newOrientation);
                            return;
                        }
                    }
                }
            }
        }
        // TODO: This has to handle loosing the device. What if it's disconnected?
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
        // clear device list
        devices.clear();
        
        int port_number = comEnumerate();
        for(int port_index=0; port_index < port_number; port_index++) {
            std::cout << "[Serial] Found device: " << comGetPortName(port_index) << std::endl;
            portlist.set(comGetInternalName(port_index),comGetPortName(port_index));
            // We have to grab all the devices into the vector because the comPort is determined by the order of elements in `devices`
            devices.push_back({comGetPortName(port_index), M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial, comGetInternalName(port_index)});
            
            // TODO: implement filter for listed devices
            if (!displayOnlyKnownIMUs){
            /// SHOW ALL CONNECTABLE SERIAL DEVICES
            } else {
                /// SHOW KNOWN SERIAL DEVICES USING DEVICE NAME FILTERS
                std::string searchName = comGetPortName(port_index);
                if (searchName.find("Mach1-") != std::string::npos || searchName.find("HC-06-DevB") != std::string::npos || searchName.find("witDevice") != std::string::npos || searchName.find("m1YostDevice") != std::string::npos || searchName.find("usbmodem1434302") != std::string::npos || searchName.find("m1Device") != std::string::npos) {
                    /// SHOW MACH1 SERIAL/BT IMUs
                }
            }
        }
        /// SUPPERWARE MIDI SERIAL TESTING
        if (supperwareInterface.getTrackerDriver().canConnect()) {
            devices.push_back({"Supperware HT IMU", M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial, ""});
        }
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            
            int comPort = matchedDevice - devices.begin(); // get the device index of all found serial devices
            
            if (matchedDevice->getDeviceName().find("Supperware HT IMU") != std::string::npos) {
                /// CONNECT SUPPERWARE
                supperwareInterface.connectSupperware();
                if (supperwareInterface.getTrackerDriver().isConnected()) {
                    // Set global ref for device's index (used for disconnect)
                    connectedSerialPortIndex = comPort;
                    connectedDevice = *matchedDevice;
                    isConnected = true;
                    statusCallback(true, "ok");
                    return;
                } else {
                    statusCallback(false, "Supperware connection error");
                }
            } else {
                /// CONNECT ALL OTHER DEVICES
                std::string address = matchedDevice->getDeviceAddress();
                int port_state = comOpen(comPort, baudRate);
                if (port_state == 1) {
                    // Set global ref for device's index (used for disconnect)
                    connectedSerialPortIndex = comPort;
                    connectedDevice = *matchedDevice;
                    isConnected = true;
                    statusCallback(true, "ok");
                    return;
                }
            }
        }
        statusCallback(false, "not found");
    }

    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }
    
    // Callback update from the SupperwareInterface object
//    std::vector<float> trackerChanged(const HeadMatrix& headMatrix) override {
//
//    }
};
