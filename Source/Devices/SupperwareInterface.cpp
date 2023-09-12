//
//  m1-orientationmanager
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
        //if (listener) listener->trackerChanged(headMatrix);
    }
}

//----------------------------------------------------------------------

void SupperwareInterface::trackerOrientation(float yawRadian, float pitchRadian, float rollRadian)
{
    headMatrix.setOrientationYPR(yawRadian, pitchRadian, rollRadian);
    //if (listener) listener->trackerChanged(headMatrix);
    
    //update public orientation
    currentOrientation.clear();
    currentOrientation.push_back(juce::radiansToDegrees(yawRadian));
    currentOrientation.push_back(juce::radiansToDegrees(pitchRadian));
    currentOrientation.push_back(juce::radiansToDegrees(rollRadian));
}

void SupperwareInterface::trackerOrientationQ(float qw, float qx, float qy, float qz)
{
    headMatrix.setOrientationQuaternion(qw, qx, qy, qz);
    //if (listener) listener->trackerChanged(headMatrix);
    
    //update public orientation
    currentOrientation.clear();
    currentOrientation.push_back(qw);
    currentOrientation.push_back(qx);
    currentOrientation.push_back(qy);
    currentOrientation.push_back(qz);
}

void SupperwareInterface::trackerZero()
{
    trackerDriver.zero();
}

//----------------------------------------------------------------------

void SupperwareInterface::connectSupperware()
{
    // connect/disconnect
    if (midiState == Midi::State::Available || midiState == Midi::State::Bootloader) {
        trackerDriver.connect();
        trackerDriver.turnOn(true, true);
    } else if (midiState == Midi::State::Connected) {
        trackerDriver.turnOn(true, true);
    } else {
        // Error: not available
    }
}

void SupperwareInterface::setListener(Listener* l)
{
    listener = l;
}

void SupperwareInterface::timerCallback()
{
}
