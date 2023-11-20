//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"

class HardwareOSC : public HardwareAbstract, private juce::OSCReceiver, private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback> {
public:
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    bool isConnected = false;
    Orientation orientation;

    bool connectOscReceiver(int new_port) {
        if (isConnected) {
            juce::OSCReceiver::disconnect();
        }
        isConnected = connect(new_port);
        addListener(this);
        return isConnected;
    }

    void disconnectOscReceiver() {
        disconnect();
    }

    void oscMessageReceived(const juce::OSCMessage& message) override {
        M1OrientationYPR newOrientationYPR;
        M1OrientationQuat newOrientationQuat;

        /// GENERIC YPR
        
        /// Custom address pattern
        if (connectedDevice.osc_msg_addr_pttrn != "" && connectedDevice.osc_msg_addr_pttrn != "/orientation") {
            if (message.size() <= 3) {
                // we still allow just yaw or just yaw & pitch messages
                if (message[0].isFloat32()) {
                    newOrientationYPR.yaw = message[0].getFloat32();
                }
                if (message.size() <= 2 && message[1].isFloat32()) {
                    newOrientationYPR.pitch = message[1].getFloat32();
                }
                if (message.size() <= 3 && message[2].isFloat32()) {
                    newOrientationYPR.roll = message[2].getFloat32();
                }
                newOrientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
                orientation.setYPR(newOrientationYPR);
            } else if (message.size() == 4) {
                // we dont check for partial messages as quaternion requires all 4 for calculation
                M1OrientationQuat newQuat;
                if (message[0].isFloat32()) {
                    newQuat.wIn = message[0].getFloat32();
                } else {
                    // skip
                }
                if (message[1].isFloat32()) {
                    newQuat.xIn = message[1].getFloat32();
                } else {
                    // skip
                }
                if (message[2].isFloat32()) {
                    newQuat.yIn = message[2].getFloat32();
                } else {
                    // skip
                }
                if (message[3].isFloat32()) {
                    newQuat.zIn = message[3].getFloat32();
                } else {
                    // skip
                }
                orientation.setQuat(newQuat);
            } else {
                // return warning or error that we do not know how to parse this message
            }
        }
        /// Predefined address patterns
        else if ((message.getAddressPattern().toString() == "/orientation" ||
             message.getAddressPattern().toString().toStdString().find("xyz") != std::string::npos ||
             message.getAddressPattern().toString().toStdString().find("ypr") != std::string::npos) && message.size() == 3) {
            if (message[0].isFloat32()) {
                newOrientationYPR.yaw = message[0].getFloat32();
            }
            if (message[1].isFloat32()) {
                newOrientationYPR.pitch = message[1].getFloat32();
            }
            if (message[2].isFloat32()) {
                newOrientationYPR.roll = message[2].getFloat32();
            }
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
            orientation.setYPR(newOrientationYPR);
        }
        /// GENERIC QUATERNION
        else if ((message.getAddressPattern().toString().toStdString().find("quat") != std::string::npos || message.getAddressPattern().toString().toStdString().find("/quaternion") != std::string::npos) && message.size() == 4) {
            M1OrientationQuat newQuat;
            if (message[0].isFloat32()) {
                newQuat.wIn = message[0].getFloat32();
            } else {
                // skip
            }
            if (message[1].isFloat32()) {
                newQuat.xIn = message[1].getFloat32();
            } else {
                // skip
            }
            if (message[2].isFloat32()) {
                newQuat.yIn = message[2].getFloat32();
            } else {
                // skip
            }
            if (message[3].isFloat32()) {
                newQuat.zIn = message[3].getFloat32();
            } else {
                // skip
            }
            orientation.setQuat(newQuat);
        }
        /// BoseAR
        else if ((message.getAddressPattern().toString() == "/bosear/sensors/rotation_six_dof" || message.getAddressPattern().toString() == "/bosear/sensors/rotation_nine_dof") && message.size() == 3) {
            // [0] Pitch 0->270 up | 0->90 down
            // [1] Yaw 0->360
            // [2] Roll 0->270 left | 0->90 right
            newOrientationYPR.yaw = message[1].getFloat32() / 360;
            newOrientationYPR.pitch = (((((message[0].getFloat32() > 180.) ? abs(message[0].getFloat32() - 360.) : -message[0].getFloat32()))) * -1. + 90. ) / 180.;
            newOrientationYPR.roll = (((((message[2].getFloat32() > 180.) ? abs(message[2].getFloat32() - 360.) : message[2].getFloat32()))) * -1. + 90. ) / 180.;
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
            orientation.setYPR(newOrientationYPR);
        }
        /// TouchOSC
        else if ((message.getAddressPattern().toString() == "/accxyz") && message.size() == 3) {
            // TODO: parse accelerometer into primitive rotation data
            // Warning: this does not yet represent orientation
            newOrientationYPR.yaw = (((-1. - 1.) / (0. - 1.)) * ((message[0].getFloat32() - 360.) + 360.));
            newOrientationYPR.pitch = (((-1. - 1.) / (-180. - 1.)) * ((message[1].getFloat32() - 180.) + 180.));
            newOrientationYPR.roll = (((-1. - 1.) / (-180. - 1.)) * ((message[2].getFloat32() - 180.) + 180.));
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::SIGNED_NORMALLED;
            orientation.setYPR(newOrientationYPR);
        }
    }

    void oscBundleReceived(const juce::OSCBundle& bundle) override {
        juce::OSCBundle::Element elem = bundle[0];
        oscMessageReceived(elem.getMessage());
    }
    
    int setup() override {
        // pushback initial device
        devices.push_back({
            "OSC Input", // name
            M1OrientationDeviceType::M1OrientationManagerDeviceTypeOSC, // type
            "127.0.0.1" // address
        });
        return 1;
    }

    int update() override {
        if (isConnected){
            // callbacks are handled by override oscBundleReceived and oscMessageReceived
            return 1;
        } else {
            return -1;
        }
    }
    
    void calibrateDevice() override {
        // get current ypr and use that to offset the ypr
        
    }
    
    int close() override {
        disconnectOscReceiver();
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
        lock();
        this->devices = getDevices();
        unlock();
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            // TODO: save the input device instead of copying over the last settings
            matchedDevice->osc_port = device.osc_port;
            matchedDevice->osc_msg_addr_pttrn = device.osc_msg_addr_pttrn;
            connectedDevice = *matchedDevice;
            isConnected = connectOscReceiver(connectedDevice.osc_port); // initial connection with default port
            if (isConnected) {
                statusCallback(true, "OSC: Connected", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
                return;
            } else {
                statusCallback(false, "OSC: Error connecting to receiver", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
            }
        }
        statusCallback(false, "OSC: Not found", matchedDevice->getDeviceName(), (int)matchedDevice->getDeviceType(), matchedDevice->getDeviceAddress());
    }

    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }

};
