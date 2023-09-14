//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

// Concept:
//   Search for device and filter by device name to expected device Interface
//   Connect to any device freely (some devices have specific connection requirements like the Supperware)
//   Update devices and use name filtering again to do specific update routines

// Orientation Flow:
//   Devices can only offset the current orientation which is sent to clients

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"
#include "rs232/rs232.h"

// include device specific
#include "Devices/SupperwareInterface.h"
#include "Devices/M1Interface.h"
#include "Devices/WitMotionInterface.h"

class HardwareSerial : public HardwareAbstract {
public:
    Orientation orientation;
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    int baudRate = 115200;
    int connectedSerialPortIndex;
    bool isConnected = false;
    juce::StringPairArray portlist;
    
    // previous value storage for non-defined devices
    M1OrientationYPR prev_ypr;
    M1OrientationQuat prev_q;

    // Device Interfaces
    M1Interface m1Interface;
    SupperwareInterface supperwareInterface;
    WitMotionInterface witmotionInterface;

    int setup() override {
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
                    
                    supperwareInterface.previousOrientation.resize(supperwareInterface.currentOrientation.size());
                    
                    if (supperwareInterface.currentOrientation.size() == 3) {
                        M1OrientationYPR new_orientation_delta;
                        
                        // calculate the delta
                        new_orientation_delta.yaw = supperwareInterface.currentOrientation[0] - supperwareInterface.previousOrientation[0];
                        new_orientation_delta.pitch = supperwareInterface.currentOrientation[1] - supperwareInterface.previousOrientation[1];
                        new_orientation_delta.roll = supperwareInterface.currentOrientation[2] - supperwareInterface.previousOrientation[2];
                        new_orientation_delta.angleType = M1OrientationYPR::AngleType::DEGREES;
                        new_orientation_delta.yaw_min = 0.0f, new_orientation_delta.pitch_min = -180.0f, new_orientation_delta.roll_min = -180.0f;
                        new_orientation_delta.yaw_max = 360.0f, new_orientation_delta.pitch_max = 180.0f, new_orientation_delta.roll_max = 180.0f;
                                                
                        // apply the delta as offset
                        M1OrientationYPR new_orientation_delta_normalled = orientation.getUnsignedNormalled(new_orientation_delta);
                        orientation.offsetYPR(new_orientation_delta_normalled);
                        
                        // update the previous for next calculation
                        supperwareInterface.previousOrientation[0] = supperwareInterface.currentOrientation[0];
                        supperwareInterface.previousOrientation[1] = supperwareInterface.currentOrientation[1];
                        supperwareInterface.previousOrientation[2] = supperwareInterface.currentOrientation[2];
                        return 1;
                        
                    } else if (supperwareInterface.currentOrientation.size() == 4) {
                        M1OrientationQuat new_orientation_delta;
                        // QInitial * QTransition = QFinal
                        // QTransition = QFinal * inverse(QInitial)
                        new_orientation_delta.wIn = supperwareInterface.currentOrientation[0] * supperwareInterface.previousOrientation[0];
                        new_orientation_delta.xIn = supperwareInterface.currentOrientation[1] * -supperwareInterface.previousOrientation[1];
                        new_orientation_delta.yIn = supperwareInterface.currentOrientation[2] * -supperwareInterface.previousOrientation[2];
                        new_orientation_delta.zIn = supperwareInterface.currentOrientation[3] * -supperwareInterface.previousOrientation[3];
                        
                        // apply the delta as offset
                        orientation.offsetQuat(new_orientation_delta);
                        
                        // update the previous for next calculation
                        supperwareInterface.previousOrientation[0] = supperwareInterface.currentOrientation[0];
                        supperwareInterface.previousOrientation[1] = supperwareInterface.currentOrientation[1];
                        supperwareInterface.previousOrientation[2] = supperwareInterface.currentOrientation[2];
                        supperwareInterface.previousOrientation[3] = supperwareInterface.currentOrientation[3];
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
                    if (getConnectedDevice().getDeviceName().find("Mach1-") != std::string::npos || getConnectedDevice().getDeviceName().find("HC-06-DevB") != std::string::npos ||  getConnectedDevice().getDeviceName().find("usbmodem1434302") != std::string::npos || getConnectedDevice().getDeviceName().find("m1Device") != std::string::npos) {

                        m1Interface.updateOrientation(queueString, queueBuffer);
                        if (m1Interface.anythingNewDetected) {
                            M1OrientationYPR new_orientation_delta;
                            
                            // calculate the delta
                            new_orientation_delta.yaw = m1Interface.decoded.y - m1Interface.last_decoded.y;
                            new_orientation_delta.pitch = m1Interface.decoded.p - m1Interface.last_decoded.p;
                            new_orientation_delta.roll = m1Interface.decoded.r - m1Interface.last_decoded.r;
                            new_orientation_delta.angleType = M1OrientationYPR::AngleType::DEGREES;
                            new_orientation_delta.yaw_min = -180.0f, new_orientation_delta.pitch_min = -180.0f, new_orientation_delta.roll_min = -180.0f;
                            new_orientation_delta.yaw_max = 180.0f, new_orientation_delta.pitch_max = 180.0f, new_orientation_delta.roll_max = 180.0f;

                            // apply the delta as offset
                            M1OrientationYPR new_orientation_delta_normalled = orientation.getUnsignedNormalled(new_orientation_delta);
                            orientation.offsetYPR(new_orientation_delta_normalled);
                            
                            // update the previous for next calculation
                            m1Interface.last_decoded.y = m1Interface.decoded.y;
                            m1Interface.last_decoded.p = m1Interface.decoded.p;
                            m1Interface.last_decoded.r = m1Interface.decoded.r;

                            // cleanup
                            queueBuffer.clear();
                            queueString.clear();
                            return 1;
                        }
                        
                    } else if (getConnectedDevice().getDeviceName().find("wit") != std::string::npos) {
                        /// UPDATES FOR WITMOTION DEVICES
                        float* witOrientationAngles = witmotionInterface.updateOrientation(readBuffer, 128);
                        M1OrientationYPR new_orientation_delta;
                        
                        // calculate the delta
                        new_orientation_delta.yaw = witOrientationAngles[0] - witmotionInterface.prev_angle[0];
                        new_orientation_delta.pitch = witOrientationAngles[1] - witmotionInterface.prev_angle[1];
                        new_orientation_delta.roll = witOrientationAngles[2] - witmotionInterface.prev_angle[2];
                        new_orientation_delta.angleType = M1OrientationYPR::AngleType::DEGREES;
                        new_orientation_delta.yaw_min = -180.0f, new_orientation_delta.pitch_min = -180.0f, new_orientation_delta.roll_min = -180.0f;
                        new_orientation_delta.yaw_max = 180.0f, new_orientation_delta.pitch_max = 180.0f, new_orientation_delta.roll_max = 180.0f;
                        
                        // apply the delta as offset
                        M1OrientationYPR new_orientation_delta_normalled = orientation.getUnsignedNormalled(new_orientation_delta);
                        orientation.offsetYPR(new_orientation_delta_normalled);
                        
                        // update the previous for next calculation
                        witmotionInterface.prev_angle[0] = witOrientationAngles[0];
                        witmotionInterface.prev_angle[1] = witOrientationAngles[1];
                        witmotionInterface.prev_angle[2] = witOrientationAngles[2];
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
                                M1OrientationQuat new_orientation_delta;
                                
                                new_orientation_delta.wIn = receivedSerialData[0].getFloatValue() * prev_q.w;
                                new_orientation_delta.xIn = receivedSerialData[1].getFloatValue() * -prev_q.x;
                                new_orientation_delta.yIn = receivedSerialData[2].getFloatValue() * -prev_q.y;
                                new_orientation_delta.zIn = receivedSerialData[3].getFloatValue() * -prev_q.z;
                                
                                // apply the delta as offset
                                orientation.offsetQuat(new_orientation_delta);
                                
                                // update the previous for next calculation
                                prev_q.w = receivedSerialData[0].getFloatValue();
                                prev_q.x = receivedSerialData[1].getFloatValue();
                                prev_q.y = receivedSerialData[2].getFloatValue();
                                prev_q.z = receivedSerialData[3].getFloatValue();
                                return 1;
                                
                            } else if (receivedSerialData.size() == 3) {
                                M1OrientationYPR new_orientation_delta;
                                
                                // calculate the delta
                                new_orientation_delta.yaw = receivedSerialData[0].getFloatValue() - prev_ypr.yaw;
                                new_orientation_delta.pitch = receivedSerialData[1].getFloatValue() - prev_ypr.pitch;
                                new_orientation_delta.roll = receivedSerialData[2].getFloatValue() - prev_ypr.roll;
                                new_orientation_delta.angleType = M1OrientationYPR::AngleType::DEGREES;
                                new_orientation_delta.yaw_min = -180.0f, new_orientation_delta.pitch_min = -180.0f, new_orientation_delta.roll_min = -180.0f;
                                new_orientation_delta.yaw_max = 180.0f, new_orientation_delta.pitch_max = 180.0f, new_orientation_delta.roll_max = 180.0f;
                                
                                // apply the delta as offset
                                M1OrientationYPR new_orientation_delta_normalled = orientation.getUnsignedNormalled(new_orientation_delta);
                                orientation.offsetYPR(new_orientation_delta_normalled);
                                
                                // update the previous for next calculation
                                prev_ypr.yaw = receivedSerialData[0].getFloatValue();
                                prev_ypr.pitch = receivedSerialData[1].getFloatValue();
                                prev_ypr.roll = receivedSerialData[2].getFloatValue();
                                
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
        // Added to the end of the serial port search to not break the serial port enumeration from just above which is critical for connection
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
};
