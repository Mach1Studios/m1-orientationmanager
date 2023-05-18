//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>
#include "juce_murka/JuceMurkaBaseComponent.h"

#include "HardwareBLE.h"
#include "HardwareSerial.h"
#include "HardwareOSC.h"
#include "HardwareEmulator.h"

#include "M1OrientationOSCServer.h"

using namespace murka;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public JuceMurkaBaseComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void initialise() override;
    void render() override;

private:
    //==============================================================================
    // Your private member variables go here...

    HardwareBLE hardwareBLE;
    HardwareSerial hardwareSerial;
	HardwareOSC hardwareOSC;
	HardwareEmulator hardwareEmulator;
	// ...

	
    M1OrientationOSCServer m1OrientationOSCServer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
