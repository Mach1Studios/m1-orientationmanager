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
#include "httplib/httplib.h"

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
    public M1OrientationManagerOSCSettings
{
	httplib::Server server;
	std::mutex mutex;
	std::string stringForClient;

	juce::OSCReceiver receiver;

    std::vector<M1OrientationClientConnection> clients;
    int serverPort = 0;

	float playerPositionInSeconds = 0;
	float playerFrameRate = 0;
	bool playerIsPlaying = false;
	int playerLastUpdate = 0;

    bool isRunning = false;
    
    bool isDevicesRefreshRequested = false;
    
    bool bTrackingYawEnabled = true;
    bool bTrackingPitchEnabled = true;
    bool bTrackingRollEnabled = true;

    std::map<M1OrientationDeviceType, HardwareAbstract*> hardwareImpl;
    M1OrientationDeviceInfo currentDevice;
    
    M1Orientation orientation;
    M1Orientation offset;

public:
    virtual ~M1OrientationManager();

    bool init(int serverPort, int helperPort) override;
    void addHardwareImplementation(M1OrientationDeviceType type, HardwareAbstract* impl);
	void startSearchingForDevices();
    void update();

    M1Orientation getOrientation();
    std::vector<M1OrientationDeviceInfo> getDevices();
    M1OrientationDeviceInfo getConnectedDevice();

    bool getTrackingYawEnabled();
    bool getTrackingPitchEnabled();
    bool getTrackingRollEnabled();

    void command_startTrackingUsingDevice(M1OrientationDeviceInfo device);
    void command_setTrackingYawEnabled(bool enable);
    void command_setTrackingPitchEnabled(bool enable);
    void command_setTrackingRollEnabled(bool enable);
    void command_updateDeviceSettings(std::string additional_settings);
    void command_recenter();
    void command_disconnect();
    void command_refresh();
    
    void close();
};
