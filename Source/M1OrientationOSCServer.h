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

// TODO: refactor this class and the find_plugin struct
class M1RegisteredPlugin {
public:
    // At a minimum we should expect port and if applicable name
    // messages to ensure we do not delete this instance of a registered plugin
    int port;
    std::string name;
    bool isPannerPlugin = false;
    int input_mode;
    float azimuth, elevation, diverge; // values expected unnormalized
    float gain; // values expected unnormalized
    
    // pointer to store osc sender to communicate to registered plugin
    juce::OSCSender* messageSender;
};

// search plugin by registered port number
// TODO: potentially improve this with uuid concept
struct find_plugin
{
    int port;
    find_plugin(int port) : port(port) {}
    bool operator () ( const M1RegisteredPlugin& p ) const
    {
        return p.port == port;
    }
};

struct M1OrientationClientConnection {
    int port;
    juce::int64 time;
};

class M1OrientationOSCServer : 
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>, 
    public M1OrientationManagerOSCSettings, juce::Timer
{
    juce::OSCReceiver receiver; 

    std::vector<M1OrientationClientConnection> clients;
    int serverPort = 0;
    int watcherPort = 0;
    bool isRunning = false;
    
    float monitor_yaw, monitor_pitch, monitor_roll;
    int monitor_mode = 0;

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

    bool init(int serverPort, int watcherPort, bool useWatcher);

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

    // Tracking for any plugin that does not need an m1_orientation_client but still needs feedback of orientation for UI purposes such as the M1-Panner plugin
    std::vector<M1RegisteredPlugin> registeredPlugins;
    bool bTimerActive = false;
    
    void timerCallback() override {
        if (registeredPlugins.size() > 0) {
            for (auto &i: registeredPlugins) {
                juce::OSCMessage m = juce::OSCMessage(juce::OSCAddressPattern("/monitor-settings"));
                m.addInt32(monitor_mode);
                m.addFloat32(monitor_yaw); // expected normalised
                m.addFloat32(monitor_pitch); // expected normalised
                m.addFloat32(monitor_roll); // expected normalised
                i.messageSender->send(m);
            }
        }
        
        // TODO: check if any registered plugins closed
        for (auto &i: registeredPlugins) {
            
        }
    }
    
    void close();
};
