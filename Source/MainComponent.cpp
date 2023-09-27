//
//  m1-orientationmanager
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
    m1OrientationOSCServer.initFromSettings(settingsFile.getFullPathName().toStdString());
    
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
    m.setFontFromRawData(PLUGIN_FONT, BINARYDATA_FONT, BINARYDATA_FONT_SIZE, DEFAULT_FONT_SIZE-2);

    m.clear(20);
    m.setColor(255);

    int offsetX = 0;
    int offsetY = 0;

    offsetX = 10;
    offsetY = 5;
    
    m.getCurrentFont()->drawString("DEVICE: " + m1OrientationOSCServer.getConnectedDevice().getDeviceName() + ":" + M1OrientationDeviceTypeName[m1OrientationOSCServer.getConnectedDevice().getDeviceType()], offsetX, offsetY);
    
    offsetY += 30;
    
    m.getCurrentFont()->drawString("ORIENTATION: ", offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("Y:  " + std::to_string(orientation.getYPRasDegrees().yaw), offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("P: " + std::to_string(orientation.getYPRasDegrees().pitch), offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("R:   " + std::to_string(orientation.getYPRasDegrees().roll), offsetX, offsetY);
    
    offsetY += 30;
    
    m.getCurrentFont()->drawString("TRACKING: ", offsetX, offsetY);
    offsetY += 15;
    std::string yaw_enabled_msg = (m1OrientationOSCServer.getTrackingYawEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("Y:  " + yaw_enabled_msg, offsetX, offsetY);
    offsetY += 15;
    std::string pitch_enabled_msg = (m1OrientationOSCServer.getTrackingPitchEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("P: " + pitch_enabled_msg, offsetX, offsetY);
    offsetY += 15;
    std::string roll_enabled_msg = (m1OrientationOSCServer.getTrackingRollEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("R:   " + roll_enabled_msg, offsetX, offsetY);
    
    offsetY += 30;
    
    std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
    for (auto& device : devices) {
        m.getCurrentFont()->drawString("> ["+M1OrientationDeviceTypeName[device.getDeviceType()]+"]: "+device.getDeviceName(), offsetX, offsetY);
        offsetY += 15;
    }

    //m.setColor(200, 255);
    //m.drawImage(m1logo, 15, m.getSize().height() - 20, 161 / 4, 39 / 4);
    
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
