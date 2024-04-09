//
//  m1-orientationmanager
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
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    int baudRate = 115200;
    int connectedSerialPortIndex;
    bool isConnected = false;
    juce::StringPairArray portlist;
    
    Mach1::Orientation orientation;
    Mach1::Float3 current;
    Mach1::Float3 previous;

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
        if (!isConnected) {
            return -1; // return for error handling
        }

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
        if (getConnectedDevice().isDeviceName("Supperware HT IMU")) {
            if (!supperwareInterface.getTrackerDriver().isConnected()) {
                return -1; // TODO: error for being abstractly connected but not literally
            }
            if (supperwareInterface.currentOrientation.size() == 3) {
                float yaw, pitch, roll;
                yaw = supperwareInterface.currentOrientation[0];
                pitch = supperwareInterface.currentOrientation[1];
                roll = supperwareInterface.currentOrientation[2];
                orientation.SetRotation(Mach1::Float3{pitch, yaw, roll}.EulerRadians());
                return 1;
            } else if (supperwareInterface.currentOrientation.size() == 4) {
                float w, x, y, z;
                w = supperwareInterface.currentOrientation[0];
                x = supperwareInterface.currentOrientation[1];
                y = supperwareInterface.currentOrientation[2];
                z = supperwareInterface.currentOrientation[3];
                orientation.SetRotation({w, x, y, z});
                return 1;
            } else {
                // error or do nothing
                return -1;
            }
        }

        else {
            /// ORIENTATION UPDATE: SERIAL
            while ((queueString.length() > 0) || (queueBuffer.size() > 0)) {
                /// UPDATES FOR KNOWN MACH1 IMUs
                if (getConnectedDevice().isDeviceName("Mach1-") ||
                    getConnectedDevice().isDeviceName("HC-06-DevB") ||
                    getConnectedDevice().isDeviceName("witDevice") ||
                    getConnectedDevice().isDeviceName("m1YostDevice") ||
                    getConnectedDevice().isDeviceName("usbmodem") ||
                    getConnectedDevice().isDeviceName("usbmodem1434302") ||
                    getConnectedDevice().isDeviceName("m1Device"))
                {

                    m1Interface.updateOrientation(queueString, queueBuffer);
                    if (m1Interface.anythingNewDetected) {
                        float yaw, pitch, roll;
                        yaw = m1Interface.decoded.y;
                        pitch = m1Interface.decoded.p;
                        roll = m1Interface.decoded.r;

                        orientation.SetRotation({pitch, yaw, roll});

                        // cleanup
                        queueBuffer.clear();
                        queueString.clear();
                        return 1;
                    }
                } else if (getConnectedDevice().getDeviceName().find("wit") != std::string::npos) {
                    /// UPDATES FOR WITMOTION DEVICES
                    float* witOrientationAngles = witmotionInterface.updateOrientation(readBuffer, 128);

                    float yaw, pitch, roll;
                    yaw = witOrientationAngles[0];
                    pitch = witOrientationAngles[1];
                    roll = witOrientationAngles[2];
                    orientation.SetRotation({pitch, yaw, roll});
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
                            float w, x, y, z;
                            w = receivedSerialData[0].getFloatValue();
                            x = receivedSerialData[1].getFloatValue();
                            y = receivedSerialData[2].getFloatValue();
                            z = receivedSerialData[3].getFloatValue();
                            orientation.SetRotation(Mach1::Quaternion{w, x, y, z});
                            return 1;
                        } else if (receivedSerialData.size() == 3) {
                            // TODO: for safety if previous string arrays were 4 float value captures maybe skip this?
                            float yaw, pitch, roll;
                            yaw = receivedSerialData[0].getFloatValue();
                            pitch = receivedSerialData[1].getFloatValue();
                            roll = receivedSerialData[2].getFloatValue();
                            orientation.SetRotation(Mach1::Float3{pitch, yaw, roll});
                            return 1;
                        } else {
                            // ignore incomplete messages
                        }
                    }
                }
            }
        }
        return 1;
    }
    
    void calibrateDevice() override {
        if ((getConnectedDevice().isDeviceName("Supperware HT IMU")) && supperwareInterface.getTrackerDriver().isConnected()) {
            supperwareInterface.calibrateCompass();
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
		std::vector<M1OrientationDeviceInfo> devices;
        
        int port_number = comEnumerate();
        for(int port_index=0; port_index < port_number; port_index++) {
            std::cout << "[Serial] Found device: " << comGetPortName(port_index) << std::endl;
            portlist.set(comGetInternalName(port_index),comGetPortName(port_index));
            // We have to grab all the devices into the vector because the comPort is determined by the order of elements in `devices`
            devices.push_back({comGetPortName(port_index), M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial, comGetInternalName(port_index)});
        }
        /// SUPPERWARE MIDI SERIAL TESTING
        // Added to the end of the serial port search to not break the serial port enumeration from just above which is critical for connection
        juce::WaitableEvent completionEvent;
        juce::MessageManager::callAsync([&]() {
            if (supperwareInterface.getTrackerDriver().canConnect()) {
				devices.push_back({"Supperware HT IMU", M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial, ""});
            }
            completionEvent.signal();
        });
        completionEvent.wait();
     
		lock();
		this->devices = devices;
		unlock();
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, TrackingCallback statusCallback) override {
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

    void recenter() override {
        orientation.Recenter();
    }

};
