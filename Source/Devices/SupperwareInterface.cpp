//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include <JuceHeader.h>
#include "SupperwareInterface.h"

//==============================================================================
SupperwareInterface::SupperwareInterface() :
    listener(nullptr),
    trackerDriver(this),
    headMatrix(),
    midiState(Midi::State::Unavailable)
{
}

SupperwareInterface::~SupperwareInterface()
{
}

const Midi::TrackerDriver& SupperwareInterface::getTrackerDriver() const
{
    return trackerDriver;
}

//----------------------------------------------------------------------

const HeadMatrix& SupperwareInterface::getHeadMatrix() const
{
    return headMatrix;
}

//----------------------------------------------------------------------

void SupperwareInterface::trackerMidiConnectionChanged(Midi::State newState)
{
    if (newState != midiState)
    {
        midiState = newState;

        if ((midiState == Midi::State::Connected) || (midiState == Midi::State::Bootloader))
        {
        }
        else if (midiState == Midi::State::Available)
        {
            headMatrix.zero();
        }
        else // Unavailable
        {
            headMatrix.zero();
        }
        if (listener) listener->trackerChanged(headMatrix);
        flagTimer();
    }
}

//----------------------------------------------------------------------

void SupperwareInterface::trackerOrientation(float yawRadian, float pitchRadian, float rollRadian)
{
    headMatrix.setOrientationYPR(yawRadian, pitchRadian, rollRadian);
    if (listener) listener->trackerChanged(headMatrix);
    flagTimer();
}

void SupperwareInterface::trackerOrientationQ(float qw, float qx, float qy, float qz)
{
    headMatrix.setOrientationQuaternion(qw, qx, qy, qz);
    if (listener) listener->trackerChanged(headMatrix);
    flagTimer();
}

void SupperwareInterface::trackerZero()
{
    trackerDriver.zero();
}

//----------------------------------------------------------------------

void SupperwareInterface::connectSupperware()
{
    // connect/disconnect
    if (midiState == Midi::State::Available) {
        trackerDriver.connect();
        trackerDriver.turnOn(false, true);
    } else {
        trackerDriver.disconnect();
    }
}

void SupperwareInterface::setListener(Listener* l)
{
    listener = l;
}

void SupperwareInterface::timerCallback()
{
    stopTimer();
    if (doTimer) {
        juce::MessageManagerLock mml;
        doTimer = false;
    }
}

void SupperwareInterface::trackerChanged(const HeadMatrix& headMatrix)
{
    // headMatrix.transform and headMatrix.transformTranspose can be used here
    // to rotate an object.
}
