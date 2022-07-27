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

struct GlobalOrientation
{
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
        double t0, t1, t2, t3, t4;
        t0 = 2.0 * (quat.qw * quat.qx + quat.qy * quat.qz);
        t1 = 1.0 - 2.0 * (quat.qx * quat.qx + quat.qy * quat.qy);
        ypr.roll = atan2(t0, t1);
        t2 = 2.0 * (quat.qw * quat.qy - quat.qz * quat.qx);
        if (t2 > 1){
            t2 = 1.0;
        } else if (t2 < -1) {
            t2 = -1.0;
        } else {
            t2 = t2;
        }
        ypr.pitch = asin(t2);
        t3 = 2.0 * (quat.qw * quat.qz + quat.qx * quat.qy);
        t4 = 1.0 - 2.0 * (quat.qy * quat.qy + quat.qz * quat.qz);
        ypr.yaw = atan2(t3, t4);
        
        // remap output ypr
        ypr.custom_output_yaw = (float)juce::jmap(ypr.yaw, (float)-180, (float)180, ypr.yaw_min, ypr.yaw_max);
        ypr.custom_output_pitch = (float)juce::jmap(ypr.pitch, (float)-180, (float)180, ypr.pitch_min, ypr.pitch_max);
        ypr.custom_output_roll = (float)juce::jmap(ypr.roll, (float)-180, (float)180, ypr.roll_min, ypr.roll_max);

        return ypr;
    };

    Quaternion getQuaternion() {
        // best to avoid this and stick to updating quat and calculating best YPR
        quat.qx = sin(ypr.roll/2) * cos(ypr.pitch/2) * cos(ypr.yaw/2) - cos(ypr.roll/2) * sin(ypr.pitch/2) * sin(ypr.yaw/2);
        quat.qy = cos(ypr.roll/2) * sin(ypr.pitch/2) * cos(ypr.yaw/2) + sin(ypr.roll/2) * cos(ypr.pitch/2) * sin(ypr.yaw/2);
        quat.qz = cos(ypr.roll/2) * cos(ypr.pitch/2) * sin(ypr.yaw/2) - sin(ypr.roll/2) * sin(ypr.pitch/2) * cos(ypr.yaw/2);
        quat.qw = cos(ypr.roll/2) * cos(ypr.pitch/2) * cos(ypr.yaw/2) + sin(ypr.roll/2) * sin(ypr.pitch/2) * sin(ypr.yaw/2);
        return quat;
    };
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
    juce::StringArray getPortInfo();
    bool connectSerial();
    void disconnectSerial();
    bool isSerialConnected();
    void timerCallback() override;
//    void setGlobalOrientation();
//    void getGlobalOrientation();
//    void resetGlobalOrientation();

    std::string currentDevice;
    std::vector<std::string> devices;

    int serialBaudRate = 115200;
    int serialPortNumber;
    bool serialDeviceConnected = false;
    juce::StringPairArray portlist;
    int port_number, port_index, port_state;

    // Setup global orientation
    GlobalOrientation currentOrientation;
    bool bTracking = true;
    bool bTrackingYaw = true;
    bool bTrackingPitch = true;
    bool bTrackingRoll = true;
    
    void update() override;

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

    std::string getCurrentDevice() override {
        return currentDevice;
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(M1OrientationManagerServer)
};
