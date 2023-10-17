//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//
#pragma once

#include <JuceHeader.h>
#include <thread>

#include "HardwareBLE.h"
#include "HardwareSerial.h"
#include "HardwareOSC.h"
#include "HardwareEmulator.h"

#include "M1OrientationOSCServer.h"

//==============================================================================
class M1OrientationService {
	std::mutex mtx;

public:
	static M1OrientationService& getInstance() {
		static M1OrientationService instance; // Singleton instance
		return instance;
	}

	M1OrientationOSCServer m1OrientationOSCServer;

	void lock() {
		mtx.lock();
	}

	void unlock() {
		mtx.unlock();
	}

	void start() {
		HardwareBLE hardwareBLE;
		HardwareSerial hardwareSerial;
		HardwareOSC hardwareOSC;
		HardwareEmulator hardwareEmulator;

		// We will assume the folders are properly created during the installation step
		// TODO: make this file path search for `Mach1` dir
		// Using common support files installation location
		juce::File m1SupportDirectory = juce::File::getSpecialLocation(juce::File::commonApplicationDataDirectory);
		// We will assume the folders are properly created during the installation step
		juce::File settingsFile;
		if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
			// test for any macOS
			settingsFile = m1SupportDirectory.getChildFile("Application Support").getChildFile("Mach1");
		}
		else if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::Windows) != 0) {
			// test for any Windows OS
			settingsFile = m1SupportDirectory.getChildFile("Mach1");
		}
		else {
			settingsFile = m1SupportDirectory.getChildFile("Mach1");
		}
		settingsFile = settingsFile.getChildFile("settings.json");
		DBG("Opening settings file: " + settingsFile.getFullPathName().quoted());

		if (m1OrientationOSCServer.initFromSettings(settingsFile.getFullPathName().toStdString(), true)) {
			// For debug testing, you can set this to false to list all connectable BLE devices
			hardwareBLE.displayOnlyKnownIMUs = true;
			hardwareBLE.setup();
			hardwareSerial.setup();
			hardwareOSC.setup();
			// Internal device emulator for debugging
			hardwareEmulator.setup();

			m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &hardwareBLE);
			m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeSerial, &hardwareSerial);
			m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeOSC, &hardwareOSC);
			m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeEmulator, &hardwareEmulator);

			while (!juce::MessageManager::getInstance()->hasStopMessageBeenSent()) {
				lock();
				m1OrientationOSCServer.update();
				unlock();

				juce::Thread::sleep(30);
			}

		}
	}

};
