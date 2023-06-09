//
//  M1-OrientationManager
//  Copyright © 2022 Mach1. All rights reserved.
//

/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include <thread>
#include "MainComponent.h"

//==============================================================================
class M1OrientationDeviceServerApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    M1OrientationDeviceServerApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
       if (JUCEApplicationBase::getCommandLineParameterArray().indexOf("--no-gui") >= 0) {
            HardwareBLE hardwareBLE;
            HardwareSerial hardwareSerial;
            HardwareOSC hardwareOSC;
            M1OrientationOSCServer m1OrientationOSCServer;

            std::string settingsFilePath = (juce::File::getCurrentWorkingDirectory().getFullPathName() + "/settings.json").toStdString();
            if (m1OrientationOSCServer.initFromSettings(settingsFilePath)) {
                hardwareBLE.displayOnlyKnownIMUs = true;
                hardwareBLE.setup();
                hardwareSerial.setup();
                hardwareOSC.setup();
                m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &hardwareBLE);
                m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeSerial, &hardwareSerial);
                m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeOSC, &hardwareOSC);
                while (true) {
                    m1OrientationOSCServer.update();
                    juce::Thread::sleep(30);
                }
            }
        } else {
            mainWindow.reset(new MainWindow(getApplicationName()));
        }
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (M1OrientationDeviceServerApplication)
