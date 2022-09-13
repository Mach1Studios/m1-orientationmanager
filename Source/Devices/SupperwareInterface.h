//
//  M1-OrientationManager
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
class SupperwareInterface: public Midi::TrackerDriver::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        virtual void trackerChanged(const HeadMatrix& headMatrix) = 0;
    };
    
    SupperwareInterface();
    ~SupperwareInterface();

    const Midi::TrackerDriver& getTrackerDriver() const;
    const HeadMatrix& getHeadMatrix() const;
    void trackerOrientation(float yawRadian, float pitchRadian, float rollRadian) override;
    void trackerOrientationQ(float qw, float qx, float qy, float qz) override;
    void trackerMidiConnectionChanged(Midi::State newState) override;
//    void trackerCompassStateChanged(Tracker::CompassState compassState) override;
//    void trackerConnectionChanged(const Tracker::State& state) override;
    void setListener(Listener* l);
    void connectSupperware();

    void trackerChanged(const HeadMatrix& headMatrix);

private:
    Listener* listener;
    Midi::State midiState;
    Midi::TrackerDriver trackerDriver;
    HeadMatrix headMatrix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SupperwareInterface)
};
