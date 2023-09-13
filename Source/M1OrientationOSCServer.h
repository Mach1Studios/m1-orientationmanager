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
    public M1OrientationManagerOSCSettings,
    public juce::MultiTimer
{
    juce::OSCReceiver receiver; 

    std::vector<M1OrientationClientConnection> clients;
    int serverPort = 0;
    bool isRunning = false;
    juce::int64 shutdownCounterTime = 0;
    
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
    bool init(int serverPort);
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
    void command_updateOscDevice(int new_port, std::string new_msg_address_pattern);
    void command_recenter();
    void command_disconnect();
    
    // Tracking for any plugin that does not need an m1_orientation_client but still needs feedback of orientation for UI purposes such as the M1-Panner plugin
    std::vector<M1RegisteredPlugin> registeredPlugins;
    bool bTimerActive = false;
    
    void timerCallback(int timerID) override {
        if (timerID == 0) {
            if (registeredPlugins.size() > 0) {
                for (auto &i: registeredPlugins) {
                    juce::OSCMessage m = juce::OSCMessage(juce::OSCAddressPattern("/monitor-settings"));
                    m.addInt32(monitor_mode);
                    m.addFloat32(monitor_yaw); // expected normalised
                    m.addFloat32(monitor_pitch); // expected normalised
                    m.addFloat32(monitor_roll); // expected normalised
                    //m.addInt32(monitor_output_mode); // TODO: add the output configuration to sync plugins when requested
                    i.messageSender->send(m);
                }
            }
            
            // TODO: check if any registered plugins closed
            //for (auto &i: registeredPlugins) {
            //}
        }
        
        // TIMER 1 = m1-orientationmanager ping timer
        // this is used to ping the clients to check if the m1-orientationmanager has crashed
        // TODO: use service to analyze the std--out//std--err on the daemon and remove this watchdog
        if (timerID == 1) {
            juce::int64 currentTime = juce::Time::currentTimeMillis();

            if (getClients().size() > 0) {
                for (int i = 0; i < getClients().size(); i++) {
                    juce::OSCSender sender;
                    if (sender.connect("127.0.0.1", clients[i].port)) {
                        DBG("[Watchdog] Pinging port "+std::to_string(clients[i].port));
                        juce::OSCMessage m = juce::OSCMessage(juce::OSCAddressPattern("/Mach1/ActiveClients"));
                        if (getClients().size() > 0) {
                            m.addInt32(getClients().size()); // report number of active clients, if number drops to 0 then commence a shutdown timer
                        } else {
                            m.addInt32(0);
                        }
                        sender.send(m);
                        sender.disconnect();
                    }
                }
            }
        }
        
        /*
        // TIMER 2 = Shutdown timer
        // this is used when there are no m1_orientation_client's found
        //to start a countdown to shutdown this service
        if (timerID == 2) {
            juce::int64 currentTime = juce::Time::currentTimeMillis();
            DBG("TIMER[2]: " + std::to_string(currentTime - shutdownCounterTime));
            if (currentTime > shutdownCounterTime && currentTime - shutdownCounterTime > 120000) {
                JUCEApplicationBase::quit(); // exit successfully to prompt service managers to not restart
            }
        }
         */
    }
    
    void close();
};
