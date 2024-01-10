//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "M1OrientationManager.h"
#include "json/single_include/nlohmann/json.hpp"
#include "httplib/httplib.h"

M1OrientationManager::~M1OrientationManager() {
    close();
}

std::vector<M1OrientationDeviceInfo> M1OrientationManager::getDevices() {
    std::vector<M1OrientationDeviceInfo> devices;
    for (const auto& hardware : hardwareImpl) {
		hardware.second->lock();
        auto devicesToAdd = hardware.second->getDevices();
		hardware.second->unlock();
		devices.insert(devices.end(), devicesToAdd.begin(), devicesToAdd.end());
    }
    return devices;
}

M1OrientationDeviceInfo M1OrientationManager::getConnectedDevice() {
    return currentDevice;
}

bool M1OrientationManager::getTrackingYawEnabled() {
    return bTrackingYawEnabled;
}

bool M1OrientationManager::getTrackingPitchEnabled() {
    return bTrackingPitchEnabled;
}

bool M1OrientationManager::getTrackingRollEnabled() {
    return bTrackingRollEnabled;
}

bool M1OrientationManager::init(int serverPort, int helperPort) {
    
    
    M1Orientation orientation;
    EulerAngleSet originalAngles, retrievedAngles;
    M1Quaternion quat;

    // Test 1: Set and get orientation
    originalAngles = EulerAngleSet(0, 0.5, 0); // Equivalent to (30, -20, 0) in degrees
    std::cout << "Original Angles (rad): " << originalAngles.yaw << ", " << originalAngles.pitch << ", " << originalAngles.roll << std::endl;

    orientation.setFromEulerYPRRadians(originalAngles.yaw, originalAngles.pitch, originalAngles.roll);
    quat = orientation.getAsQuaternion();
    std::cout << "Quaternion: " << quat.w << ", " << quat.x << ", " << quat.y << ", " << quat.z << std::endl;

    orientation.setFromQuaternion(quat);
    // Returning angles as signed and non normalized
    retrievedAngles = orientation.getAsEulerYPRRadians(true);
    std::cout << "Retrieved Angles (rad): " << retrievedAngles.yaw << ", " << retrievedAngles.pitch << ", " << retrievedAngles.roll << std::endl;

    if (std::abs(originalAngles.yaw - retrievedAngles.yaw) < 1e-4 &&
        std::abs(originalAngles.pitch - retrievedAngles.pitch) < 1e-4 &&
        std::abs(originalAngles.roll - retrievedAngles.roll) < 1e-4) {
        std::cout << "Test 1 success\n";
    } else {
        std::cout << "Test 1 fail\n";
    }

    // Test 2: Concatenate two orientations
    M1Orientation orientation1, orientation2, resultOrientation;
//    EulerAngleSet angles1(5, -20, 0), angles2(5, 45, 15), resultAngles;
    EulerAngleSet angles1(0.5, -0.5, 0), angles2(0, 0.7, 0), resultAngles;
    orientation1.setFromEulerYPRRadians(angles1.yaw, angles1.pitch, angles1.roll);
//
//    orientation1.setFromEulerYPRDegrees(angles1.yaw, angles1.pitch, angles1.roll);
    auto ori1_rad = orientation1.getAsEulerYPRRadians(true);
    std::cout << "Orientation 1 (rad): " << ori1_rad.yaw << ", " << ori1_rad.pitch << ", " << ori1_rad.roll << std::endl;

//    orientation2.setFromEulerYPRDegrees(angles2.yaw, angles2.pitch, angles2.roll);

    orientation2.setFromEulerYPRRadians(angles2.yaw, angles2.pitch, angles2.roll);
    auto ori2_rad = orientation2.getAsEulerYPRRadians(true);
    std::cout << "Orientation 1 (rad): " << ori2_rad.yaw << ", " << ori2_rad.pitch << ", " << ori2_rad.roll << std::endl;

    resultOrientation = orientation1 + orientation2;
    auto ori_r_rad = resultOrientation.getAsEulerYPRRadians(true);
    std::cout << "Orientation combined (rad): " << ori_r_rad.yaw << ", " << ori_r_rad.pitch << ", " << ori_r_rad.roll << std::endl;
    
    resultAngles = resultOrientation.getAsEulerYPRRadians(true);

    std::cout << "Angles 1 (rad): " << angles1.yaw << ", " << angles1.pitch << ", " << angles1.roll << std::endl;
    std::cout << "Angles 2 (rad): " << angles2.yaw << ", " << angles2.pitch << ", " << angles2.roll << std::endl;
    std::cout << "Result Angles (rad): " << resultAngles.yaw << ", " << resultAngles.pitch << ", " << resultAngles.roll << std::endl;

    if (std::abs(resultAngles.yaw - (angles1.yaw + angles2.yaw)) < 1e-4 &&
        std::abs(resultAngles.pitch - (angles1.pitch + angles2.pitch)) < 1e-4 &&
        std::abs(resultAngles.roll - (angles1.roll + angles2.roll)) < 1e-4) {
        std::cout << "Test 2 success\n";
    } else {
        std::cout << "Test 2 fail\n";
    }
    
    
    // Test 3: converting radians to euler back and forth
    
    auto angles1_deg = orientation1.getAsEulerYPRDegrees();
    orientation1.setFromEulerYPRDegrees(angles1_deg.yaw, angles1_deg.pitch, angles1_deg.roll);
    auto angles1_2_rad = orientation1.getAsEulerYPRRadians();
    if (std::abs(angles1_2_rad.yaw - angles1.yaw) < 1e-4 &&
        std::abs(angles1_2_rad.pitch - angles1.pitch) < 1e-4 &&
        std::abs(angles1_2_rad.roll - angles1.roll) < 1e-4) {
        std::cout << "Test 3 success\n";
    } else {
        std::cout << "Test 3 fail\n";
    }


	// check the port
    this->serverPort = serverPort;
	std::thread([&]() {
	
		server.Get("/ping", [&](const httplib::Request &req, httplib::Response &res) {
			mutex.lock();
			res.set_content(stringForClient, "text/plain");
			mutex.unlock();
			}
		);

		server.Post("/startTrackingUsingDevice", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(std::string(data, data_length));
				M1OrientationDeviceInfo device = { (std::string)j.at(0), (M1OrientationDeviceType)j.at(1), (std::string)j.at(2) };
				command_startTrackingUsingDevice(device);
				return true;
				}
			);
			}
		);

		server.Post("/setTrackingYawEnabled", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(std::string(data, data_length));
				bool enable = j.at(0);
				command_setTrackingYawEnabled(enable);
				return true;
				}
			);
			}
		);

		server.Post("/setTrackingPitchEnabled", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(std::string(data, data_length));
				bool enable = j.at(0);
				command_setTrackingPitchEnabled(enable);
				return true;
				}
			);
			}
		);

		server.Post("/setTrackingRollEnabled", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(std::string(data, data_length));
				bool enable = j.at(0);
				command_setTrackingRollEnabled(enable);
				return true;
				}
			);
			}
		);
        
        server.Post("/setDeviceSettings", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
            content_reader([&](const char *data, size_t data_length) {
                auto j = nlohmann::json::parse(std::string(data, data_length));
                std::string new_settings = j.at(0);
                command_updateDeviceSettings(new_settings);
                return true;
                }
            );
            }
        );

		server.Post("/recenter", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			command_recenter();
			}
		);

        server.Post("/devicesrefresh", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
            command_refresh();
            }
        );
        
		server.Post("/disconnect", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			command_disconnect();
			}
		);

		server.Post("/setPlayerPosition", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(std::string(data, data_length));
				playerPositionInSeconds = j.at(0);
				playerLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				return true;
				}
			);
			}
		);

		server.Post("/setPlayerIsPlaying", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(std::string(data, data_length));
				playerIsPlaying = j.at(0);
				playerLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
				return true;
				}
			);
			}
		);
		server.listen("localhost", this->serverPort);
	}).detach();

	return true;
    
}

void M1OrientationManager::startSearchingForDevices() {
	std::thread([&] {
		while (true) {
            if (isDevicesRefreshRequested) {
                for (const auto& v : hardwareImpl) {
                    v.second->refreshDevices();
                }
                isDevicesRefreshRequested = false;
            }
            std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}).detach();
}

void M1OrientationManager::update() {
	std::vector<M1OrientationDeviceInfo> devices = getDevices();
    EulerAngleSet ypr;

    if (currentDevice.getDeviceType() != M1OrientationManagerDeviceTypeNone) {
        if (!hardwareImpl[currentDevice.getDeviceType()]->update()) {
            /// ERROR STATE
            // TODO: Check for connection to client, if not then reconnect
            // TODO: if reconnect does not work then error that client is no longer available
            // if (client still exists){
                // TODO: Check if connected, if not then reconnect
                command_startTrackingUsingDevice(currentDevice);
            //}
        }

        // update orientation
        if (currentDevice.getDeviceType() != M1OrientationManagerDeviceTypeNone) {
            ypr = hardwareImpl[currentDevice.getDeviceType()]->getOrientation().currentOrientation.getAsEulerYPRNormalized(true);
//            ypr.angleType = M1OrientationYPR::SIGNED_NORMALLED;
            if (!getTrackingYawEnabled()) ypr.yaw = 0.0;
            if (!getTrackingPitchEnabled()) ypr.pitch = 0.0;
            if (!getTrackingRollEnabled()) ypr.roll = 0.0;
            
            // commented out to avoid double applying offset angles from the get()
            orientation.setFromEulerYPRRadians(ypr.yaw, ypr.pitch, ypr.roll);
        }
	}

	nlohmann::json j;
	j["devices"] = nlohmann::json::array();

	for (auto& device : devices) {
		bool hasStrength = std::holds_alternative<int>(device.getDeviceSignalStrength());

		j["devices"].push_back({
			device.getDeviceName(),
			device.getDeviceType(),
			device.getDeviceAddress(),
			hasStrength,
			hasStrength ? std::get<int>(device.getDeviceSignalStrength()) : 0,
			});
	}

	auto it = std::find(devices.begin(), devices.end(), currentDevice);
	if (it != devices.end()) {
		j["currentDeviceIdx"] = std::distance(devices.begin(), it);
	}
	else {
		j["currentDeviceIdx"] = -1;
	}
    
    M1Orientation finalOrientation = orientation + offset;
    EulerAngleSet finalOrientationEuler = finalOrientation.getAsEulerYPRRadians();

	j["trackingEnabled"] = { bTrackingYawEnabled, bTrackingPitchEnabled, bTrackingRollEnabled };
	j["orientation"] = { finalOrientationEuler.yaw,
                         finalOrientationEuler.pitch,
                         finalOrientationEuler.roll };

	j["player"]["frameRate"] = playerFrameRate;
	j["player"]["positionInSeconds"] = playerPositionInSeconds;
	j["player"]["isPlaying"] = playerIsPlaying;
	j["player"]["lastUpdate"] = playerLastUpdate;

	mutex.lock();
	stringForClient = j.dump();
	mutex.unlock();
}

M1Orientation M1OrientationManager::getOrientation() {
    return orientation;
}

void M1OrientationManager::addHardwareImplementation(M1OrientationDeviceType type, HardwareAbstract* impl) {
    hardwareImpl[type] = impl;
}

void M1OrientationManager::close() {
    isRunning = false;
	server.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void M1OrientationManager::command_disconnect() {
    orientation.resetOrientation();
    if (currentDevice.getDeviceType() != M1OrientationManagerDeviceTypeNone) {
		hardwareImpl[currentDevice.getDeviceType()]->lock();
		hardwareImpl[currentDevice.getDeviceType()]->close();
		hardwareImpl[currentDevice.getDeviceType()]->unlock();
		currentDevice = M1OrientationDeviceInfo();
    }
}

void M1OrientationManager::command_startTrackingUsingDevice(M1OrientationDeviceInfo device) {
    orientation.resetOrientation();
    if (currentDevice != device){
		hardwareImpl[device.getDeviceType()]->lock();
        hardwareImpl[device.getDeviceType()]->startTrackingUsingDevice(device, [&](bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress) {
            if (success) {
                currentDevice = device;
            }
        });
		hardwareImpl[device.getDeviceType()]->unlock();
	} else {
        // already connected to this device
    }
}

void M1OrientationManager::command_setTrackingYawEnabled(bool enable) {
    bTrackingYawEnabled = enable;
}

void M1OrientationManager::command_setTrackingPitchEnabled(bool enable) {
    bTrackingPitchEnabled = enable;
}

void M1OrientationManager::command_setTrackingRollEnabled(bool enable) {
    bTrackingRollEnabled = enable;
}

void M1OrientationManager::command_recenter() {
    offset = orientation;
    orientation.resetOrientation();
    
    DBG("[REQ] Recenter requested from a client...");
}

void M1OrientationManager::command_refresh() {
    isDevicesRefreshRequested = true;
    DBG("[REQ] Refresh requested from a client...");
}

bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void M1OrientationManager::command_updateDeviceSettings(std::string additional_settings) {
    // This function interprets device specific settings from a string blob
    
    // OSC Device Settings
    if (currentDevice.getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeOSC) {
        // search the start of the string for a known setting key
        if (additional_settings.rfind("osc_add=", 0) == 0) {
            std::string new_msg_address_pattern;
            new_msg_address_pattern = additional_settings.substr(additional_settings.find("osc_add=") + std::string("osc_add=").size());
            DBG("Setting Update: " + new_msg_address_pattern);

            // requires a starting '/' char within the received address pattern string
            if (currentDevice.osc_msg_addr_pttrn != new_msg_address_pattern) {
                auto devices = getDevices();
                for (int i = 0; i < devices.size(); i++) {
                    if (currentDevice == devices[i]) {
                        // update custom message pattern
                        devices[i].osc_msg_addr_pttrn = new_msg_address_pattern;
                        
                    }
                }
            }

        } else
        if (additional_settings.rfind("osc_p=", 0) == 0) {
            std::string new_port;
            new_port = additional_settings.substr(additional_settings.find("osc_p=") + std::string("osc_p=").size());
            DBG("Setting Update: " + new_port);

            if (is_number(new_port)) {
                int new_parsed_port = stoi(new_port);
                if (currentDevice.osc_port != new_parsed_port) {
                    auto devices = getDevices();
                    for (int i = 0; i < devices.size(); i++) {
                        if (currentDevice == devices[i]) {
                            // update port
                            devices[i].osc_port = new_parsed_port;
                            
                            // reconnect
                            command_disconnect();
                            command_startTrackingUsingDevice(devices[i]);
                        }
                    }
                }
            }
        }
    }

    // Supperware Device Settings
    if (currentDevice.getDeviceName().find("Supperware HT IMU") != std::string::npos) {
        if (additional_settings.rfind("sw_chir=", 0) == 0) {
            std::string new_sw_chirality;
            new_sw_chirality = additional_settings.substr(additional_settings.find("sw_chir=") + std::string("sw_chir=").size());
            DBG("Setting Update: Supperware Right Side Chirality = " + new_sw_chirality);
            
            // Expects the bool values sent via the command_updateDeviceSettings to be '0' or '1'
            if ((bool)stoi(new_sw_chirality) == 0) {
                // TODO: call supperwareInterface.setChirality(false)
            } else if ((bool)stoi(new_sw_chirality) == 1) {
                // TODO: call supperwareInterface.setChirality(true)
            }
            
            // TODO: parse calibrate
        }
    }
}
