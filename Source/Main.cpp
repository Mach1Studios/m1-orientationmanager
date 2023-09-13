//
//  m1-orientationmanager
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
class M1OrientationDeviceServerApplication  : public juce::JUCEApplication, private juce::Timer
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
        // We will assume the folders are properly created during the installation step
        // TODO: make this file path search for `Mach1` dir
        // Using common support files installation location
        juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);
        std::string settingsFilePath;
        if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
            // test for any windows OS
            settingsFilePath = (m1SupportDirectory.getFullPathName()+"/Mach1/settings.json").toStdString();
        } else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
            // test for any mac OS
            settingsFilePath = (m1SupportDirectory.getFullPathName()+"/Application Support/Mach1/settings.json").toStdString();
        } else {
            settingsFilePath = (m1SupportDirectory.getFullPathName()+"/Mach1/settings.json").toStdString();
        }

        if (m1OrientationOSCServer.initFromSettings(settingsFilePath, false)) {
            hardwareBLE.displayOnlyKnownIMUs = true;
            hardwareBLE.setup(); // move this to its own thread
            hardwareSerial.setup(); // move this to its own thread
            hardwareOSC.setup(); // move this to its own thread
            m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &hardwareBLE);
            m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeSerial, &hardwareSerial);
            m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeOSC, &hardwareOSC);
            
            // emulator for debug
            hardwareEmulator.setup();
            m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeEmulator, &hardwareEmulator);
        }

        // starts the update loop
        startTimer(15);
        
        if (JUCEApplicationBase::getCommandLineParameterArray().indexOf("--no-gui") >= 0) {
            /// WITHOUT GUI

        } else {
            /// WITH GUI
            mainWindow.reset(new MainWindow(getApplicationName(), &m1OrientationOSCServer));
        }
    }
    
    void timerCallback() override
    {
        // update loop for threaded device server
        m1OrientationOSCServer.update();
    }

    void shutdown() override
    {
        stopTimer();
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
        MainWindow (juce::String name, M1OrientationOSCServer* server)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            m1OrientationOSCServer = server;
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(server), true);

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
        M1OrientationOSCServer* m1OrientationOSCServer;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
public:
    HardwareBLE hardwareBLE;
    HardwareSerial hardwareSerial;
    HardwareOSC hardwareOSC;
    HardwareEmulator hardwareEmulator; // debug
    M1OrientationOSCServer m1OrientationOSCServer;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (M1OrientationDeviceServerApplication)
