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

void killProcessByName(const char *name)
{
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

        // run process M1-OrientationManager.exe from the same folder
        juce::ChildProcess process;

#ifdef JUCE_WINDOWS
        juce::File exeFile = juce::File::getCurrentWorkingDirectory().getChildFile("M1-OrientationManager.exe");
#else
        juce::File exeFile = juce::File::getCurrentWorkingDirectory().getChildFile("M1-OrientationManager");
#endif

		juce::StringArray arguments;
		arguments.add(exeFile.getFullPathName());
		//arguments.add("--no-gui");

		if (!process.start(arguments)) {
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
    juce::uint32 time = 0;

public:
    M1OrientationManagerWatcherApplication() {}
    ~M1OrientationManagerWatcherApplication() {}

    void oscMessageReceived(const juce::OSCMessage& message) override
    {
        time = juce::Time::currentTimeMillis();
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
            time = juce::Time::currentTimeMillis();

            startTimer(100);
        }
    }

    void shutdown() override
    {
        stopTimer();

        receiver.removeListener(this);
        receiver.disconnect();
    }

    const juce::String getApplicationName() override
    {
        return "M1OrientationManagerWatcher";
    }

    const juce::String getApplicationVersion() override
    {
        return "1.0";
    }

    void timerCallback() override
    {
        juce::uint32 currentTime = juce::Time::currentTimeMillis();
        DBG("time: " + std::to_string(currentTime - time));
        if (currentTime - time > 2000) {
            killProcessByName("M1-OrientationManager");
            time = currentTime;
            startOrientationManager();
        }
    }
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(M1OrientationManagerWatcherApplication)

