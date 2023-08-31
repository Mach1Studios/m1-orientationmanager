//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "M1OrientationOSCServer.h"

void M1OrientationOSCServer::oscMessageReceived(const juce::OSCMessage& message) {
    if (message.getAddressPattern() == "/addClient") {
        // add client to clients list
        int port = message[0].getInt32();
        bool found = false;

        for (auto& client : clients) {
            if (client.port == port) {
                client.time = juce::Time::currentTimeMillis();
                found = true;
            }
        }

        if (!found) {
            M1OrientationClientConnection client;
            client.port = port;
            client.time = juce::Time::currentTimeMillis();
            clients.push_back(client);
        }

        juce::OSCSender sender;
        if (sender.connect("127.0.0.1", port)) {
            juce::OSCMessage msg("/connectedToServer");
            sender.send(msg);
        }

        std::vector<M1OrientationClientConnection> clients = { M1OrientationClientConnection { port, 0 } };
        send_getDevices(clients);
        send_getCurrentDevice(clients);
        send_getTrackingYawEnabled(clients);
        send_getTrackingPitchEnabled(clients);
        send_getTrackingRollEnabled(clients);
    }
    else if (message.getAddressPattern() == "/refreshDevices") {
        command_refreshDevices();
    }
    else if (message.getAddressPattern() == "/startTrackingUsingDevice") {
        M1OrientationDeviceInfo device = { message[0].getString().toStdString(), (M1OrientationDeviceType)message[1].getInt32(), message[2].getString().toStdString() };
        command_startTrackingUsingDevice(device);
    }
    else if (message.getAddressPattern() == "/setTrackingYawEnabled") {
        bool enable = message[0].getInt32();
        command_setTrackingYawEnabled(enable);
    }
    else if (message.getAddressPattern() == "/setTrackingPitchEnabled") {
        bool enable = message[0].getInt32();
        command_setTrackingPitchEnabled(enable);
    }
    else if (message.getAddressPattern() == "/setTrackingRollEnabled") {
        bool enable = message[0].getInt32();
        command_setTrackingRollEnabled(enable);
    }
    else if (message.getAddressPattern() == "/recenter") {
        command_recenter();
    }
    else if (message.getAddressPattern() == "/disconnect") {
        command_disconnect();
    }
    else if (message.getAddressPattern() == "/removeClient") {
        int search_port = message[0].getInt32();
        for (int index = 0; index < clients.size(); index++) {
            if (clients[index].port = message[0].getInt32()) {
                clients.erase(clients.begin() + index);
            }
        }
    }
    else if (message.getAddressPattern() == "/setMonitorYPR") {
        // receiving updated monitor YPR and mode
        // Note: It is expected that the orientation manager receives orientation and sends it to a client and for the client to offset this orientation before sending it back to registered plugins, the adding of all orientations should happen on client side only
        monitor_mode = message[0].getInt32();
        monitor_yaw = message[1].getFloat32();
        monitor_pitch = message[2].getFloat32();
        monitor_roll = message[3].getFloat32();
        DBG("[Monitor] Mode: "+std::to_string(monitor_mode)+", YPR="+std::to_string(monitor_yaw)+", "+std::to_string(monitor_pitch)+", "+std::to_string(monitor_roll));
        // this is used to update the orientation on the manager before passing back to any registered plugins via the "/monitor-settings" address
    }
    else if (message.getAddressPattern() == "/m1-register-plugin") {
        // registering new panner instance
        auto port = message[0].getInt32();
        // protect port creation to only messages from registered plugin (example: an m1-panner)
        if (std::find_if(registeredPlugins.begin(), registeredPlugins.end(), find_plugin(port)) == registeredPlugins.end()) {
            M1RegisteredPlugin foundPlugin;
            foundPlugin.port = port;
            foundPlugin.messageSender = new juce::OSCSender();
            foundPlugin.messageSender->connect("127.0.0.1", port); // connect to that newly discovered panner locally
            registeredPlugins.push_back(foundPlugin);
            DBG("Plugin registered: " + std::to_string(port));
        } else {
            DBG("Plugin port already registered: " + std::to_string(port));
        }
        if (!bTimerActive && registeredPlugins.size() > 0) {
            startTimer(60);
            bTimerActive = true;
        } else {
            if (registeredPlugins.size() == 0) {
                // TODO: setup logic for deleting from `registeredPlugins`
                stopTimer();
                bTimerActive = false;
            }
        }
    }
    else if (message.getAddressPattern() == "/panner-settings") {
        if (message.size() > 0) { // check message size
            auto plugin_port = message[0].getInt32();
            if (message.size() == 6) {
                auto input_mode = message[1].getInt32();
                auto azi = message[2].getFloat32();
                auto ele = message[3].getFloat32();
                auto div = message[4].getFloat32();
                auto gain = message[5].getFloat32();
                DBG("[OSC] Panner: port="+std::to_string(plugin_port)+", in="+std::to_string(input_mode)+", az="+std::to_string(azi)+", el="+std::to_string(ele)+", di="+std::to_string(div)+", gain="+std::to_string(gain));
                // Check if port matches expected registered-plugin port
                if (registeredPlugins.size() > 0) {
                    auto it = std::find_if(registeredPlugins.begin(), registeredPlugins.end(), find_plugin(plugin_port));
                    auto index = it - registeredPlugins.begin(); // find the index from the found plugin
                    registeredPlugins[index].isPannerPlugin = true;
                    registeredPlugins[index].input_mode = input_mode;
                    registeredPlugins[index].azimuth = azi;
                    registeredPlugins[index].elevation = ele;
                    registeredPlugins[index].diverge = div;
                    registeredPlugins[index].gain = gain;
                }
            }
        } else {
            // port not found, error here
        }
    }
    else {
        DBG("OSC Message not implemented: " + message.getAddressPattern().toString());
    }
}

bool M1OrientationOSCServer::send(const std::vector<M1OrientationClientConnection>& clients, std::string str) {
    juce::OSCMessage msg(str.c_str());
    return send(clients, msg);
}

bool M1OrientationOSCServer::send(const std::vector<M1OrientationClientConnection>& clients, juce::OSCMessage& msg) {
    for (auto& client : clients) {
        juce::OSCSender sender;
        if (sender.connect("127.0.0.1", client.port)) {
            if (sender.send(msg)) {
                return true;
                // TODO: we need some feedback because we can send messages without error even when there is no client receiving
            } else {
                // TODO: ERROR: Issue with sending OSC message on server side
                return false;
            }
        } else {
            // TODO: ERROR: Issue with sending OSC message on server side
            return false;
        }
    }
}

void M1OrientationOSCServer::send_getDevices(const std::vector<M1OrientationClientConnection>& clients) {
    std::vector<M1OrientationDeviceInfo> devices = getDevices();

    juce::OSCMessage msg("/getDevices");
    for (auto& device : devices) {
        msg.addString(device.getDeviceName());
        msg.addInt32(device.getDeviceType());
        msg.addString(device.getDeviceAddress());
        bool hasStrength = std::holds_alternative<int>(device.getDeviceSignalStrength());
        msg.addInt32(hasStrength ? 1 : 0);
        if (hasStrength) msg.addInt32(std::get<int>(device.getDeviceSignalStrength()));
            else msg.addInt32(0);
    }
    send(clients, msg);
}

void M1OrientationOSCServer::send_getCurrentDevice(const std::vector<M1OrientationClientConnection>& clients) {
    M1OrientationDeviceInfo device = getConnectedDevice();
    juce::OSCMessage msg("/getCurrentDevice");
    msg.addString(device.getDeviceName());
    msg.addInt32(device.getDeviceType());
    msg.addString(device.getDeviceAddress());
    auto signalStrength = device.getDeviceSignalStrength();

    bool hasStrength = std::holds_alternative<int>(signalStrength);
    msg.addInt32(hasStrength ? 1 : 0);
    if (hasStrength) {
        msg.addInt32(std::get<int>(signalStrength));
    }
    else {
        msg.addInt32(0);
    }
    send(clients, msg);
}

void M1OrientationOSCServer::send_getTrackingYawEnabled(const std::vector<M1OrientationClientConnection>& clients) {
    bool enable = getTrackingYawEnabled();
    juce::OSCMessage msg("/getTrackingYawEnabled");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationOSCServer::send_getTrackingPitchEnabled(const std::vector<M1OrientationClientConnection>& clients) {
    bool enable = getTrackingPitchEnabled();
    juce::OSCMessage msg("/getTrackingPitchEnabled");
    msg.addInt32(enable);
    send(clients, msg);
}

void M1OrientationOSCServer::send_getTrackingRollEnabled(const std::vector<M1OrientationClientConnection>& clients) {
    bool enable = getTrackingRollEnabled();
    juce::OSCMessage msg("/getTrackingRollEnabled");
    msg.addInt32(enable);
    send(clients, msg);
}

M1OrientationOSCServer::~M1OrientationOSCServer() {
    close();
}
 
std::vector<M1OrientationClientConnection> M1OrientationOSCServer::getClients() {
    return clients;
}

std::vector<M1OrientationDeviceInfo> M1OrientationOSCServer::getDevices() {
    std::vector<M1OrientationDeviceInfo> devices;
    for (const auto& hardware : hardwareImpl) {
        auto devicesToAdd = hardware.second->getDevices();
        devices.insert(devices.end(), devicesToAdd.begin(), devicesToAdd.end());
    }
    return devices;
}

M1OrientationDeviceInfo M1OrientationOSCServer::getConnectedDevice() {
    return currentDevice;
}

bool M1OrientationOSCServer::getTrackingYawEnabled() {
    return bTrackingYawEnabled;
}

bool M1OrientationOSCServer::getTrackingPitchEnabled() {
    return bTrackingPitchEnabled;
}

bool M1OrientationOSCServer::getTrackingRollEnabled() {
    return bTrackingRollEnabled;
}

bool M1OrientationOSCServer::init(int serverPort, int watcherPort, bool useWatcher = false) {
    // check the port
    juce::DatagramSocket socket(false);
    socket.setEnablePortReuse(false);
    if (socket.bindToPort(serverPort)) {
        socket.shutdown();

        receiver.connect(serverPort);
        receiver.addListener(this);

        this->serverPort = serverPort;
        this->watcherPort = watcherPort;
        this->isRunning = true;

        if (useWatcher) {
            std::thread([&]() {
                while (this->isRunning) {
                    juce::OSCSender sender;
                    if (sender.connect("127.0.0.1", this->watcherPort)) {
                        juce::OSCMessage msg("/Mach1/ActiveClients");
                        if (getClients().size() > 0) {
                            msg.addInt32(getClients().size()); // report number of active clients, if number drops to 0 then watcher will shutdown
                        }
                        sender.send(msg);
                        sender.disconnect();
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }).detach();
        }
        return true;
    }
    else {
        return false;
    }
}

void M1OrientationOSCServer::update() {

    if (currentDevice.getDeviceType() != M1OrientationManagerDeviceTypeNone) {
        if (!hardwareImpl[currentDevice.getDeviceType()]->update()) {
            /// ERROR STATE
            // TODO: Check for connection to client, if not then reconnect
            // TODO: if reconnect does not work then error that client is no longer available
            // if (client still exists){
                // TODO: Check if connected, if not then reconnect
                command_startTrackingUsingDevice(currentDevice);
            //}
        }

        M1OrientationYPR ypr = hardwareImpl[currentDevice.getDeviceType()]->getOrientation().currentOrientation.getYPRinDegrees();
        if (!getTrackingYawEnabled()) ypr.yaw = 0;
        if (!getTrackingPitchEnabled()) ypr.pitch = 0;
        if (!getTrackingRollEnabled()) ypr.roll = 0;
        orientation.setYPR(ypr);

        juce::OSCMessage msg("/getOrientation");
        msg.addFloat32(ypr.yaw);
        msg.addFloat32(ypr.pitch);
        msg.addFloat32(ypr.roll);
        send(clients, msg);
    }
}

Orientation M1OrientationOSCServer::getOrientation() {
    return orientation;
}

void M1OrientationOSCServer::addHardwareImplementation(M1OrientationDeviceType type, HardwareAbstract* impl) {
    hardwareImpl[type] = impl;
}

void M1OrientationOSCServer::close() {
    isRunning = false;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver.removeListener(this);
    receiver.disconnect();
}

void M1OrientationOSCServer::command_refreshDevices() {
    // call the other thread?
    for (const auto& v : hardwareImpl) {
        v.second->refreshDevices();
    }
    send_getDevices(clients);
}

void M1OrientationOSCServer::command_disconnect() {
    orientation.resetOrientation();
    if (currentDevice.getDeviceType() != M1OrientationManagerDeviceTypeNone) {
        hardwareImpl[currentDevice.getDeviceType()]->close();
        currentDevice = M1OrientationDeviceInfo();
        send_getCurrentDevice(clients);
    }
}

void M1OrientationOSCServer::command_startTrackingUsingDevice(M1OrientationDeviceInfo device) {
    orientation.resetOrientation();
    if (currentDevice != device){
        hardwareImpl[device.getDeviceType()]->startTrackingUsingDevice(device, [&](bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress) {
            if (success) {
                send_getCurrentDevice(clients);
                currentDevice = device;
            }
            
            juce::OSCMessage msg("/getStatus");
            msg.addInt32(success);
            msg.addString(message);
            msg.addString(connectedDeviceName);
            msg.addInt32(connectedDeviceType);
            msg.addString(connectedDeviceAddress);
            send(clients, msg);
        });
    } else {
        // already connected to this device
    }
}

void M1OrientationOSCServer::command_setTrackingYawEnabled(bool enable) {
    bTrackingYawEnabled = enable;
    send_getTrackingYawEnabled(clients);
}

void M1OrientationOSCServer::command_setTrackingPitchEnabled(bool enable) {
    bTrackingPitchEnabled = enable;
    send_getTrackingPitchEnabled(clients);
}

void M1OrientationOSCServer::command_setTrackingRollEnabled(bool enable) {
    bTrackingRollEnabled = enable;
    send_getTrackingRollEnabled(clients);
}

void M1OrientationOSCServer::command_recenter() {
    orientation.resetOrientation();
}
