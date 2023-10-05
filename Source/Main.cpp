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
void M1OrientationService() {
	HardwareBLE hardwareBLE;
	HardwareSerial hardwareSerial;
	HardwareOSC hardwareOSC;
	M1OrientationOSCServer m1OrientationOSCServer;

	// We will assume the folders are properly created during the installation step
	// TODO: make this file path search for `Mach1` dir
	// Using common support files installation location
	juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);
	std::string settingsFilePath;
	if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
		// test for any windows OS
		settingsFilePath = (m1SupportDirectory.getFullPathName() + "/Mach1/settings.json").toStdString();
	}
	else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
		// test for any mac OS
		settingsFilePath = (m1SupportDirectory.getFullPathName() + "/Application Support/Mach1/settings.json").toStdString();
	}
	else {
		settingsFilePath = (m1SupportDirectory.getFullPathName() + "/Mach1/settings.json").toStdString();
	}

	if (m1OrientationOSCServer.initFromSettings(settingsFilePath, true)) {
		hardwareBLE.displayOnlyKnownIMUs = true;
		hardwareBLE.setup();
		hardwareSerial.setup();
		hardwareOSC.setup();
		m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &hardwareBLE);
		m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeSerial, &hardwareSerial);
		m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeOSC, &hardwareOSC);

		while (!juce::MessageManager::getInstance()->hasStopMessageBeenSent()) {
			m1OrientationOSCServer.update();
			juce::Thread::sleep(30);
		}
	}
}
 
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
		if (!JUCEApplicationBase::getCommandLineParameterArray().indexOf("--no-gui") >= 0) {
			std::thread(M1OrientationService).detach();
			juce::MessageManager::getInstance()->runDispatchLoop();
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
#if defined(SERVICE) && defined(JUCE_WINDOWS)

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
	g_StatusHandle = RegisterServiceCtrlHandler("HelloWorldService", ServiceControlHandler);
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
	std::thread(M1OrientationService).join();

	// Cleanup and report the service status as STOPPED.
	CloseHandle(g_ServiceStopEvent);
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwControlsAccepted = 0;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}
 
int main(int argc, char* argv[]) {
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ "HelloWorldService", (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{ NULL, NULL }
	};

	// This is where the service starts.
	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
		std::cerr << "StartServiceCtrlDispatcher failed: " << GetLastError() << std::endl;
	}

	return 0;
}

#elif defined(SERVICE) && defined(JUCE_MAC)

#include <iostream>
#include <signal.h>
#include <unistd.h>

// Function to handle signals for service termination
void signalHandler(int signum) {
	if (signum == SIGTERM || signum == SIGINT) {
		juce::MessageManager::getInstance()->stopDispatchLoop();
	}
}

int main(int argc, char* argv[]) {
	// Set up signal handlers to gracefully handle service termination
	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);

	// Initialize your service here
	std::cout << "Service is starting..." << std::endl;

	// Start a worker thread to perform the service's workand wait for the worker thread to complete.
	std::thread([&] { juce::MessageManager::getInstance()->runDispatchLoop(); }).detach();
	std::thread(M1OrientationService).join();

	return 0;
}

#else
	START_JUCE_APPLICATION (M1OrientationDeviceServerApplication)
#endif