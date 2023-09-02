#pragma once

#include <JuceHeader.h>

#include "juce_murka/JuceMurkaBaseComponent.h"

#include "Config.h"
#include "m1_orientation_client/UI/M1Label.h"
#include "m1_orientation_client/UI/M1OrientationWindowToggleButton.h"
#include "m1_orientation_client/UI/M1OrientationClientWindow.h"

using namespace murka;

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

    std::string status;

    void timerCallback() override;
    
    juce::OSCSender output_osc_sender;
    
    bool yawActive = true;
    bool pitchActive = true;
    bool rollActive = true;

private:
    //==============================================================================
    MurImage m1logo;
    
    // Orientation Manager/Client
    void setStatus(bool success, std::string message);
    M1OrientationOSCClient m1OrientationOSCClient;
    M1OrientationYPR currentOrientation;
    
    M1OrientationClientWindow orientationControlWindow;
    bool showOrientationControlMenu = false;
    bool showedOrientationControlBefore = false;
    bool showMonitorModeDropdown = false;
    
    void update_orientation_client_window(murka::Murka &m, M1OrientationOSCClient &m1OrientationOSCClient, M1OrientationClientWindow &orientationControlWindow, bool &showOrientationControlMenu, bool showedOrientationControlBefore);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
