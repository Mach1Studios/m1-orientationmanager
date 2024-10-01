#pragma once

#include <JuceHeader.h>

#include "MurkaBasicWidgets.h"
#include "juce_murka/JuceMurkaBaseComponent.h"

#include "Config.h"
#include "TextField.h"
#include "M1Checkbox.h"
#include "m1_orientation_client/UI/M1Label.h"
#include "m1_orientation_client/UI/M1OrientationWindowToggleButton.h"
#include "m1_orientation_client/UI/M1OrientationClientWindow.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public JuceMurkaBaseComponent, public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void initialise() override;
    void draw() override;

    void timerCallback() override;
    
    bool isConnectedToOutput = false;
    juce::OSCSender output_osc_sender;
    std::string requested_osc_ip_address = "127.0.0.1";
    std::string current_osc_ip_address = "";
    int requested_osc_port = 9999;
    int current_osc_port;
    std::string requested_osc_msg_address = "orientation";
    std::string current_osc_msg_address = "";
    bool output_send_as_ypr = true;
    
    // Orientation Manager/Client
    M1OrientationClient m1OrientationClient;
    M1OrientationClientWindow* orientationControlWindow;
    bool showOrientationControlMenu = false;
    bool showedOrientationControlBefore = false;
    void setStatus(bool success, std::string message);
    Mach1::Orientation currentOrientation;

    void draw_orientation_client(murka::Murka &m, M1OrientationClient &m1OrientationClient);
    
private:
    //==============================================================================
    MurImage m1logo;
    
    void update_osc_address_pattern(std::string new_address_pattern);
    void update_osc_destination(std::string ip_address, int port);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

// (This function is called by the app startup code to create our main component)
juce::Component* createMainContentComponent() { return (juce::OpenGLAppComponent*)new MainComponent(); }
