//
//  m1-orientationmanager
//  Copyright © 2022 Mach1. All rights reserved.
//

#include "M1OrientationManager.h"
#include "nlohmann/json.hpp"
#include "httplib.h"

void M1OrientationManager::oscMessageReceived(const juce::OSCMessage& message) {
     
}
 
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

bool M1OrientationManager::init(int serverPort, int watcherPort, bool useWatcher = false) {
	// check the port

	//http://localhost:8088/ping
	std::thread([&]() {
	
		server.Get("/ping", [&](const httplib::Request &req, httplib::Response &res) {
			mutex.lock();
			res.set_content(stringForClient, "text/plain");
			mutex.unlock();
			}
		);

		server.Post("/startTrackingUsingDevice", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				M1OrientationDeviceInfo device = { (std::string)j.at(0), (M1OrientationDeviceType)j.at(1), (std::string)j.at(2) };
				command_startTrackingUsingDevice(device);
				return true;
				}
			);
			}
		);

		server.Post("/setTrackingYawEnabled", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				bool enable = j.at(0);
				command_setTrackingYawEnabled(enable);
				return true;
				}
			);
			}
		);

		server.Post("/setTrackingPitchEnabled", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				bool enable = j.at(0);
				command_setTrackingPitchEnabled(enable);
				return true;
				}
			);
			}
		);

		server.Post("/setTrackingRollEnabled", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				bool enable = j.at(0);
				command_setTrackingRollEnabled(enable);
				return true;
				}
			);
			}
		);

		server.Post("/setOscDeviceSettings", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				int new_port = j.at(0);
				std::string new_pttrn = j.at(1);
				command_updateOscDevice(new_port, new_pttrn);
				return true;
				}
			);
			}
		);

		server.Post("/recenter", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			command_recenter();
			}
		);

		server.Post("/disconnect", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			command_disconnect();
			}
		);

		server.Post("/setMonitoringMode", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			// receiving updated monitoring mode or other misc settings for clients
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				master_mode = j.at(0);
				DBG("[Monitor] Mode: " + std::to_string(master_mode));
				return true;
				}
			);
			}
		);

		server.Post("/setOffsetYPR", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			// receiving updated client YPR
			// Note: It is expected that the orientation manager receives orientation and sends it to a client and for the client to offset this orientation before sending it back to registered plugins, the adding of all orientations should happen on client side only
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				int client_id = j.at(0); // use the client port to id the client
				client_offset_ypr[client_id][0] = j.at(1); // yaw
				client_offset_ypr[client_id][1] = j.at(2); // pitch
				client_offset_ypr[client_id][2] = j.at(3); // roll
				DBG("[Client] YPR=" + std::to_string(client_offset_ypr[client_id][0]) + ", " + std::to_string(client_offset_ypr[client_id][1]) + ", " + std::to_string(client_offset_ypr[client_id][2]));
				return true;
				}
			);
			}
		);

		server.Post("/setMasterYPR", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			// Used for relaying a master calculated orientation to registered plugins that require this for GUI systems
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				master_yaw = j.at(0);
				master_pitch = j.at(1);
				master_roll = j.at(2);
				return true;
				}
			);
			}
		);

		server.Post("/m1-register-plugin", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			// registering new panner instance
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				auto port = (int)j.at(0);
				// protect port creation to only messages from registered plugin (example: an m1-panner)
				if (std::find_if(registeredPlugins.begin(), registeredPlugins.end(), find_plugin(port)) == registeredPlugins.end()) {
					M1RegisteredPlugin foundPlugin;
					foundPlugin.port = port;
					foundPlugin.messageSender = new juce::OSCSender();
					foundPlugin.messageSender->connect("127.0.0.1", port); // connect to that newly discovered panner locally
					registeredPlugins.push_back(foundPlugin);
					DBG("Plugin registered: " + std::to_string(port));
				}
				else {
					DBG("Plugin port already registered: " + std::to_string(port));
				}
				if (!bTimerActive && registeredPlugins.size() > 0) {
					startTimer(60);
					bTimerActive = true;
				}
				else {
					if (registeredPlugins.size() == 0) {
						// TODO: setup logic for deleting from `registeredPlugins`
						stopTimer();
						bTimerActive = false;
					}
				}
				return true;
				}
			);
			}
		);

		server.Post("/panner-settings", [&](const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader) {
			content_reader([&](const char *data, size_t data_length) {
				auto j = nlohmann::json::parse(data);
				if (j.size() > 0) { // check message size
					auto plugin_port = (int)j.at(0);
					if (j.size() == 6) {
						auto input_mode = (int)j.at(1);
						auto azi = (float)j.at(2);
						auto ele = (float)j.at(3);
						auto div = (float)j.at(4);
						auto gain = (float)j.at(5);
						DBG("[OSC] Panner: port=" + std::to_string(plugin_port) + ", in=" + std::to_string(input_mode) + ", az=" + std::to_string(azi) + ", el=" + std::to_string(ele) + ", di=" + std::to_string(div) + ", gain=" + std::to_string(gain));
						// Check if port matches expected registered-plugin port
						if (registeredPlugins.size() > 0) {
							auto it = std::find_if(registeredPlugins.begin(), registeredPlugins.end(), find_plugin(plugin_port));
							auto index = it - registeredPlugins.begin(); // find the index from the found plugin
							registeredPlugins[index].isPannerPlugin = true;
							registeredPlugins[index].input_mode = input_mode;
							registeredPlugins[index].azimuth = azi;
							registeredPlugins[index].elevation = ele;
							registeredPlugins[index].diverge = div;
							registeredPlugins[index].gain = gain;
						}
					}
				}
				else {
					// port not found, error here
				}

				return true;
				}
			);
			}
		);


		server.listen("localhost", 8088);
	}).detach();

	return true;
    
}

void M1OrientationManager::startSearchingForDevices() {
	std::thread([&] {
		while (isRunning) {
			for (const auto& v : hardwareImpl) {
				v.second->refreshDevices();
			}
			std::this_thread::sleep_for(std::chrono::seconds(10));
		}
	}).detach();
}

void M1OrientationManager::update() {
	std::vector<M1OrientationDeviceInfo> devices = getDevices();
	M1OrientationYPR ypr;

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
            ypr = hardwareImpl[currentDevice.getDeviceType()]->getOrientation().currentOrientation.getYPRasSignedNormalled();
            ypr.angleType = M1OrientationYPR::SIGNED_NORMALLED;
            if (!getTrackingYawEnabled()) ypr.yaw = 0.0;
            if (!getTrackingPitchEnabled()) ypr.pitch = 0.0;
            if (!getTrackingRollEnabled()) ypr.roll = 0.0;
            

            // commented out to avoid double applying offset angles from the get()
			//orientation.setYPR(ypr);
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

	j["trackingEnabled"] = { bTrackingYawEnabled, bTrackingPitchEnabled, bTrackingRollEnabled };
	j["orientation"] = { ypr.yaw, ypr.pitch, ypr.roll };

	mutex.lock();
	stringForClient = j.dump();
	mutex.unlock();


}

Orientation M1OrientationManager::getOrientation() {
    return orientation;
}

void M1OrientationManager::addHardwareImplementation(M1OrientationDeviceType type, HardwareAbstract* impl) {
    hardwareImpl[type] = impl;
}

void M1OrientationManager::close() {
    isRunning = false;

	server.stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    receiver.removeListener(this);
    receiver.disconnect();
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
    orientation.resetOrientation();
}

void M1OrientationManager::command_updateOscDevice(int new_port, std::string new_msg_address_pattern) {
    if (currentDevice.getDeviceType() == M1OrientationDeviceType::M1OrientationManagerDeviceTypeOSC) {
        if (currentDevice.osc_port != new_port) {
            // update port
            currentDevice.osc_port = new_port;
        }
        if (currentDevice.osc_msg_addr_pttrn != new_msg_address_pattern) {
            // update custom message pattern
            currentDevice.osc_msg_addr_pttrn = new_msg_address_pattern;
        }
    }
}
