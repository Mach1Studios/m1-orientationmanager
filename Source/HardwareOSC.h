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
    int oscReceiverPort = 8888;

    bool connectOscReceiver() {
        bool isConnected = connect(oscReceiverPort);
        addListener(this);
        return isConnected;
    }

    void disconnectOscReceiver() {
        disconnect();
    }

    void oscMessageReceived(const juce::OSCMessage& message) override {
        if (message.getAddressPattern().toString() == "/orientation" && message.size() == 3) {
            M1OrientationYPR newOrientation;
            newOrientation.yaw = message[0].getFloat32();
            newOrientation.pitch = message[1].getFloat32();
            newOrientation.roll = message[2].getFloat32();
            orientation.setYPR(newOrientation);
        }
        if (message.getAddressPattern().toString() == "/orientation-quat" && message.size() == 4) {
            M1OrientationQuat newOrientation;
            newOrientation.wIn = message[0].getFloat32();
            newOrientation.xIn = message[1].getFloat32();
            newOrientation.yIn = message[2].getFloat32();
            newOrientation.zIn = message[3].getFloat32();
            orientation.setQuat(newOrientation);
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
