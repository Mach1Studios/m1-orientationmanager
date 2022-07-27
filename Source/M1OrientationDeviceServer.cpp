/*
  ==============================================================================

    M1OrientationDeviceOSCServer.cpp
    Created: 27 Jul 2022 3:52:17pm
    Author:  Dylan Marcus

  ==============================================================================
*/

#include "M1OrientationDeviceServer.h"

M1OrientationManagerServer::M1OrientationManagerServer() {
    output_to_custom_osc.connect(outputOSCIPAddress, outputOSCPort);
}

M1OrientationManagerServer::~M1OrientationManagerServer() {
    if (isSerialConnected()) {
        disconnectSerial();
    }
    disconnectOscReceiver();
    output_to_custom_osc.disconnect();
}


bool M1OrientationManagerServer::connectOscReceiver() {
    bool isConnected = connect(INPUT_OSC_LISTENER_PORT);
//    addListener(this);
    return isConnected;
}

void M1OrientationManagerServer::disconnectOscReceiver() {
    disconnect();
}

void M1OrientationManagerServer::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern().toString() == "/orientation" && message.size() == 4) {
        currentOrientation.quat.qw = message[0].getFloat32();
        currentOrientation.quat.qx = message[1].getFloat32();
        currentOrientation.quat.qy = message[2].getFloat32();
        currentOrientation.quat.qz = message[3].getFloat32();
    } else if (message.getAddressPattern().toString() == "/orientation") { // capture any other sized messages
        currentOrientation.ypr.yaw = message[0].getFloat32();
        currentOrientation.ypr.pitch = message[1].getFloat32();
        currentOrientation.ypr.roll = message[2].getFloat32();
    }
    if (message.getAddressPattern().toString() == "/bosear/sensors/rotation_six_dof" || message.getAddressPattern().toString() == "/bosear/sensors/rotation_nine_dof") {
        // swap yaw and pitch for BoseAR
        float y = message[1].getFloat32();
        float p = message[0].getFloat32();
        float r = message[2].getFloat32();
        currentOrientation.ypr.yaw = juce::radiansToDegrees(y / 360.);
        currentOrientation.ypr.pitch = juce::radiansToDegrees((((((p > 180.) ? abs(p - 360.) : -p))) * -1. + 90. ) / 180.);
        currentOrientation.ypr.roll = juce::radiansToDegrees((((((r > 180.) ? abs(r - 360.) : -r))) * -1. + 90. ) / 180.);
    }
    if (message.getAddressPattern().toString() == "/accxyz") {
        float x = message[0].getFloat32();
        float y = message[1].getFloat32();
        float t = message[2].getFloat32();
        //transform from -1. -> 1
        currentOrientation.ypr.yaw = (((-1. - 1.) / (0. - 1.)) * ((x - 360.) + 360.));
        currentOrientation.ypr.pitch = (((-1. - 1.) / (-180. - 1.)) * ((y - 180.) + 180.));
        currentOrientation.ypr.roll = (((-1. - 1.) / (-180. - 1.)) * ((t - 180.) + 180.));
    }
    
    // TRANSPORT MESSAGES
    if (message.getAddressPattern().toString() == "/offset") {
//        ofLog() << "got offset message with " << m.getNumArgs() << " args";
//        ofLog() << "hh:" << m.getArgAsInt(0);
//        ofLog() << "mm:" << m.getArgAsInt(1);
//        ofLog() << "ss:" << m.getArgAsInt(2);
//        ofLog() << "fs:" << m.getArgAsInt(3);
//        videoPlayer.updateTimecodeOffset(m.getArgAsInt(0), m.getArgAsInt(1), m.getArgAsInt(2), m.getArgAsInt(3));
    }
    if (message.getAddressPattern().toString() == "/transport") {
//        videoPlayer.updateTransport(m);
    }

    if (message.getAddressPattern().toString() == "/pannertov") {
//            updateOverlayCoords(m.getArgAsFloat(0), m.getArgAsFloat(1), m.getArgAsFloat(2));
    }
}

void M1OrientationManagerServer::oscBundleReceived(const juce::OSCBundle& bundle) {
    juce::OSCBundle::Element elem = bundle[0];
    oscMessageReceived(elem.getMessage());
}

juce::StringArray M1OrientationManagerServer::getPortInfo() {
    port_number = comEnumerate();
    for(port_index=0; port_index < port_number; port_index++)
        portlist.set(comGetInternalName(port_index),comGetPortName(port_index));
    return portlist.getAllValues();
}

bool M1OrientationManagerServer::connectSerial() {
    port_state = comOpen(serialPortNumber, serialBaudRate);
    if (port_state == 1) {
        serialDeviceConnected = true;
        startTimer(10);
        return true;
    } else {
        return false;
    }
}

void M1OrientationManagerServer::disconnectSerial() {
    comClose(serialPortNumber);
    serialDeviceConnected = false;
    stopTimer();
}

bool M1OrientationManagerServer::isSerialConnected() {
    return serialDeviceConnected;
}

void M1OrientationManagerServer::timerCallback()
{
    if(isSerialConnected()) {
        char readBuffer[128];
        comRead(serialPortNumber, readBuffer, 128);
        
        if(strlen(readBuffer) != 0) {
            juce::StringArray dataReceived = juce::StringArray::fromTokens(readBuffer, ",", "\"");
            if(dataReceived.size() == 4) {
                currentOrientation.quat.qw = dataReceived[0].getFloatValue();
                currentOrientation.quat.qx = dataReceived[1].getFloatValue();
                currentOrientation.quat.qy = dataReceived[2].getFloatValue();
                currentOrientation.quat.qz = dataReceived[3].getFloatValue();
            } else {
                currentOrientation.ypr.yaw = dataReceived[0].getFloatValue();
                currentOrientation.ypr.pitch = dataReceived[1].getFloatValue();
                currentOrientation.ypr.roll = dataReceived[2].getFloatValue();
            }
        }
    }
}
