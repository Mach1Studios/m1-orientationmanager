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
        send_getDevicesNames(clients);
        send_getCurrentDevice(clients);
        send_getTrackingYawEnabled(clients);
        send_getTrackingPitchEnabled(clients);
        send_getTrackingRollEnabled(clients);
    }
    else if (message.getAddressPattern() == "/refreshDevices") {
        command_refreshDevices();
    }
    else if (message.getAddressPattern() == "/startTrackingUsingDevice") {
        M1OrientationDevice device = { message[0].getString().toStdString(), (M1OrientationDeviceType)message[1].getInt32() };
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
    else {
        std::cout << "not implemented!" << std::endl;
    }
}

void M1OrientationOSCServer::send(const std::vector<M1OrientationClientConnection>& clients, std::string str) {
    juce::OSCMessage msg(str.c_str());
    send(clients, msg);
}

void M1OrientationOSCServer::send(const std::vector<M1OrientationClientConnection>& clients, juce::OSCMessage& msg) {
    for (auto& client : clients) {
        juce::OSCSender sender;
        if (sender.connect("127.0.0.1", client.port)) {
            sender.send(msg);
        }
    }
}

void M1OrientationOSCServer::send_getDevicesNames(const std::vector<M1OrientationClientConnection>& clients) {
    std::vector<M1OrientationDevice> devices = getDevicesNames();

    juce::OSCMessage msg("/getDevices");
    for (auto& device : devices) {
        msg.addString(device.name);
        msg.addInt32(device.type);
    }
    send(clients, msg);
}

void M1OrientationOSCServer::send_getCurrentDevice(const std::vector<M1OrientationClientConnection>& clients) {
    M1OrientationDevice currentDevice = getCurrentDevice();
    juce::OSCMessage msg("/getCurrentDevice");
    msg.addString(currentDevice.name);
    msg.addInt32(currentDevice.type);
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

std::vector<M1OrientationDevice> M1OrientationOSCServer::getDevicesNames() {
    std::vector<M1OrientationDevice> devices;
    for (const auto& v : hardwareImpl) {
        M1OrientationDeviceType type = v.first;
        std::vector<std::string> devs = hardwareImpl[type]->getDevicesNames();
        for (auto& dev : devs) {
            devices.push_back({ dev , v.first });
        }
    }
    return devices;
}

M1OrientationDevice M1OrientationOSCServer::getCurrentDevice() {
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

bool M1OrientationOSCServer::init(int serverPort) {
    // check the port
    juce::DatagramSocket socket(false);
    socket.setEnablePortReuse(false);
    if (socket.bindToPort(serverPort)) {
        socket.shutdown();

        receiver.connect(serverPort);
        receiver.addListener(this);

        this->serverPort = serverPort;

        return true;
    }
    return false;
}

void M1OrientationOSCServer::update() {
    if (currentDevice.type != M1OrientationManagerDeviceTypeNone) {
        hardwareImpl[currentDevice.type]->update();

        M1OrientationYPR ypr = hardwareImpl[currentDevice.type]->getOrientation().currentOrientation.getYPR(); // todo
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
    receiver.removeListener(this);
    receiver.disconnect();
}

void M1OrientationOSCServer::command_refreshDevices() {
    // call the other thread?
    for (const auto& v : hardwareImpl) {
        v.second->refreshDevices();
    }

    send_getDevicesNames(clients);
}

void M1OrientationOSCServer::command_startTrackingUsingDevice(M1OrientationDevice device) {
    currentDevice = device;
    
    hardwareImpl[device.type]->startTrackingUsingDevice(device.name, [&](bool success, std::string errorMessage) {
        if (success) {
            send_getCurrentDevice(clients);
        }

        juce::OSCMessage msg("/getStatus");
        msg.addInt32(success);
        msg.addString(errorMessage);
        send(clients, msg);
    });
   
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
