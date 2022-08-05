//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include <JuceHeader.h>
#include "SupperwareInterface.h"

//==============================================================================
SupperwareInterface::SupperwareInterface()
{
    headPanel.setListener(this);
    headPanel.setTopLeftPosition(8, 8);
    addAndMakeVisible(headPanel);
    setSize(headPanel.getWidth()+16, headPanel.getHeight()+16);
}

SupperwareInterface::~SupperwareInterface()
{
}

void SupperwareInterface::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    // Note that the default JUCE look and feel colour is used here. The component works
    // best against dark backgrounds such as this: there's no 'light mode' at the moment.
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SupperwareInterface::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void SupperwareInterface::trackerChanged(const HeadMatrix& headMatrix)
{
    // headMatrix.transform and headMatrix.transformTranspose can be used here
    // to rotate an object.
}
