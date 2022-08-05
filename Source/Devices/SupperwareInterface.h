//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>

#include "ht-api-juce/supperware/HeadMatrix.h"
#include "ht-api-juce/supperware/Tracker.h"
#include "ht-api-juce/supperware/midi/midi.h"
#include "ht-api-juce/supperware/configpanel/configPanel.h"
#include "ht-api-juce/supperware/headpanel/headPanel.h"

//==============================================================================
/*
*/
class SupperwareInterface  : public juce::Component, HeadPanel::HeadPanel::Listener
{
public:
    SupperwareInterface();
    ~SupperwareInterface() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void trackerChanged(const HeadMatrix& headMatrix) override;

private:
    HeadPanel::HeadPanel headPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SupperwareInterface)
};
