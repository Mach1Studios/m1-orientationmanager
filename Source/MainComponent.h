//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include "Config.h"
#include <JuceHeader.h>
#include "juce_murka/JuceMurkaBaseComponent.h"
#include "M1OrientationManagerService.h"

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
    ~MainComponent();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void initialise() override;
    void draw() override;
    
    bool yawActive = true;
    bool pitchActive = true;
    bool rollActive = true;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
