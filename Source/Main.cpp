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
#include "M1OrientationManagerService.h"

#if defined(JUCE_WINDOWS)

#include <Windows.h>
#include <iostream>
#include <thread>

SERVICE_STATUS          g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE   g_StatusHandle = NULL;
HANDLE                  g_ServiceStopEvent = INVALID_HANDLE_VALUE;

// Service control handler function
VOID WINAPI ServiceControlHandler(DWORD dwControl) {
    switch (dwControl) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        // Report the service status as STOP_PENDING.
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

        juce::MessageManager::getInstance()->stopDispatchLoop();

        // Signal the service to stop.
        SetEvent(g_ServiceStopEvent);
        return;

    default:
        break;
    }
}

// Service entry point
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv) {
    g_StatusHandle = RegisterServiceCtrlHandler("M1-OrientationManager", ServiceControlHandler);
    if (!g_StatusHandle) {
        std::cerr << "RegisterServiceCtrlHandler failed: " << GetLastError() << std::endl;
        return;
    }

    // Report the service status as RUNNING.
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;
    g_ServiceStatus.dwWaitHint = 0;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Create an event to signal the service to stop.
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) {
        // Handle error.
        return;
    }

    // Start a worker thread to perform the service's workand wait for the worker thread to complete.
    std::thread([&] { juce::MessageManager::getInstance()->runDispatchLoop(); }).detach();
    std::thread([]() { M1OrientationManagerService::getInstance().start(); }).join();

    // Cleanup and report the service status as STOPPED.
    CloseHandle(g_ServiceStopEvent);
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwControlsAccepted = 0;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}
#endif // end of windows

class M1OrientationDeviceServerApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    M1OrientationDeviceServerApplication() {
#if defined(JUCE_WINDOWS)
        SERVICE_TABLE_ENTRY ServiceTable[] =
        {
            { "M1-OrientationManager", (LPSERVICE_MAIN_FUNCTION)ServiceMain},
            { NULL, NULL }
        };

        // This is where the service starts.
        if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
            std::cerr << "StartServiceCtrlDispatcher failed: " << GetLastError() << std::endl;
        }
#endif
    }

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return false; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
		std::thread([]() { M1OrientationManagerService::getInstance().start(); }).detach();

#if defined(GUI_APP)
		mainWindow.reset(new MainWindow(getApplicationName()));
#else
		juce::MessageManager::getInstance()->runDispatchLoop();
#endif
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

#if defined(GUI_APP)
        mainWindow = nullptr; // (deletes our window)
#endif
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
        quit();
    }

#if defined(GUI_APP)
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
#endif

private:
#if defined(GUI_APP)
    std::unique_ptr<MainWindow> mainWindow;
#endif

};

//==============================================================================
// This macro generates the main() routine that launches the app.
//==============================================================================

    START_JUCE_APPLICATION (M1OrientationDeviceServerApplication)
