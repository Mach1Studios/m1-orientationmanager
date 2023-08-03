/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/
#include <JuceHeader.h>

#ifndef JUCE_WINDOWS
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

int serverPort = 6345;
int watcherPort = 6346;

juce::int64 pingTime = 0;

// run process M1-OrientationManager.exe from the same folder
juce::ChildProcess orientationManagerProcess;

void killProcessByName(const char *name)
{
    DBG("Killing M1-Orientation-Server...");
#ifdef JUCE_WINDOWS
	std::string command = "taskkill /IM " + std::string(name) + " /F";
	system(command.c_str());
#else
    std::string command = "pkill " + std::string(name);
    system(command.c_str());
#endif
}

void startOrientationManager()
{
    // Create a DatagramSocket to check the availability of port serverPort
    juce::DatagramSocket socket(false);
    socket.setEnablePortReuse(false);
    if (socket.bindToPort(serverPort)) {
        socket.shutdown();

		juce::File executableFile = juce::File::getSpecialLocation(juce::File::invokedExecutableFile);
        juce::File appDirectory = executableFile.getParentDirectory();

#ifdef JUCE_WINDOWS
		juce::File exeFile = appDirectory.getChildFile("M1-OrientationManager.exe");
#else
		juce::File exeFile = appDirectory.getChildFile("M1-OrientationManager");
#endif
        appDirectory.setAsCurrentWorkingDirectory();

        DBG("appDirectory...");
        DBG(appDirectory.getFullPathName());

		juce::StringArray arguments;
		arguments.add(exeFile.getFullPathName());
		arguments.add("--no-gui");

        DBG("Starting M1-OrientationManager-Watcher...");
        DBG(exeFile.getFullPathName());

		if (orientationManagerProcess.start(arguments)) {
            DBG("Started M1-OrientationManager server");
        } else {
            // Failed to start the process
            DBG("Failed to start the M1-OrientationManager");
            exit(1);
        }
	}
}

//==============================================================================
class M1OrientationManagerWatcherApplication : public juce::JUCEApplication,
    private juce::Timer,
    public juce::OSCReceiver::Listener<juce::OSCReceiver::RealtimeCallback>
{
    juce::OSCReceiver receiver;

public:
    M1OrientationManagerWatcherApplication() {}
    ~M1OrientationManagerWatcherApplication() {}

    void oscMessageReceived(const juce::OSCMessage& message) override
    {
        pingTime = juce::Time::currentTimeMillis();
        DBG("Received message from " + message.getAddressPattern().toString());
    }

    void initialise(const juce::String&) override
    {
        juce::DatagramSocket socket(false); 
        socket.setEnablePortReuse(false);
        if (!socket.bindToPort(watcherPort)) {
            socket.shutdown();

            juce::String message = "Failed to bind to port " + std::to_string(watcherPort);
            juce::MessageManager::getInstance()->callFunctionOnMessageThread([](void* m) -> void* {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Alert Box",
                    static_cast<juce::String*>(m)->toRawUTF8(),
                    "OK",
                    nullptr,  // No associated component
                    juce::ModalCallbackFunction::create([m](int result) {
                        quit();
                    })
                );
                return nullptr;
                }, &message);
            DBG(message);
        }
        else {
            socket.shutdown();

            receiver.connect(watcherPort);
            receiver.addListener(this);

            startOrientationManager();
            pingTime = juce::Time::currentTimeMillis();

            startTimer(100);
        }
    }

    void shutdown() override
    {
        stopTimer();
        receiver.removeListener(this);
        receiver.disconnect();
        DBG("M1-OrientationManager-Watcher is shutting down...");
    }

    const juce::String getApplicationName() override
    {
        return ProjectInfo::projectName;
    }

    const juce::String getApplicationVersion() override
    {
        return ProjectInfo::versionString;
    }
    
    bool moreThanOneInstanceAllowed() override
    {
        return false;
    }

    void timerCallback() override
    {
        juce::int64 currentTime = juce::Time::currentTimeMillis();
        DBG("time: " + std::to_string(currentTime - pingTime));
        if (currentTime > pingTime && currentTime - pingTime > 1000) {
            pingTime = juce::Time::currentTimeMillis() + 10000; // wait 10 seconds

            killProcessByName("M1-OrientationManager");
            startOrientationManager();
        }
    }
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(M1OrientationManagerWatcherApplication)
