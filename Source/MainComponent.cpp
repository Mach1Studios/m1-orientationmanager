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

}

MainComponent::~MainComponent()
{
	murka::JuceMurkaBaseComponent::shutdownOpenGL();
}

//==============================================================================
void MainComponent::initialise()
{
    murka::JuceMurkaBaseComponent::initialise();

    // Setup for shared resources
	std::string resourcesPath;
	if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
		resourcesPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("Application Support").getChildFile("/Mach1 Spatial System/resources").getFullPathName().toStdString();
	}
	else {
		resourcesPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("Mach1 Spatial System/resources").getFullPathName().toStdString();
	}
	printf("Resources Loaded From: %s \n", resourcesPath.c_str());
	m.setResourcesPath(resourcesPath);
    
    // We will assume the folders are properly created during the installation step
    juce::File settingsFile;
    // Using common support files installation location
    juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);

    if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
        // test for any mac OS
        settingsFile = m1SupportDirectory.getChildFile("Application Support").getChildFile("Mach1");
    } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
        // test for any windows OS
        settingsFile = m1SupportDirectory.getChildFile("Mach1");
    } else {
        settingsFile = m1SupportDirectory.getChildFile("Mach1");
    }
    settingsFile = settingsFile.getChildFile("settings.json");
    DBG("Opening settings file: " + settingsFile.getFullPathName().quoted());
    m1OrientationOSCServer.initFromSettings(settingsFile.getFullPathName().toStdString(), true);
    
    // For debug testing you can set this to false to list all connectable BLE devices
    hardwareBLE.displayOnlyKnownIMUs = true;
    hardwareBLE.setup();
    hardwareSerial.setup();
    hardwareOSC.setup();
    // Internal device emulator for debugging
    hardwareEmulator.setup();
    
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &hardwareBLE);
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeSerial, &hardwareSerial);
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeOSC, &hardwareOSC);
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeEmulator, &hardwareEmulator);
}

//==============================================================================
void MainComponent::draw()
{
	m1OrientationOSCServer.update();
	Orientation orientation = m1OrientationOSCServer.getOrientation();
    M1OrientationDeviceInfo device = m1OrientationOSCServer.getConnectedDevice();
	auto clients = m1OrientationOSCServer.getClients();

//#ifdef BUILD_DEBUG_UI
	m.setFontFromRawData(PLUGIN_FONT, BINARYDATA_FONT, BINARYDATA_FONT_SIZE, DEFAULT_FONT_SIZE);
	m.clear(20);
	m.setColor(255);

	int offsetY = 0;
	int offsetX = 0;
	
	offsetX = 10;
	offsetY = 20;

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

    auto& selectDevice3Button = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("select device 3");
    selectDevice3Button.draw();
    if (selectDevice3Button.pressed) {
        std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
        if (devices.size() > 1) {
           m1OrientationOSCServer.command_startTrackingUsingDevice(devices[2]);
        }
    }
    offsetY += 50;

    auto& selectDevice4Button = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("select device 4");
    selectDevice4Button.draw();
    if (selectDevice4Button.pressed) {
        std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
        if (devices.size() > 1) {
           m1OrientationOSCServer.command_startTrackingUsingDevice(devices[3]);
        }
    }
    offsetY += 50;
    
    auto& selectDevice5Button = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("select device 5");
    selectDevice5Button.draw();
    if (selectDevice5Button.pressed) {
        std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
        if (devices.size() > 1) {
           m1OrientationOSCServer.command_startTrackingUsingDevice(devices[4]);
        }
    }
    offsetY += 50;

	auto& toggleYawButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toggle yaw");
	toggleYawButton.draw();
	if (toggleYawButton.pressed) {
		m1OrientationOSCServer.command_setTrackingYawEnabled(!m1OrientationOSCServer.getTrackingYawEnabled());
	}
	offsetY += 50;

	auto& togglePitchButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toggle pitch");
	togglePitchButton.draw();
	if (togglePitchButton.pressed) {
		m1OrientationOSCServer.command_setTrackingPitchEnabled(!m1OrientationOSCServer.getTrackingPitchEnabled());
	}
	offsetY += 50;

	auto& toggleRollButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toggle roll");
	toggleRollButton.draw();
	if (toggleRollButton.pressed) {
		m1OrientationOSCServer.command_setTrackingRollEnabled(!m1OrientationOSCServer.getTrackingRollEnabled());
	}
	offsetY += 50;

	auto& disconnectButton = m.prepare<murka::Button>({ offsetX, offsetY, 130, 30 }).text("disconnect");
	disconnectButton.draw();
	if (disconnectButton.pressed) {
		m1OrientationOSCServer.command_disconnect();
	}
	offsetY += 50;
//#endif // end of debug UI macro
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
