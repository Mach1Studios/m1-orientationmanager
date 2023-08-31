//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "HardwareAbstract.h"

class HardwareOSC : public HardwareAbstract, private juce::OSCReceiver, private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback> {
public:
    Orientation orientation;
    M1OrientationDeviceInfo connectedDevice;
    std::vector<M1OrientationDeviceInfo> devices;
    bool isConnected = false;
    int oscDevicePort = 9901;

    bool connectOscReceiver() {
        bool isConnected = connect(oscDevicePort);
        addListener(this);
        return isConnected;
    }

    void disconnectOscReceiver() {
        disconnect();
    }

    void oscMessageReceived(const juce::OSCMessage& message) override {
        /// GENERIC YPR
        M1OrientationYPR newOrientationYPR;
        M1OrientationQuat newOrientationQuat;
        if ((message.getAddressPattern().toString() == "/orientation" || message.getAddressPattern().toString().toStdString().find("ypr") != std::string::npos || message.getAddressPattern().toString().toStdString().find("xyz") != std::string::npos) && message.size() == 3) {
            if (message[0].isFloat32()) {
                newOrientationYPR.yaw = message[0].getFloat32();
            } else if (message[0].isInt32()) {
                newOrientationYPR.yaw = (float)message[0].getInt32();
            }
            if (message[1].isFloat32()) {
                newOrientationYPR.pitch = message[1].getFloat32();
            } else if (message[1].isInt32()) {
                newOrientationYPR.pitch = (float)message[1].getInt32();
            }
            if (message[2].isFloat32()) {
                newOrientationYPR.roll = message[2].getFloat32();
            } else if (message[2].isInt32()) {
                newOrientationYPR.roll = (float)message[2].getInt32();
            }
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
            orientation.setYPR(newOrientationYPR);
        }
        /// GENERIC QUATERNION
        else if ((message.getAddressPattern().toString().toStdString().find("quat") != std::string::npos || message.getAddressPattern().toString().toStdString().find("/quaternion") != std::string::npos) && message.size() == 4) {
            if (message[0].isFloat32()) {
                newOrientationQuat.wIn = message[0].getFloat32();
            } else if (message[0].isInt32()) {
                newOrientationQuat.wIn = (float)message[0].getInt32();
            }
            if (message[1].isFloat32()) {
                newOrientationQuat.xIn = message[1].getFloat32();
            } else if (message[1].isInt32()) {
                newOrientationQuat.xIn = (float)message[1].getInt32();
            }
            if (message[2].isFloat32()) {
                newOrientationQuat.yIn = message[2].getFloat32();
            } else if (message[2].isInt32()) {
                newOrientationQuat.yIn = (float)message[2].getInt32();
            }
            if (message[3].isFloat32()) {
                newOrientationQuat.zIn = message[3].getFloat32();
            } else if (message[3].isInt32()) {
                newOrientationQuat.zIn = (float)message[3].getInt32();
            }
            orientation.setQuat(newOrientationQuat);
        }
        /// BoseAR
        else if ((message.getAddressPattern().toString() == "/bosear/sensors/rotation_six_dof" || message.getAddressPattern().toString() == "/bosear/sensors/rotation_nine_dof") && message.size() == 3) {
            // [0] Pitch 0->270 up | 0->90 down
            // [1] Yaw 0->360
            // [2] Roll 0->270 left | 0->90 right
            newOrientationYPR.yaw = message[1].getFloat32() / 360;
            newOrientationYPR.pitch = (((((message[0].getFloat32() > 180.) ? abs(message[0].getFloat32() - 360.) : -message[0].getFloat32()))) * -1. + 90. ) / 180.;
            newOrientationYPR.roll = (((((message[2].getFloat32() > 180.) ? abs(message[2].getFloat32() - 360.) : message[2].getFloat32()))) * -1. + 90. ) / 180.;
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::SIGNED_NORMALLED;
            orientation.setYPR(newOrientationYPR);
        }
        /// TouchOSC
        else if ((message.getAddressPattern().toString() == "/accxyz") && message.size() == 3) {
            // TODO: fix parsing!
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
        refreshDevices();
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
        // clear device list
        devices.clear();
        
        // TODO: create OSC object and pushback
        devices.push_back({ "OSC Input", M1OrientationDeviceType::M1OrientationManagerDeviceTypeOSC, std::to_string(oscDevicePort) });
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            connectedDevice = *matchedDevice;
            isConnected = connectOscReceiver();
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
