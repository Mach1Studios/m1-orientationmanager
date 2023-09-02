
#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	// Make sure you set the size of the component after
	// you add any child components.
	juce::OpenGLAppComponent::setSize(300, 600);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::initialise()
{
	murka::JuceMurkaBaseComponent::initialise();

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

    m1OrientationOSCClient.initFromSettings(settingsFile.getFullPathName().toStdString(), false); // the bool determines if we want to also launch the watcher helper executable to relaunch the server after any unexepected crashes
	m1OrientationOSCClient.setStatusCallback(std::bind(&MainComponent::setStatus, this, std::placeholders::_1, std::placeholders::_2));
    
    m1logo.loadFromRawData(BinaryData::mach1logo_png, BinaryData::mach1logo_pngSize);
}

void MainComponent::timerCallback()
{
    if (m1OrientationOSCClient.isConnectedToServer()) {
        if (m1OrientationOSCClient.getCurrentDevice().getDeviceName() != "" && m1OrientationOSCClient.getCurrentDevice().getDeviceAddress() != "") {
            //output_osc_sender
        }
    }
}

void MainComponent::update_orientation_client_window(murka::Murka &m, M1OrientationOSCClient &m1OrientationOSCClient, M1OrientationClientWindow &orientationControlWindow, bool &showOrientationControlMenu, bool showedOrientationControlBefore) {
    std::vector<M1OrientationClientWindowDeviceSlot> slots;
    
    std::vector<M1OrientationDeviceInfo> devices = m1OrientationOSCClient.getDevices();
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
        slots.push_back({ icon, name, name == m1OrientationOSCClient.getCurrentDevice().getDeviceName(), i, [&](int idx)
            {
                m1OrientationOSCClient.command_startTrackingUsingDevice(devices[idx]);
            }
        });
    }
    
    auto& orientationControlButton = m.prepare<M1OrientationWindowToggleButton>({ m.getSize().width() - 40 - 5, 5, 40, 40 }).onClick([&](M1OrientationWindowToggleButton& b) {
        showOrientationControlMenu = !showOrientationControlMenu;
    })
        .withInteractiveOrientationGimmick(m1OrientationOSCClient.getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone, m1OrientationOSCClient.getOrientation().getYPRinDegrees().yaw)
        .draw();
    
    // TODO: move this to be to the left of the orientation client window button
    if (std::holds_alternative<bool>(m1OrientationOSCClient.getCurrentDevice().batteryPercentage)) {
        // it's false, which means the battery percentage is unknown
    } else {
        // it has a battery percentage value
        int battery_value = std::get<int>(m1OrientationOSCClient.getCurrentDevice().batteryPercentage);
        m.getCurrentFont()->drawString("Battery: " + std::to_string(battery_value), m.getWindowWidth() - 100, m.getWindowHeight() - 100);
    }
    
    if (orientationControlButton.hovered && (m1OrientationOSCClient.getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone)) {
        std::string deviceReportString = "CONNECTED DEVICE: " + m1OrientationOSCClient.getCurrentDevice().getDeviceName();
        auto font = m.getCurrentFont();
        auto bbox = font->getStringBoundingBox(deviceReportString, 0, 0);
        //m.setColor(40, 40, 40, 200);
        // TODO: fix this bounding box (doesnt draw the same place despite matching settings with Label.draw
        //m.drawRectangle(     m.getSize().width() - 40 - 10 /* padding */ - bbox.width - 5, 5, bbox.width + 10, 40);
        m.setColor(230, 230, 230);
        m.prepare<M1Label>({ m.getSize().width() - 40 - 10 /* padding */ - bbox.width - 5, 5 + 10, bbox.width + 10, 40 }).text(deviceReportString).withTextAlignment(TEXT_CENTER).draw();
    }
    
    if (showOrientationControlMenu) {
        bool showOrientationSettingsPanelInsideWindow = (m1OrientationOSCClient.getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone);
        orientationControlWindow = m.prepare<M1OrientationClientWindow>({ m.getSize().width() - 218 - 5 , 5, 218, 240 + 100 * showOrientationSettingsPanelInsideWindow })
            .withDeviceList(slots)
            .withSettingsPanelEnabled(showOrientationSettingsPanelInsideWindow)
            .onClickOutside([&]() {
                if (!orientationControlButton.hovered) { // Only switch showing the orientation control if we didn't click on the button
                    showOrientationControlMenu = !showOrientationControlMenu;
                    if (showOrientationControlMenu && !showedOrientationControlBefore) {
                        orientationControlWindow.startRefreshing();
                    }
                }
            })
            .onDisconnectClicked([&]() {
                m1OrientationOSCClient.command_disconnect();
            })
            .onRefreshClicked([&]() {
                m1OrientationOSCClient.command_refreshDevices();
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
                                     m1OrientationOSCClient.getTrackingYawEnabled(),
                                     m1OrientationOSCClient.getTrackingPitchEnabled(),
                                     m1OrientationOSCClient.getTrackingRollEnabled(),
                                     std::pair<int, int>(0, 180),
                                     std::pair<int, int>(0, 180),
                                     std::pair<int, int>(0, 180)
            )
            .withYPR(
                     m1OrientationOSCClient.getOrientation().getYPRinDegrees().yaw,
                     m1OrientationOSCClient.getOrientation().getYPRinDegrees().pitch,
                     m1OrientationOSCClient.getOrientation().getYPRinDegrees().roll
            );
            orientationControlWindow.draw();
    }
}

//==============================================================================

void MainComponent::setStatus(bool success, std::string message)
{
	this->status = message;
	std::cout << success << " , " << message << std::endl;
}

void MainComponent::draw()
{
    // This clears the context with our background.
    //juce::OpenGLHelpers::clear(juce::Colour(255.0, 198.0, 30.0));
    
    float scale = (float)openGLContext.getRenderingScale() * 0.7; // (Desktop::getInstance().getMainMouseSource().getScreenPosition().x / 300.0); //  0.7;

    if (scale != m.getScreenScale()) {
        m.setScreenScale(scale);
        m.updateFontsTextures(&m);
        m.clearFontsTextures();
    }
   
    m.setColor(BACKGROUND_GREY);
    m.clear();
        
    m.setFontFromRawData(PLUGIN_FONT, BINARYDATA_FONT, BINARYDATA_FONT_SIZE, DEFAULT_FONT_SIZE);

    m.startFrame();
    m.setScreenScale((float)openGLContext.getRenderingScale());

    m.clear(20);
    m.setColor(255);

    m.begin();

    int offsetX = 0;
    int offsetY = 0;

    offsetX = 10;
    offsetY = 20;

    m.getCurrentFont()->drawString("connected: " + std::to_string(m1OrientationOSCClient.isConnectedToServer()), offsetX, offsetY);
    offsetY += 30;

    offsetY += 20;

    Orientation orientation = m1OrientationOSCClient.getOrientation();

    m.getCurrentFont()->drawString("orientation: ", offsetX, offsetY);
    offsetY += 30;
    m.getCurrentFont()->drawString("yaw: " + std::to_string(orientation.getYPRinDegrees().yaw), offsetX, offsetY);
    offsetY += 30;
    m.getCurrentFont()->drawString("pitch: " + std::to_string(orientation.getYPRinDegrees().pitch), offsetX, offsetY);
    offsetY += 30;
    m.getCurrentFont()->drawString("roll: " + std::to_string(orientation.getYPRinDegrees().roll), offsetX, offsetY);
    offsetY += 30;

    offsetY += 20;

    m.getCurrentFont()->drawString("tracking enabled: ", offsetX, offsetY);
    offsetY += 30;
    m.getCurrentFont()->drawString("yaw: " + std::to_string(m1OrientationOSCClient.getTrackingYawEnabled()), offsetX, offsetY);
    offsetY += 30;
    m.getCurrentFont()->drawString("pitch: " + std::to_string(m1OrientationOSCClient.getTrackingPitchEnabled()), offsetX, offsetY);
    offsetY += 30;
    m.getCurrentFont()->drawString("roll: " + std::to_string(m1OrientationOSCClient.getTrackingRollEnabled()), offsetX, offsetY);
    offsetY += 30;

    offsetY += 20;

    m.getCurrentFont()->drawString("device: " + m1OrientationOSCClient.getCurrentDevice().getDeviceName() + ":" + M1OrientationDeviceTypeName[m1OrientationOSCClient.getCurrentDevice().getDeviceType()], offsetX, offsetY);
    offsetY += 40;
        
    // orientation button
    update_orientation_client_window(m, m1OrientationOSCClient, orientationControlWindow, showOrientationControlMenu, showedOrientationControlBefore);
    
    /// OSC label
    m.setColor(200, 255);
    m.setFontFromRawData(PLUGIN_FONT, BINARYDATA_FONT, BINARYDATA_FONT_SIZE, DEFAULT_FONT_SIZE);

    int labelYOffset;
    auto& oscLabel = m.prepare<M1Label>(MurkaShape(m.getSize().width() - 100, m.getSize().height() - labelYOffset, 80, 20));
    oscLabel.label = "OSC";
    oscLabel.alignment = TEXT_CENTER;
    oscLabel.enabled = false;
    oscLabel.highlighted = false;
    oscLabel.draw();
    
    m.setColor(200, 255);
    m.drawImage(m1logo, 30, m.getSize().height() - labelYOffset, 161 / 3, 39 / 3);
    
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
