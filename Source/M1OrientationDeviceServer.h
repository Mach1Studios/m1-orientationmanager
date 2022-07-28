/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <juce_events/juce_events.h>
#include "rs232/rs232.h"

#define INPUT_OSC_LISTENER_PORT 9902
#define INPUT_OSC_MONITOR_TRANSPORT_PORT 9001
#define INPUT_OSC_M1MNTRCTRL_PORT 9901
#define OUTPUT_OSC_MONITOR_PORT 9900
#define OUTPUT_OSC_MONITOR_TRANSPORT_PORT 9003
#define OUTPUT_OSC_M1HPCTRL_PORT 9004

class M1OrientationManagerServer :  public M1OrientationManagerOSCServerBase,
                                    private juce::Timer,
                                    private juce::OSCReceiver
{
public:
    M1OrientationManagerServer();
    ~M1OrientationManagerServer();
    
    // Setup for input OSC listener
    bool bBlockIncomingOSCMessages = false;
    bool connectOscReceiver();
    void disconnectOscReceiver();
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void oscBundleReceived(const juce::OSCBundle& bundle) override;
    
    // Setup for input serial listener
    std::vector<SerialDeviceInfo> getSerialDevices();
    bool connectSerial(SerialDeviceInfo device);
    void disconnectSerial();
    bool isSerialConnected();
    void timerCallback() override;

    SerialDeviceInfo currentDevice;
    std::vector<SerialDeviceInfo> devices;

    int serialBaudRate = 115200;
    int serialPortNumber;
    bool serialDeviceConnected = false;
    std::vector<SerialDeviceInfo> serialList;
    int number_of_serial_devices;

    // Setup global orientation
    GlobalOrientation currentOrientation;
    bool bTracking = true;
    bool bTrackingYaw = true;
    bool bTrackingPitch = true;
    bool bTrackingRoll = true;
    
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
        
        // Get Serial Device List
        devices.insert(std::end(devices), std::begin(getSerialDevices()), std::end(getSerialDevices()));
        
        // Get BLE Device List
        
        // TODO: Get Camera Device List
    }
    
    std::vector<std::string> getDevices() override {
        refreshDevices();
        std::vector<std::string> templist;
        
        for (auto &device_index : devices){
            templist.push_back(device_index.deviceName);
        }
        return templist;
    }
    
    // TODO: block this if no device connected
    std::string getCurrentDevice() override {
        //TODO: fix this so it uses unique IDs?
        for (auto &device_index : devices){
            if (device_index.deviceState == 1){
                currentDevice = device_index;
                return currentDevice.deviceName;
            }
        }
        return "";
    }
    
    void selectDevice(std::string device) override {
        //TODO: fix this so it uses unique IDs?
        for (auto &device_index : devices){
            if (device_index.deviceName == device){
                currentDevice = device_index;
            }
        }
    }
    
    void selectDevice(int device) {
        currentDevice = devices[device];
    }

    void setTracking(bool enable)  override {
        bTracking = enable;
    }

    bool getTracking() override {
        return bTracking;
    }
    
    // OSC OUTPUTS
    juce::OSCSender output_to_monitor;
    juce::OSCSender output_to_m1hpctrl;
    
    // Setup for output OSC component
    // Description: this copies the current orientation to output to other peripherals if needed
    bool outputOSCActive = false;
    juce::OSCSender output_to_custom_osc;
    juce::String outputOSCIPAddress;
    int outputOSCPort = 9999;
    void setupOutputOSCIP(juce::String address, int port);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(M1OrientationManagerServer)
};
