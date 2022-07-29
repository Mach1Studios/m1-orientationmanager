#pragma once

#include <JuceHeader.h>
#include "M1OrientationTypes.h"
#include "M1OrientationSettings.h"
#include "M1OrientationHardwareAbstract.h"

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

    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;

    std::function<void(const juce::OSCMessage& message)> callback = nullptr;

    void oscMessageReceived(const juce::OSCMessage& message) override;
    void send(const std::vector<M1OrientationClientConnection>& clients, std::string str);
    void send(const std::vector<M1OrientationClientConnection>& clients, juce::OSCMessage& msg);

    void send_getDevices(const std::vector<M1OrientationClientConnection>& clients);
    void send_getCurrentDevice(const std::vector<M1OrientationClientConnection>& clients);
    void send_getTrackingYawEnabled(const std::vector<M1OrientationClientConnection>& clients);
    void send_getTrackingPitchEnabled(const std::vector<M1OrientationClientConnection>& clients);
    void send_getTrackingRollEnabled(const std::vector<M1OrientationClientConnection>& clients);

    std::map<M1OrientationDeviceType, M1OrientationHardwareAbstract*> hardwareImpl;
    M1OrientationDevice currentDevice = { "", M1OrientationManagerDeviceTypeNone };
    Orientation orientation;

public:
    
    virtual ~M1OrientationOSCServer();

    bool init(int serverPort);

    void update();

    Orientation getOrientation();

    void addHardwareImplementation(M1OrientationDeviceType type, M1OrientationHardwareAbstract* impl);

    std::vector<M1OrientationClientConnection> getClients();

    std::vector<M1OrientationDevice> getDevices();
    M1OrientationDevice getCurrentDevice();

    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();

    void command_refreshDevices();
    void command_startTrackingUsingDevice(M1OrientationDevice device);
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);

    void setCallback(std::function<void(const juce::OSCMessage& message)> callback);
    void close();
};
