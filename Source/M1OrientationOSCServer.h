//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "m1_orientation_client/M1OrientationTypes.h"
#include "m1_orientation_client/M1OrientationSettings.h"
#include "HardwareAbstract.h"
#include <thread>

struct M1OrientationClientConnection {
    int port;
    juce::int64 time;
};

class M1OrientationOSCServer : 
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>, 
    public M1OrientationManagerOSCSettings
{
    juce::OSCReceiver receiver; 

    std::vector<M1OrientationClientConnection> clients;
    int serverPort = 0;
    int watcherPort = 0;
    bool isRunning = false;

    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;

    void oscMessageReceived(const juce::OSCMessage& message) override;
    bool send(const std::vector<M1OrientationClientConnection>& clients, std::string str);
    bool send(const std::vector<M1OrientationClientConnection>& clients, juce::OSCMessage& msg);

    void send_getDevices(const std::vector<M1OrientationClientConnection>& clients);
    void send_getCurrentDevice(const std::vector<M1OrientationClientConnection>& clients);
    void send_getTrackingYawEnabled(const std::vector<M1OrientationClientConnection>& clients);
    void send_getTrackingPitchEnabled(const std::vector<M1OrientationClientConnection>& clients);
    void send_getTrackingRollEnabled(const std::vector<M1OrientationClientConnection>& clients);

    std::map<M1OrientationDeviceType, HardwareAbstract*> hardwareImpl;
    M1OrientationDeviceInfo currentDevice;
    Orientation orientation;

public:
    
    virtual ~M1OrientationOSCServer();

    bool init(int serverPort, int watcherPort);

    void update();

    Orientation getOrientation();

    void addHardwareImplementation(M1OrientationDeviceType type, HardwareAbstract* impl);

    std::vector<M1OrientationClientConnection> getClients();
    std::vector<M1OrientationDeviceInfo> getDevices();
    M1OrientationDeviceInfo getConnectedDevice();

    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();

    void command_refreshDevices();
    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_recenter();
    void command_disconnect();

    void close();
};
