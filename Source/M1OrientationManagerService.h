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

#include "M1OrientationManager.h"

//==============================================================================
class M1OrientationManagerService {
	std::mutex mtx;

public:
	static M1OrientationManagerService& getInstance() {
		static M1OrientationManagerService instance; // Singleton instance
		return instance;
	}

	M1OrientationManager m1OrientationManager;

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

		if (m1OrientationManager.initFromSettings(settingsFile.getFullPathName().toStdString())) {
			// For debug testing, you can set this to false to list all connectable BLE devices
			// hardwareBLE.displayOnlyKnownIMUs = true; // TODO: Reimplement with new scheme
			hardwareBLE.setup();
			hardwareSerial.setup();
			hardwareOSC.setup();
			// Internal device emulator for debugging
			hardwareEmulator.setup();

			m1OrientationManager.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &hardwareBLE);
			m1OrientationManager.addHardwareImplementation(M1OrientationManagerDeviceTypeSerial, &hardwareSerial);
			m1OrientationManager.addHardwareImplementation(M1OrientationManagerDeviceTypeOSC, &hardwareOSC);
			m1OrientationManager.addHardwareImplementation(M1OrientationManagerDeviceTypeEmulator, &hardwareEmulator);

			m1OrientationManager.startSearchingForDevices();

			while (!juce::MessageManager::getInstance()->hasStopMessageBeenSent()) {
				m1OrientationManager.update();

				juce::Thread::sleep(10);
			}

		}
	}

};
