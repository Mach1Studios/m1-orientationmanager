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

void MainComponent::update_orientation_client_window(murka::Murka &m, M1OrientationOSCServer &m1OrientationOSCServer, M1OrientationClientWindow* orientationControlWindow, bool &showOrientationControlMenu, bool showedOrientationControlBefore) {
    std::vector<M1OrientationClientWindowDeviceSlot> slots;
    
    std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCServer.getDevices();
    for (int i = 0; i < devices.size(); i++) {
        std::string icon = "";
        if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial && devices[i].getDeviceName().find("Bluetooth-Incoming-Port") != std::string::npos) {
            icon = "bt";
        } else if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial && devices[i].getDeviceName().find("Mach1-") != std::string::npos) {
            icon = "bt";
        } else if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE) {
            icon = "bt";
        } else if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial) {
            icon = "usb";
        } else if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeCamera) {
            icon = "camera";
        } else if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeEmulator) {
            icon = "none";
        } else {
            icon = "wifi";
        }
                
        std::string name = devices[i].getDeviceName();
        slots.push_back({ icon, name, name == m1OrientationOSCServer.getConnectedDevice().getDeviceName(), i, [&](int idx)
            {
                m1OrientationOSCServer.command_startTrackingUsingDevice(devices[idx]);
            }
        });
    }
    
    auto& orientationControlButton = m.prepare<M1OrientationWindowToggleButton>({ m.getSize().width() - 40 - 5, 5, 40, 40 }).onClick([&](M1OrientationWindowToggleButton& b) {
        showOrientationControlMenu = !showOrientationControlMenu;
    })
        .withInteractiveOrientationGimmick(m1OrientationOSCServer.getConnectedDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone, m1OrientationOSCServer.getOrientation().getYPRinDegrees().yaw)
        .draw();
    
    // TODO: move this to be to the left of the orientation client window button
    if (std::holds_alternative<bool>(m1OrientationOSCServer.getConnectedDevice().batteryPercentage)) {
        // it's false, which means the battery percentage is unknown
    } else {
        // it has a battery percentage value
        int battery_value = std::get<int>(m1OrientationOSCServer.getConnectedDevice().batteryPercentage);
        m.getCurrentFont()->drawString("Battery: " + std::to_string(battery_value), m.getSize().width() - 50 - 40 - 5, 10);
    }
    
    if (orientationControlButton.hovered && (m1OrientationOSCServer.getConnectedDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone)) {
        std::string deviceReportString = "CONNECTED DEVICE: " + m1OrientationOSCServer.getConnectedDevice().getDeviceName();
        auto font = m.getCurrentFont();
        auto bbox = font->getStringBoundingBox(deviceReportString, 0, 0);
        //m.setColor(40, 40, 40, 200);
        // TODO: fix this bounding box (doesnt draw the same place despite matching settings with Label.draw
        //m.drawRectangle(     m.getSize().width() - 40 - 10 /* padding */ - bbox.width - 5, 5, bbox.width + 10, 40);
        m.setColor(230, 230, 230);
        m.prepare<M1Label>({ m.getSize().width() - 40 - 10 /* padding */ - bbox.width - 5, 5 + 10, bbox.width + 10, 40 }).text(deviceReportString).withTextAlignment(TEXT_CENTER).draw();
    }
    
    if (showOrientationControlMenu) {
        bool showOrientationSettingsPanelInsideWindow = (m1OrientationOSCServer.getConnectedDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone);
        orientationControlWindow = &(m.prepare<M1OrientationClientWindow>({ m.getSize().width() - 218 - 5 , 5, 218, 240 + 100 * showOrientationSettingsPanelInsideWindow })
            .withDeviceList(slots)
            .withSettingsPanelEnabled(showOrientationSettingsPanelInsideWindow)
            .withOscSettingsEnabled((m1OrientationOSCServer.getConnectedDevice().getDeviceType() == M1OrientationManagerDeviceTypeOSC))
            .onClickOutside([&]() {
                if (!orientationControlButton.hovered) { // Only switch showing the orientation control if we didn't click on the button
                    showOrientationControlMenu = !showOrientationControlMenu;
                    if (showOrientationControlMenu && !showedOrientationControlBefore) {
                        orientationControlWindow->startRefreshing();
                    }
                }
            })
            .onDisconnectClicked([&]() {
                m1OrientationOSCServer.command_disconnect();
            })
            .onRefreshClicked([&]() {
                m1OrientationOSCServer.command_refreshDevices();
            })
            .onOscSettingsChanged([&](int port, std::string addr_pttrn) {
                m1OrientationOSCServer.command_updateOscDevice(port, addr_pttrn);
            })
            .onYPRSwitchesClicked([&](int whichone) {
                if (whichone == 0)
                    // yaw clicked
                    yawActive = !yawActive;
                if (whichone == 1)
                    // pitch clicked
                    pitchActive = !pitchActive;
                if (whichone == 2)
                    // roll clicked
                    rollActive = !rollActive;
            })
            .withYPRTrackingSettings(
                                     m1OrientationOSCServer.getTrackingYawEnabled(),
                                     m1OrientationOSCServer.getTrackingPitchEnabled(),
                                     m1OrientationOSCServer.getTrackingRollEnabled(),
                                     std::pair<int, int>(0, 180),
                                     std::pair<int, int>(0, 180),
                                     std::pair<int, int>(0, 180)
            )
            .withYPR(
                     m1OrientationOSCServer.getOrientation().getYPRinDegrees().yaw,
                     m1OrientationOSCServer.getOrientation().getYPRinDegrees().pitch,
                     m1OrientationOSCServer.getOrientation().getYPRinDegrees().roll
            ));
            orientationControlWindow->draw();
    }
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
    m.getCurrentFont()->drawString("Y:  " + std::to_string(orientation.getYPRinDegrees().yaw), offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("P: " + std::to_string(orientation.getYPRinDegrees().pitch), offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("R:   " + std::to_string(orientation.getYPRinDegrees().roll), offsetX, offsetY);
    
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
    
    // orientation button
    update_orientation_client_window(m, m1OrientationOSCServer, orientationControlWindow, showOrientationControlMenu, showedOrientationControlBefore);
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
