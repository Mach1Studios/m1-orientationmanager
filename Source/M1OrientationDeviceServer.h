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

/// Class for orientation and orientation utilities
/// Designed to aggregate all orientation handling to a single collection point
struct GlobalOrientation {
    struct YPR {
        float yaw, pitch, roll = 0.0f;
        float yaw_min, pitch_min, roll_min = -180.0f;
        float yaw_max, pitch_max, roll_max = 180.0f;
        float custom_output_yaw, custom_output_pitch, custom_output_roll = 0.0f;
    };
    
    struct Quaternion {
        double qw, qx, qy, qz = 0;
    };

    YPR ypr;
    Quaternion quat;
    
    YPR getYPR() {
        double qW, qX, qY, qZ;
        float y, p, r;
        const juce::Array<float> quats = { quat.qw, quat.qx, quat.qy, quat.qz };

        qW = quats[0];
        qX = quats[2];
        qY = quats[1];
        qZ = quats[3];

        double test = qX * qZ + qY * qW;
        if (test > 0.499999) {
            // singularity at north pole
            ypr.yaw = 2 * atan2(qX, qW);
            ypr.pitch = juce::MathConstants<double>::pi / 2;
            ypr.roll = 0;
            ypr.custom_output_yaw = (float)juce::jmap(ypr.yaw, (float)-180, (float)180, ypr.yaw_min, ypr.yaw_max);
            ypr.custom_output_pitch = (float)juce::jmap(ypr.pitch, (float)-180, (float)180, ypr.pitch_min, ypr.pitch_max);
            ypr.custom_output_roll = (float)juce::jmap(ypr.roll, (float)-180, (float)180, ypr.roll_min, ypr.roll_max);
            return ypr;
        }
        if (test < -0.499999) {
            // singularity at south pole
            ypr.yaw = -2 * atan2(qX, qW);
            ypr.pitch = -juce::MathConstants<double>::pi / 2;
            ypr.roll = 0;
            ypr.custom_output_yaw = (float)juce::jmap(ypr.yaw, (float)-180, (float)180, ypr.yaw_min, ypr.yaw_max);
            ypr.custom_output_pitch = (float)juce::jmap(ypr.pitch, (float)-180, (float)180, ypr.pitch_min, ypr.pitch_max);
            ypr.custom_output_roll = (float)juce::jmap(ypr.roll, (float)-180, (float)180, ypr.roll_min, ypr.roll_max);
            return ypr;
        }
        double sqx = qX * qX;
        double sqy = qZ * qZ;
        double sqz = qY * qY;

        y = atan2(2 * qZ*qW - 2 * qX*qY, 1 - 2 * sqy - 2 * sqz);
        p = asin(2 * test);
        r = atan2(2 * qX*qW - 2 * qZ*qY, 1 - 2 * sqx - 2 * sqz);

        y *= -1.0f;

        ypr.yaw = juce::radiansToDegrees(y);
        ypr.pitch = juce::radiansToDegrees(p);
        ypr.roll = juce::radiansToDegrees(r);
        
        // remap output ypr
        ypr.custom_output_yaw = (float)juce::jmap(ypr.yaw, (float)-180, (float)180, ypr.yaw_min, ypr.yaw_max);
        ypr.custom_output_pitch = (float)juce::jmap(ypr.pitch, (float)-180, (float)180, ypr.pitch_min, ypr.pitch_max);
        ypr.custom_output_roll = (float)juce::jmap(ypr.roll, (float)-180, (float)180, ypr.roll_min, ypr.roll_max);

        return ypr;
    };
    
    Quaternion getQuaternion() {
        /// It is better to avoid this function and stick to updating quat and calculating best YPR
        quat.qx = sin(ypr.roll/2) * cos(ypr.pitch/2) * cos(ypr.yaw/2) - cos(ypr.roll/2) * sin(ypr.pitch/2) * sin(ypr.yaw/2);
        quat.qy = cos(ypr.roll/2) * sin(ypr.pitch/2) * cos(ypr.yaw/2) + sin(ypr.roll/2) * cos(ypr.pitch/2) * sin(ypr.yaw/2);
        quat.qz = cos(ypr.roll/2) * cos(ypr.pitch/2) * sin(ypr.yaw/2) - sin(ypr.roll/2) * sin(ypr.pitch/2) * cos(ypr.yaw/2);
        quat.qw = cos(ypr.roll/2) * cos(ypr.pitch/2) * cos(ypr.yaw/2) + sin(ypr.roll/2) * sin(ypr.pitch/2) * sin(ypr.yaw/2);
        return quat;
    };
    
    void resetOrientation() {
        // TODO: setup reset logic here
        // Ideally use last quat values
    };
};

/// Class for collecting devices
class SerialDeviceInfo {
public:
    std::string devicePath;
    std::string deviceName;
    // index for device, connection state for device
    int deviceID, deviceState;
    
    SerialDeviceInfo(std::string devicePathIn, std::string deviceNameIn, int deviceIDIn, int deviceStateIn){
        devicePath = devicePathIn;
        deviceName = deviceNameIn;
        deviceID = deviceIDIn;
        deviceState = deviceStateIn;
    }
    SerialDeviceInfo(){
        deviceName = "device undefined";
        deviceID = -1;
        deviceState = -1;
    }
    std::string getDevicePath(){
        return devicePath;
    }
    std::string getDeviceName(){
        return deviceName;
    }
    int getDeviceID(){
        return deviceID;
    }
    int getDeviceState(){
        return deviceState;
    }
};

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
