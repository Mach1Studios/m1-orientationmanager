//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

// Concept:
//   Search for device and filter by device name to expected device Interface
//   Connect to any device freely (some devices have specific connection requirements like the Supperware)
//   Update devices and use name filtering again to do specific update routines

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"
#include "rs232/rs232.h"

// include device specific
#include "Devices/SupperwareInterface.h"
#include "Devices/M1Interface.h"
#include "Devices/WitMotionInterface.h"

class HardwareSerial : public HardwareAbstract/*, SupperwareInterface::Listener*/ {
public:
    Orientation orientation;
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    int baudRate = 115200;
    int connectedSerialPortIndex;
    bool isConnected = false;
    juce::StringPairArray portlist;

    // Device Interfaces
    M1Interface m1Interface;
    SupperwareInterface supperwareInterface;
    WitMotionInterface witmotionInterface;

    int setup() override {
        // TODO: move the orientation update into this listener callback
        //supperwareInterface.setListener(this);
        refreshDevices();
        return 1;
    }

    int update() override {
        if (isConnected){
            char readBuffer[128];
            comRead(connectedSerialPortIndex, readBuffer, 128);
            
            // reformatting input data for some device types
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
                        return 1;
                    } else if (supperwareInterface.currentOrientation.size() == 4) {
                        M1OrientationQuat newOrientation;
                        newOrientation.wIn = supperwareInterface.currentOrientation[0];
                        newOrientation.xIn = supperwareInterface.currentOrientation[1];
                        newOrientation.yIn = supperwareInterface.currentOrientation[2];
                        newOrientation.zIn = supperwareInterface.currentOrientation[3];
                        orientation.setQuat(newOrientation);
                        return 1;
                    } else {
                        // error or do nothing
                        return -1;
                    }
                } else {
                    // TODO: error for being abstractly connected by not literally
                    return -1;
                }
            } else {
                /// ORIENTATION UPDATE: SERIAL
                while ((queueString.length() > 0) || (queueBuffer.size() > 0)) {
                    /// UPDATES FOR KNOWN MACH1 IMUs
                    if (getConnectedDevice().getDeviceName().find("Mach1-") != std::string::npos || getConnectedDevice().getDeviceName().find("HC-06-DevB") != std::string::npos || getConnectedDevice().getDeviceName().find("witDevice") != std::string::npos || getConnectedDevice().getDeviceName().find("m1YostDevice") != std::string::npos || getConnectedDevice().getDeviceName().find("usbmodem") != std::string::npos ||
                        getConnectedDevice().getDeviceName().find("usbmodem1434302") != std::string::npos || getConnectedDevice().getDeviceName().find("m1Device") != std::string::npos) {

                        m1Interface.updateOrientation(queueString, queueBuffer);
                        if (m1Interface.anythingNewDetected) {
                            M1OrientationYPR newOrientation;
                            newOrientation.yaw = m1Interface.decoded.y;
                            newOrientation.pitch = m1Interface.decoded.p;
                            newOrientation.roll = m1Interface.decoded.r;
                            newOrientation.angleType = M1OrientationYPR::DEGREES;
                            orientation.setYPR(newOrientation);
                            // cleanup
                            queueBuffer.clear();
                            queueString.clear();
                            return 1;
                        }
                    } else if (getConnectedDevice().getDeviceName().find("wit") != std::string::npos) {
                        /// UPDATES FOR WITMOTION DEVICES
                        float* witOrientationAngles = witmotionInterface.updateOrientation(readBuffer, 128);
                        M1OrientationYPR newOrientation;
                        newOrientation.yaw = witOrientationAngles[0];
                        newOrientation.pitch = witOrientationAngles[1];
                        newOrientation.roll = witOrientationAngles[2];
                        newOrientation.angleType = M1OrientationYPR::DEGREES;
                        orientation.setYPR(newOrientation);
                        return 1;
                    } else {
                        /// UPDATES FOR GENERIC DEVICES
                        
                        // test if incoming data uses ";" break character
                        juce::StringArray receivedSerialDataLines = juce::StringArray::fromTokens(readBuffer, ";", "\""); // break lines from ";" character
                        
                        if (receivedSerialDataLines.size() > 1) {
                            // we can assume the break character is ";" and will process
                        } else {
                            // clearing the test above and recapturing assuming return lines are being used per reading
                            receivedSerialDataLines.clear();
                            receivedSerialDataLines = juce::StringArray::fromLines(readBuffer); // break lines from lines
                        }
                        
                        for (int i = 0; i < receivedSerialDataLines.size(); ++i) {
                            juce::StringArray receivedSerialData = juce::StringArray::fromTokens(receivedSerialDataLines[i], ",", "\""); // expect delimited characters of ,
                            
                            if(receivedSerialData.size() == 4) {
                                M1OrientationQuat newOrientation;
                                newOrientation.wIn = receivedSerialData[0].getFloatValue();
                                newOrientation.xIn = receivedSerialData[1].getFloatValue();
                                newOrientation.yIn = receivedSerialData[2].getFloatValue();
                                newOrientation.zIn = receivedSerialData[3].getFloatValue();
                                orientation.setQuat(newOrientation);
                                return 1;
                            } else if (receivedSerialData.size() == 3) {
                                // TODO: for safety if previous string arrays were 4 float value captures maybe skip this? 
                                M1OrientationYPR newOrientation;
                                newOrientation.yaw = receivedSerialData[0].getFloatValue();
                                newOrientation.pitch = receivedSerialData[1].getFloatValue();
                                newOrientation.roll = receivedSerialData[2].getFloatValue();
                                orientation.setYPR(newOrientation);
                                return 1;
                            } else {
                                // ignore incomplete messages
                            }
                        }
                    }
                }
            }
            return 1;
        } else {
            // return for error handling
            return -1;
        }
    }
    
    int close() override {
        comClose(connectedSerialPortIndex);
        isConnected = false;
        return 1;
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
        }
        /// SUPPERWARE MIDI SERIAL TESTING
        // Added to the end of the serial port search to not break the serial port enumeration which is critical for connection
        juce::WaitableEvent completionEvent;
        juce::MessageManager::callAsync([this, &completionEvent]() {
            if (supperwareInterface.getTrackerDriver().canConnect()) {
                devices.push_back({"Supperware HT IMU", M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial, ""});
            }
            completionEvent.signal();
        });
        completionEvent.wait();
        
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            
            int comPort = matchedDevice - devices.begin(); // get the device index of all found serial devices
            
            if (matchedDevice->getDeviceName().find("Supperware HT IMU") != std::string::npos) {
                /// CONNECT SUPPERWARE
                juce::WaitableEvent completionEvent;
                juce::MessageManager::callAsync([this, &completionEvent]() {
                    supperwareInterface.connectSupperware();
                    completionEvent.signal();
                });
                completionEvent.wait();
                                
                if (supperwareInterface.getTrackerDriver().isConnected()) {
                    // Set global ref for device's index (used for disconnect)
                    connectedSerialPortIndex = comPort;
                    connectedDevice = *matchedDevice;
                    statusCallback(true, "Serial: Supperware Connected", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
                    isConnected = true;
                    return;
                } else {
                    statusCallback(false, "Serial: Supperware connection error", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
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
                    statusCallback(true, "Serial: Connected", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
                    return;
                }
            }
        }
        statusCallback(false, "Serial: Not found", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
    }

    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }
    
    // Callback update from the SupperwareInterface object
//    std::vector<float> trackerChanged(const HeadMatrix& headMatrix) override {
//
//    }
};
