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
    int oscReceiverPort = 9901;

    bool connectOscReceiver() {
        bool isConnected = connect(oscReceiverPort);
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
            newOrientationYPR.yaw = message[0].getFloat32();
            newOrientationYPR.pitch = message[1].getFloat32();
            newOrientationYPR.roll = message[2].getFloat32();
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::DEGREES;
            orientation.setYPR(newOrientationYPR);
        }
        /// GENERIC QUATERNION
        else if ((message.getAddressPattern().toString().toStdString().find("quat") != std::string::npos || message.getAddressPattern().toString().toStdString().find("/quaternion") != std::string::npos) && message.size() == 4) {
            newOrientationQuat.wIn = message[0].getFloat32();
            newOrientationQuat.xIn = message[1].getFloat32();
            newOrientationQuat.yIn = message[2].getFloat32();
            newOrientationQuat.zIn = message[3].getFloat32();
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
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::NORMALED;
            orientation.setYPR(newOrientationYPR);
        }
        /// TouchOSC
        else if ((message.getAddressPattern().toString() == "/accxyz") && message.size() == 3) {
            // TODO: fix parsing!
            newOrientationYPR.yaw = (((-1. - 1.) / (0. - 1.)) * ((message[0].getFloat32() - 360.) + 360.));
            newOrientationYPR.pitch = (((-1. - 1.) / (-180. - 1.)) * ((message[1].getFloat32() - 180.) + 180.));
            newOrientationYPR.roll = (((-1. - 1.) / (-180. - 1.)) * ((message[2].getFloat32() - 180.) + 180.));
            newOrientationYPR.angleType = M1OrientationYPR::AngleType::NORMALED;
            orientation.setYPR(newOrientationYPR);
        }
    }

    void oscBundleReceived(const juce::OSCBundle& bundle) override {
        juce::OSCBundle::Element elem = bundle[0];
        oscMessageReceived(elem.getMessage());
    }
    
    void setup() override {
        refreshDevices();
    }

    void update() override {
        if (isConnected){
            // callbacks are handled by override oscBundleReceived and oscMessageReceived
        }
    }
    
    void close() override {
        disconnectOscReceiver();
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
        
        // TODO: create OSC object and pushback
        devices.push_back({ "OSC Input", M1OrientationDeviceType::M1OrientationManagerDeviceTypeOSC, std::to_string(oscReceiverPort) });
    }

    std::vector<M1OrientationDeviceInfo> getDevices() override {
        return devices;
    }

    void startTrackingUsingDevice(M1OrientationDeviceInfo device, std::function<void(bool success, std::string errorMessage)> statusCallback) override {
        auto matchedDevice = std::find_if(devices.begin(), devices.end(), M1OrientationDeviceInfo::find_id(device.getDeviceName()));
        if (matchedDevice != devices.end()) {
            connectedDevice = *matchedDevice;
            isConnected = connectOscReceiver();
            if (isConnected) {
                statusCallback(true, "OSC: Connected");
                return;
            } else {
                statusCallback(false, "OSC: Error connecting to receiver");
            }
        }
        statusCallback(false, "OSC: Not found");
    }

    M1OrientationDeviceInfo getConnectedDevice() override {
        return connectedDevice;
    }

};
