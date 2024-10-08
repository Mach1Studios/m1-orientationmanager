
#include "MainComponent.h"

#include <signal.h>
#include <sys/types.h>
#if !defined(JUCE_WINDOWS)
#include <unistd.h>
#endif

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
    
    // print build time for debug
    juce::String date(__DATE__);
    juce::String time(__TIME__);
    DBG("[OSCCLIENT] Build date: " + date + " | Build time: " + time);
}

MainComponent::~MainComponent()
{
    m1OrientationClient.command_disconnect();
    m1OrientationClient.close();
    stopTimer();
    murka::JuceMurkaBaseComponent::shutdown();
    juce::OpenGLAppComponent::shutdownOpenGL();
}

//==============================================================================
void MainComponent::initialise()
{
	murka::JuceMurkaBaseComponent::initialise();

    juce::File settingsFile;

#ifdef M1_ORIENTATION_MANAGER_EMBEDDED
    // This tool only looks for local m1-orientationmanager executables and "settings.json" file
    
    // TODO: test for already running service first!!
    
    // run process m1-orientationmanager.exe from the same folder
    juce::ChildProcess orientationManagerProcess;
    
    juce::File m1orientationmanager_exe;
    m1orientationmanager_exe = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile).getSiblingFile("m1-orientationmanager");
    DBG("Opening m1-orientationmanager exec: " + m1orientationmanager_exe.getFullPathName().quoted());

    // runs as a clear external process to help the user understand the separation of services
    if (m1orientationmanager_exe.startAsProcess()) {
        DBG("Started m1-orientationmanager server");
    } else {
        // Failed to start the process
        DBG("Failed to start the m1-orientationmanager");
        juce::JUCEApplicationBase::quit();
    }
    
    if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
        settingsFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile).getChildFile("Contents").getChildFile("Resources").getChildFile("settings.json");
    } else {
        settingsFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile).getSiblingFile("settings.json");
    }
#else
    // use the typical installation and service locations of m1-orientationmanager
    
    // We will assume the folders are properly created during the installation step
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
#endif
    
    m1OrientationClient.setClientType("osc-client"); // Needs to be set before the init() function
    DBG("Opening settings file: " + settingsFile.getFullPathName().quoted());
    m1OrientationClient.initFromSettings(settingsFile.getFullPathName().toStdString());
	m1OrientationClient.setStatusCallback(std::bind(&MainComponent::setStatus, this, std::placeholders::_1, std::placeholders::_2));
    
    m1logo.loadFromRawData(BinaryData::mach1logo_png, BinaryData::mach1logo_pngSize);
    
    // Telling Murka we're not in a plugin
    m.isPlugin = false;
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
    if (m1OrientationClient.isConnectedToServer() && isConnectedToOutput) {
        Mach1::Orientation oc_orientation = m1OrientationClient.getOrientation();
        if (output_send_as_ypr) {
            Mach1::Float3 ori_vec_deg = oc_orientation.GetGlobalRotationAsEulerDegrees();
            output_osc_sender.send(juce::String("/"+current_osc_msg_address),
                                   ori_vec_deg.GetYaw(),
                                   ori_vec_deg.GetPitch(),
                                   ori_vec_deg.GetRoll());
        } else {
            Mach1::Quaternion ori_quat = oc_orientation.GetGlobalRotationAsQuaternion();
            output_osc_sender.send(juce::String("/"+current_osc_msg_address),
                                   ori_quat.GetW(),
                                   ori_quat.GetX(),
                                   ori_quat.GetY(),
                                   ori_quat.GetZ());
        }
    }
}

void MainComponent::draw_orientation_client(murka::Murka &m, M1OrientationClient &m1OrientationClient) {
    std::vector<M1OrientationClientWindowDeviceSlot> slots;

    std::vector<M1OrientationDeviceInfo> devices = m1OrientationClient.getDevices();
    for (int i = 0; i < devices.size(); i++) {
        std::string icon = "";
        if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial && devices[i].
            getDeviceName().find("Bluetooth-Incoming-Port") != std::string::npos) {
            icon = "bt";
        } else if (devices[i].getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeSerial &&
                   devices[i].getDeviceName().find("Mach1-") != std::string::npos) {
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
        slots.push_back({
            icon,
            name,
            name == m1OrientationClient.getCurrentDevice().getDeviceName(),
            i,
            [&](int idx) {
                m1OrientationClient.command_startTrackingUsingDevice(devices[idx]);
            }
        });
    }
    
    //m.getSize().width() - 218 - 5 , 5, 218, 240
    float rightSide_LeftBound_x = m.getSize().width() / 2 + 40;
    float settings_topBound_y = m.getSize().height() * 0.23f + 18;

    // trigger a server side refresh for listed devices while menu is open
    m1OrientationClient.command_refresh();
    //bool showOrientationSettingsPanelInsideWindow = (m1OrientationClient.getCurrentDevice().getDeviceType() != M1OrientationManagerDeviceTypeNone);
    
    m.setFontFromRawData(PLUGIN_FONT, BINARYDATA_FONT, BINARYDATA_FONT_SIZE, DEFAULT_FONT_SIZE-2);
    orientationControlWindow = &(m.prepare<M1OrientationClientWindow>({ rightSide_LeftBound_x, settings_topBound_y, 300, 400}));
    orientationControlWindow->withDeviceSlots(slots);
    orientationControlWindow->withOrientationClient(m1OrientationClient);
    orientationControlWindow->draw();
}

//==============================================================================

void MainComponent::setStatus(bool success, std::string message)
{
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
    
    std::string connect_msg = (m1OrientationClient.isConnectedToServer()) ? "YES" : "NO";
    m.getCurrentFont()->drawString("CONNECTED: " + connect_msg, offsetX, offsetY);
    
    offsetY += 15;

    m.getCurrentFont()->drawString("DEVICE: " + m1OrientationClient.getCurrentDevice().getDeviceName() + ":" + M1OrientationDeviceTypeName[m1OrientationClient.getCurrentDevice().getDeviceType()], offsetX, offsetY);
    
    offsetY += 30;
    
    Mach1::Orientation orientation = m1OrientationClient.getOrientation();
    m.getCurrentFont()->drawString("ORIENTATION: ", offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("Y:  " + std::to_string(orientation.GetGlobalRotationAsEulerDegrees().GetYaw()), offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("P:  " + std::to_string(orientation.GetGlobalRotationAsEulerDegrees().GetPitch()), offsetX, offsetY);
    offsetY += 15;
    m.getCurrentFont()->drawString("R:  " + std::to_string(orientation.GetGlobalRotationAsEulerDegrees().GetRoll()), offsetX, offsetY);
    
    offsetY += 30;
    
    m.getCurrentFont()->drawString("TRACKING: ", offsetX, offsetY);
    offsetY += 15;
    std::string yaw_enabled_msg = (m1OrientationClient.getTrackingYawEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("Y:  " + yaw_enabled_msg, offsetX, offsetY);
    offsetY += 15;
    std::string pitch_enabled_msg = (m1OrientationClient.getTrackingPitchEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("P:  " + pitch_enabled_msg, offsetX, offsetY);
    offsetY += 15;
    std::string roll_enabled_msg = (m1OrientationClient.getTrackingRollEnabled()) ? "ENABLED" : "DISABLED";
    m.getCurrentFont()->drawString("R:  " + roll_enabled_msg, offsetX, offsetY);
    
    offsetY += 30;

    // IP ADDRESS TEXTFIELD
    m.setColor(BACKGROUND_COMPONENT);
    m.enableFill();
    m.drawRectangle(offsetX, offsetY, 200, 30);
    m.disableFill();
    
    m.setColor(ENABLED_PARAM);
    auto& ip_address_field = m.prepare<murka::TextField>({offsetX, offsetY, 200, 30}).onlyAllowNumbers(false).controlling(&requested_osc_ip_address);
    //ip_address_field.widgetBgColor.a = 0;
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
    //ip_port_field.widgetBgColor.a = 0;
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
    //msg_address_pattern_field.widgetBgColor.a = 0;
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
    
    // orientation client window
    draw_orientation_client(m, m1OrientationClient);
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
