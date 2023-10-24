//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//
#pragma once

#include <JuceHeader.h>
#include "m1_orientation_client/M1OrientationTypes.h"
#include "m1_orientation_client/M1OrientationSettings.h"
#include "HardwareAbstract.h"
#include <thread>
#include <map>

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
struct find_plugin {
    int port;
    find_plugin(int port) : port(port) {}
    bool operator () ( const M1RegisteredPlugin& p ) const
    {
        return p.port == port;
    }
};

struct M1OrientationClientConnection {
    int port;
    std::string type = "";
    juce::int64 time;
};

struct find_client_by_type {
    std::string type;
    find_client_by_type(std::string type) : type(type) {}
    bool operator () ( const M1OrientationClientConnection& c ) const
    {
        return c.type == type;
    }
};

class M1OrientationManager : 
    private juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>, 
    public M1OrientationManagerOSCSettings, juce::Timer
{
	httplib::Server server;
	std::mutex mutex;
	std::string stringForClient;

	juce::OSCReceiver receiver;

    std::vector<M1OrientationClientConnection> clients;
    int serverPort = 0;
    int watcherPort = 0;

	float playerPositionInSeconds = 0;
	float playerFrameRate = 0;
	bool playerIsPlaying = false;
	int playerLastUpdate = 0;

    bool isRunning = false;
    
    bool isDevicesRefreshRequested = false;
    
    std::map<int, std::vector<float> > client_offset_ypr;
    std::vector<M1OrientationClientConnection> monitors; // track all the monitor instances
    float master_yaw = 0; float master_pitch = 0; float master_roll = 0;
    int master_mode = 0;

    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;

    void oscMessageReceived(const juce::OSCMessage& message) override;

    std::map<M1OrientationDeviceType, HardwareAbstract*> hardwareImpl;
    M1OrientationDeviceInfo currentDevice;
    Orientation orientation;

public:
    
    virtual ~M1OrientationManager();

    bool init(int serverPort, int watcherPort, bool useWatcher);
    void addHardwareImplementation(M1OrientationDeviceType type, HardwareAbstract* impl);

	void startSearchingForDevices();

    void update();

    Orientation getOrientation();
    std::vector<M1OrientationDeviceInfo> getDevices();
    M1OrientationDeviceInfo getConnectedDevice();

    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();

    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_updateOscDevice(int new_port, std::string new_msg_address_pattern);
    void command_recenter();
    void command_disconnect();
    void command_refresh();
   
    // Tracking for any plugin that does not need an m1_orientation_client but still needs feedback of orientation for UI purposes such as the M1-Panner plugin
    std::vector<M1RegisteredPlugin> registeredPlugins;
    bool bTimerActive = false;
    
    void timerCallback() override {
        if (registeredPlugins.size() > 0) {
            for (auto &i: registeredPlugins) {
                juce::OSCMessage m = juce::OSCMessage(juce::OSCAddressPattern("/monitor-settings"));
                m.addInt32(master_mode);
                m.addFloat32(master_yaw); // expected normalised
                m.addFloat32(master_pitch); // expected normalised
                m.addFloat32(master_roll); // expected normalised
                //m.addInt32(monitor_output_mode); // TODO: add the output configuration to sync plugins when requested
                i.messageSender->send(m);
            }
        }
        
        // TODO: check if any registered plugins closed
        //for (auto &i: registeredPlugins) {
        //}
    }
    
    void close();
};
