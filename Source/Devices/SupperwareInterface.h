//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#pragma once

#include <JuceHeader.h>

#include "ht-api-juce/supperware/HeadMatrix.h"
#include "ht-api-juce/supperware/Tracker.h"
#include "ht-api-juce/supperware/midi/midi.h"
#include "headpanel-PointList.h"
#include "headpanel-Points.h"
#include "headpanel-Plotter.h"

//==============================================================================
/*
*/
class SupperwareInterface: public juce::Timer, Midi::TrackerDriver::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        //virtual std::vector<float> trackerChanged(const HeadMatrix& headMatrix) = 0;
    };
    
    SupperwareInterface();
    ~SupperwareInterface();

    const Midi::TrackerDriver& getTrackerDriver() const;
    const HeadMatrix& getHeadMatrix() const;
    void trackerOrientation(float yawRadian, float pitchRadian, float rollRadian) override;
    void trackerOrientationQ(float qw, float qx, float qy, float qz) override;
    void trackerMidiConnectionChanged(Midi::State newState) override;
    void trackerZero();
    void connectSupperware();
    void setListener(Listener* l);
    void timerCallback() override;
    
    // Public current orientation
    std::vector<float> currentOrientation;
    std::vector<float> previousOrientation; // used to calculate delta

private:
    Listener* listener;
    Midi::State midiState;
    Midi::TrackerDriver trackerDriver;
    HeadMatrix headMatrix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SupperwareInterface)
};
