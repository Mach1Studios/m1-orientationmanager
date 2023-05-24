//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    juce::OpenGLAppComponent::setSize(800, 600);
    
    // For debug testing you can set this to false to list all connectable BLE devices
    hardwareBLE.displayOnlyKnownIMUs = true;
    hardwareBLE.setup();
    hardwareSerial.setup();
    
	hardwareOSC.setup();
	hardwareEmulator.setup();
}

MainComponent::~MainComponent()
{
	murka::JuceMurkaBaseComponent::shutdownOpenGL();
}

//==============================================================================
void MainComponent::initialise()
{
    murka::JuceMurkaBaseComponent::initialise();

	std::string resourcesPath;
	if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
		resourcesPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("Application Support/Mach1 Spatial System/resources").getFullPathName().toStdString();
	}
	else {
		resourcesPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("Mach1 Spatial System/resources").getFullPathName().toStdString();
	}
	printf("Resources Loaded From: %s \n", resourcesPath.c_str());
	m.setResourcesPath(resourcesPath);

	//std::string settingsFilePath = (juce::File::getCurrentWorkingDirectory().getFullPathName() + "/settings.json").toStdString();
	//m1OrientationOSCServer.initFromSettings(settingsFilePath);
	m1OrientationOSCServer.init(6345);

	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &hardwareBLE);
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeSerial, &hardwareSerial);
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeOSC, &hardwareOSC);
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeEmulator, &hardwareEmulator);
}

//==============================================================================
void MainComponent::render()
{
	m.setFont("ProximaNovaReg.ttf", 10);

	m.startFrame();
	m.setScreenScale((float)openGLContext.getRenderingScale());

	m.clear(20);
	m.setColor(255);

	m.begin();

	m1OrientationOSCServer.update();

	int offsetY = 0;
	int offsetX = 0;
	
	offsetX = 10;
	offsetY = 20;

	Orientation orientation = m1OrientationOSCServer.getOrientation();

	m.getCurrentFont()->drawString("orientation: ", offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("yaw: " + std::to_string(orientation.getYPR().yaw), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("pitch: " + std::to_string(orientation.getYPR().pitch), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("roll: " + std::to_string(orientation.getYPR().roll), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

	m.getCurrentFont()->drawString("tracking enabled: ", offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("yaw: " + std::to_string(m1OrientationOSCServer.getTrackingYawEnabled()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("pitch: " + std::to_string(m1OrientationOSCServer.getTrackingPitchEnabled()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("roll: " + std::to_string(m1OrientationOSCServer.getTrackingRollEnabled()), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

    M1OrientationDeviceInfo device = m1OrientationOSCServer.getConnectedDevice();
	m.getCurrentFont()->drawString("device: " + device.getDeviceName() + ":" + M1OrientationDeviceTypeName[device.getDeviceType()], offsetX, offsetY);
	offsetY += 40;

	m.getCurrentFont()->drawString("devices: ", offsetX, offsetY);
	offsetY += 40;
	std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
	for (auto& device : devices) {
		m.getCurrentFont()->drawString("> ["+M1OrientationDeviceTypeName[device.getDeviceType()]+"]: "+device.getDeviceName(), offsetX, offsetY);
		offsetY += 40;
	}

	offsetX = 220;
	offsetY = 20;
	
	auto clients = m1OrientationOSCServer.getClients();
	m.getCurrentFont()->drawString("clients: " + std::to_string(clients.size()), offsetX, offsetY);
	offsetY += 40;
	for (int i = 0; i < clients.size(); i++) {
		m.getCurrentFont()->drawString("> " + std::to_string(clients[i].port), offsetX, offsetY);
		offsetY += 30;
	}
	offsetY += 40;

	offsetX = 400;
	offsetY = 20;

	auto& refreshDeviceButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("refresh devices");
	refreshDeviceButton.draw();
	if (refreshDeviceButton.pressed) {
        // TODO: clear the previous list of devices?
        // should clear be handled
       
		m1OrientationOSCServer.command_refreshDevices();
      
	}
	offsetY += 50;

//	// Draw a murka::Button for each device detected from the m1OrientationOSCServer.getDevices()
//	std::vector<murka::Button> deviceSelectButton;
//	for (int i = 0; i < m1OrientationOSCServer.getDevices().size(); ++i) {
//        auto& newDeviceButton = m.prepare<murka::Button>({ offsetX, offsetY, 200, 20 }).text(m1OrientationOSCServer.getDevices()[i].getDeviceName());
//        newDeviceButton.draw();
//        deviceSelectButton.push_back(newDeviceButton);
//		if (deviceSelectButton[i].pressed) {
//			m1OrientationOSCServer.command_startTrackingUsingDevice(m1OrientationOSCServer.getDevices()[i]);
//		}
//		offsetY +=20;
//	}

	 auto& selectDevice1Button = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("select device 1");
	 selectDevice1Button.draw();
	 if (selectDevice1Button.pressed) {
	 	std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
	 	if (devices.size() > 0) {
			m1OrientationOSCServer.command_startTrackingUsingDevice(devices[0]);
	 	}
	 }
	 offsetY += 50;

	 auto& selectDevice2Button = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("select device 2");
	 selectDevice2Button.draw();
	 if (selectDevice2Button.pressed) {
	 	std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
	 	if (devices.size() > 1) {
            m1OrientationOSCServer.command_startTrackingUsingDevice(devices[1]);
	 	}
	 }
	 offsetY += 50;

	auto& toogleYawButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toogle yaw");
	toogleYawButton.draw();
	if (toogleYawButton.pressed) {
		m1OrientationOSCServer.command_setTrackingYawEnabled(!m1OrientationOSCServer.getTrackingYawEnabled());
	}
	offsetY += 50;

	auto& tooglePitchButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toogle pitch");
	tooglePitchButton.draw();
	if (tooglePitchButton.pressed) {
		m1OrientationOSCServer.command_setTrackingPitchEnabled(!m1OrientationOSCServer.getTrackingPitchEnabled());
	}
	offsetY += 50;

	auto& toogleRollButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toogle roll");
	toogleRollButton.draw();
	if (toogleRollButton.pressed) {
		m1OrientationOSCServer.command_setTrackingRollEnabled(!m1OrientationOSCServer.getTrackingRollEnabled());
	}
	offsetY += 50;

	auto& disconnectButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("disconnect");
	disconnectButton.draw();
	if (disconnectButton.pressed) {
		m1OrientationOSCServer.command_disconnect();
	}
	offsetY += 50;

	m.end();
}

void MainComponent::paint(juce::Graphics& g)
{
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
