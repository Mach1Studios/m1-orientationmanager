/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <juce_events/juce_events.h>

class M1OrientationManagerOSCServer : public M1OrientationManagerOSCServerBase {
    bool bTrackingYaw = true;
    bool bTrackingPitch = true;
    bool bTrackingRoll = true;
    bool bTracking = true;

    std::string currentDevice;
    std::vector<std::string> devices;

public:

    void update() override {

    }

    void setTrackingYaw(bool enable) override {
        bTrackingYaw = enable;
    }

    void setTrackingPitch(bool enable) override {
        bTrackingPitch = enable;
    }

    void setTrackingRoll(bool enable)  override {
        bTrackingRoll = enable;
    }

    bool getTrackingYaw() override {
        return bTrackingYaw;
    }

    bool getTrackingPitch() override {
        return bTrackingPitch;
    }

    bool getTrackingRoll() override {
        return bTrackingRoll;
    }

    void refreshDevices() override {
        // add/remove to list of discovered devices of all connection types
        devices = { "device1", "device2" };
    }

    std::vector<std::string> getDevices() override {
        return devices;
    }
    
    void selectDevice(std::string device) override {
        currentDevice = device;
    }

    std::string getCurrentDevice() {
        return currentDevice;
    }

    void setTracking(bool enable)  override {
        bTracking = enable;
    }

    bool getTracking() override {
        return bTracking;
    }
};
  
