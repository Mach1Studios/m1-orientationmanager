
#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	// Make sure you set the size of the component after
	// you add any child components.
	juce::OpenGLAppComponent::setSize(350, 600);
    
    startTimer(10); // send output osc every 10ms
    
    // initial osc output start
    update_osc_address_pattern(requested_osc_msg_address);
    update_osc_destination(requested_osc_ip_address, requested_osc_port);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

//==============================================================================
void MainComponent::initialise()
{
	murka::JuceMurkaBaseComponent::initialise();

    // This tool only looks for sibling m1-orientationmanager executables and "settings.json" file
    juce::File settingsFile;
    settingsFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile).getSiblingFile("settings.json");
    DBG("Opening settings file: " + settingsFile.getFullPathName().quoted());

    m1OrientationOSCClient.initFromSettings(settingsFile.getFullPathName().toStdString());
	m1OrientationOSCClient.setStatusCallback(std::bind(&MainComponent::setStatus, this, std::placeholders::_1, std::placeholders::_2));
    
    m1logo.loadFromRawData(BinaryData::mach1logo_png, BinaryData::mach1logo_pngSize);
}

void MainComponent::update_osc_address_pattern(std::string new_pattern) {
    if (current_osc_msg_address != new_pattern) {
        current_osc_msg_address = new_pattern;
    }
}

void MainComponent::update_osc_destination(std::string new_address, int new_port) {
    if (current_osc_ip_address != new_address || current_osc_port != new_port) {
        output_osc_sender.disconnect();
        isConnectedToOutput = output_osc_sender.connect(new_address, new_port);
        if (isConnectedToOutput) {
            // apply connection for next check
            current_osc_ip_address = new_address;
            current_osc_port = new_port;
        }
    }
}

void MainComponent::timerCallback() {
    if (m1OrientationOSCClient.isConnectedToServer() && isConnectedToOutput) {
        if (output_send_as_ypr) {
            output_osc_sender.send(juce::String("/"+current_osc_msg_address),
                m1OrientationOSCClient.getOrientation().getYPRinDegrees().yaw,
                m1OrientationOSCClient.getOrientation().getYPRinDegrees().pitch,
                m1OrientationOSCClient.getOrientation().getYPRinDegrees().roll);
        } else {
            output_osc_sender.send(juce::String("/"+current_osc_msg_address),
                m1OrientationOSCClient.getOrientation().getQuat().w,
                m1OrientationOSCClient.getOrientation().getQuat().x,
                m1OrientationOSCClient.getOrientation().getQuat().y,
                m1OrientationOSCClient.getOrientation().getQuat().z);
        }
    }
}

void MainComponent::update_orientation_client_window(murka::Murka &m, M1OrientationOSCClient &m1OrientationOSCClient, M1OrientationClientWindow* orientationControlWindow, bool &showOrientationControlMenu, bool showedOrientationControlBefore) {
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
        orientationControlWindow = &(m.prepare<M1OrientationClientWindow>({ m.getSize().width() - 218 - 5 , 5, 218, 240 + 100 * showOrientationSettingsPanelInsideWindow })
            .withDeviceList(slots)
            .withSettingsPanelEnabled(showOrientationSettingsPanelInsideWindow)
            .onClickOutside([&]() {
                if (!orientationControlButton.hovered) { // Only switch showing the orientation control if we didn't click on the button
                    showOrientationControlMenu = !showOrientationControlMenu;
                    if (showOrientationControlMenu && !showedOrientationControlBefore) {
                        orientationControlWindow->startRefreshing();
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
            ));
            orientationControlWindow->draw();
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
    m.setFontFromRawData(PLUGIN_FONT, BINARYDATA_FONT, BINARYDATA_FONT_SIZE, DEFAULT_FONT_SIZE-2);

    m.clear(20);
    m.setColor(255);

    int offsetX = 0;
    int offsetY = 0;

    offsetX = 10;
    offsetY = 5;
    
    std::string connect_msg = (m1OrientationOSCClient.isConnectedToServer()) ? "YES" : "NO";
    m.getCurrentFont()->drawString("CONNECTED: " + connect_msg, offsetX, offsetY);
    
    offsetY += 15;

    m.getCurrentFont()->drawString("DEVICE: " + m1OrientationOSCClient.getCurrentDevice().getDeviceName() + ":" + M1OrientationDeviceTypeName[m1OrientationOSCClient.getCurrentDevice().getDeviceType()], offsetX, offsetY);
    
    offsetY += 30;
    
    Orientation orientation = m1OrientationOSCClient.getOrientation();
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
    std::string yaw_enabled_msg = (m1OrientationOSCClient.getTrackingYawEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("Y:  " + yaw_enabled_msg, offsetX, offsetY);
    offsetY += 15;
    std::string pitch_enabled_msg = (m1OrientationOSCClient.getTrackingPitchEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("P: " + pitch_enabled_msg, offsetX, offsetY);
    offsetY += 15;
    std::string roll_enabled_msg = (m1OrientationOSCClient.getTrackingRollEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("R:   " + roll_enabled_msg, offsetX, offsetY);
    
    offsetY += 30;

    // IP ADDRESS TEXTFIELD
    m.setColor(BACKGROUND_COMPONENT);
    m.enableFill();
    m.drawRectangle(offsetX, offsetY, 200, 30);
    m.disableFill();
    
    m.setColor(ENABLED_PARAM);
    auto& ip_address_field = m.prepare<murka::TextField>({offsetX, offsetY, 200, 30}).onlyAllowNumbers(false).controlling(&requested_osc_ip_address);
    ip_address_field.widgetBgColor.a = 0;
    ip_address_field.drawBounds = false;
    ip_address_field.hint = requested_osc_ip_address;
    ip_address_field.draw();
    
    offsetY += 40;
    
    // IP PORT TEXTFIELD
    m.setColor(BACKGROUND_COMPONENT);
    m.enableFill();
    m.drawRectangle(offsetX, offsetY, 100, 30);
    m.disableFill();

    m.setColor(ENABLED_PARAM);
    auto& ip_port_field = m.prepare<murka::TextField>({offsetX, offsetY, 100, 30}).onlyAllowNumbers(true).controlling(&requested_osc_port);
    ip_port_field.widgetBgColor.a = 0;
    ip_port_field.drawBounds = false;
    ip_port_field.hint = std::to_string(requested_osc_port);
    ip_port_field.draw();
    
    if (ip_port_field.editingFinished || ip_address_field.editingFinished) {
        update_osc_destination(requested_osc_ip_address, requested_osc_port);
    }
        
    offsetY += 40;

    // MSG ADDRESS PATTERN TEXTFIELD
    m.setColor(BACKGROUND_COMPONENT);
    m.enableFill();
    m.drawRectangle(offsetX, offsetY, 200, 30);
    m.disableFill();
    
    m.setColor(ENABLED_PARAM);
    auto& msg_address_pattern_field = m.prepare<murka::TextField>({offsetX, offsetY, 200, 30}).onlyAllowNumbers(false).controlling(&requested_osc_msg_address);
    msg_address_pattern_field.widgetBgColor.a = 0;
    msg_address_pattern_field.drawBounds = false;
    msg_address_pattern_field.hint = "/"+requested_osc_msg_address;
    msg_address_pattern_field.draw();
    
    if (msg_address_pattern_field.editingFinished) {
        update_osc_address_pattern(requested_osc_msg_address);
    }
    
    offsetY += 40;
    
    auto& sendAsYprButton = m.prepare<M1Checkbox>({ offsetX, offsetY, 120, 20 })
    .controlling(&output_send_as_ypr)
    .withLabel("SEND AS YPR");
    sendAsYprButton.fontSize = DEFAULT_FONT_SIZE-2;
    sendAsYprButton.enabled = true;
    sendAsYprButton.draw();
    
    /// OSC label
    m.setColor(200, 255);
    auto& oscLabel = m.prepare<M1Label>(MurkaShape(m.getSize().width() - 65, m.getSize().height() - 20, 80, 20));
    oscLabel.label = "OSC";
    oscLabel.alignment = TEXT_CENTER;
    oscLabel.enabled = false;
    oscLabel.highlighted = false;
    oscLabel.draw();
    
    m.setColor(200, 255);
    m.drawImage(m1logo, 15, m.getSize().height() - 20, 161 / 4, 39 / 4);
    
    // orientation button
    update_orientation_client_window(m, m1OrientationOSCClient, orientationControlWindow, showOrientationControlMenu, showedOrientationControlBefore);
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
