#include "BLEDeviceMap.h"


Mach1::BLEDeviceMap::BLEDeviceMap() : m_adapters(), m_active_adapter(nullptr), m_devices() {

    auto ble_list = SimpleBLE::Safe::Adapter::get_adapters();

	if (!ble_list.has_value() || ble_list->empty()) {
		std::cout << "[BLE] Warning: No adapter was found. BLE functionality will be disabled.\r\n";
		return;
	}
	
	m_adapters = std::move(ble_list.value());

    m_active_adapter = &m_adapters[0];

    m_active_adapter->set_callback_on_scan_start([&]() {
        OnScanStarted();
    });

    m_active_adapter->set_callback_on_scan_stop([&]() {
        OnScanStopped();
    });

    m_active_adapter->set_callback_on_scan_found([&](SimpleBLE::Safe::Peripheral peripheral) {
        OnDeviceDiscovered(peripheral);
    });

}

void Mach1::BLEDeviceMap::Refresh() {
	Clear();

	if (m_active_adapter) {
		m_active_adapter->scan_for(BLEDeviceScanTimeout);
	}
}

void Mach1::BLEDeviceMap::UpdateConnectedDevice() {
    if (!IsConnected()) {
        return;
    }

    // Update RSSI value
    m_connected_device.signalStrength = m_devices.at(m_connected_device).rssi().value_or(0);

    if (m_connected_device.isDeviceName("Nx Tracker")) {
        for(auto* listener: m_named_device_listeners) {
            listener->OnNxTrackerUpdated(m_connected_device, m_devices.at(m_connected_device));
        }
    }

    else if (m_connected_device.isDeviceName("MetaWear")) {
        for(auto* listener: m_named_device_listeners) {
            listener->OnMetaWearUpdated(m_connected_device,  m_devices.at(m_connected_device));
        }
    }

    else if (m_connected_device.isDeviceName("Mach1-M")) {
        for(auto* listener: m_named_device_listeners) {
            listener->OnMach1MUpdated(m_connected_device,  m_devices.at(m_connected_device));
        }
    }

    else if (!displayOnlyKnownIMUs) {
        for(auto* listener: m_named_device_listeners) {
            listener->OnUnknownDeviceUpdated(m_connected_device,  m_devices.at(m_connected_device));
        }
    }

}

void Mach1::BLEDeviceMap::Disconnect() {
    std::scoped_lock<std::mutex> lock(m_refresh_mutex);
    if (!IsConnected()) {
        return;
    }

    m_devices.at(m_connected_device).disconnect();
    m_connected = false;
}

std::vector<M1OrientationDeviceInfo> Mach1::BLEDeviceMap::GetCurrentDevices() {
    std::vector<M1OrientationDeviceInfo> tbr;
    tbr.reserve(m_devices.size());

    for(auto& iter: m_devices) {
        tbr.push_back(iter.first);
    }

    return tbr;
}

void Mach1::BLEDeviceMap::ConnectDevice(const std::string& device_name, M1OrientationDeviceType type, const std::string& device_address, Mach1::BLEDeviceMap::TrackingCallback tracking_callback) {
    for(auto& iter: m_devices) {

        if (!iter.first.isDeviceName(device_name)) {
            continue;
        }

        if (!iter.first.isDeviceAddress(device_address)) {
            continue;
        }

        m_connected_device = iter.first;

        m_devices.at(m_connected_device).set_callback_on_connected([&]() {

            m_connected = true;

            if (m_connected_device.isDeviceName("Nx Tracker")) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnNxTrackerConnected(m_connected_device,  m_devices.at(m_connected_device), tracking_callback);
                }
            }

            else if (m_connected_device.isDeviceName("MetaWear")) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnMetaWearConnected(m_connected_device,  m_devices.at(m_connected_device), tracking_callback);
                }
            }

            else if (m_connected_device.isDeviceName("Mach1-M")) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnMach1MConnected(m_connected_device,  m_devices.at(m_connected_device), tracking_callback);
                }
            }

            else if (!displayOnlyKnownIMUs) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnUnknownDeviceConnected(m_connected_device,  m_devices.at(m_connected_device), tracking_callback);
                }
            }

        });

        m_devices.at(m_connected_device).set_callback_on_disconnected([&]() {

            m_connected = false;

            if (m_connected_device.isDeviceName("Nx Tracker")) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnNxTrackerDisconnected(m_connected_device,  m_devices.at(m_connected_device));
                }
            }

            else if (m_connected_device.isDeviceName("MetaWear")) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnMetaWearDisconnected(m_connected_device,  m_devices.at(m_connected_device));
                }
            }

            else if (m_connected_device.isDeviceName("Mach1-M")) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnMach1MDisconnected(m_connected_device,  m_devices.at(m_connected_device));
                }
            }

            else if (!displayOnlyKnownIMUs) {
                for(auto* listener: m_named_device_listeners) {
                    listener->OnUnknownDeviceDisconnected(m_connected_device,  m_devices.at(m_connected_device));
                }
            }

        });

        m_devices.at(m_connected_device).connect();

        return;
    }

    m_connected = false;

}

void Mach1::BLEDeviceMap::OnDeviceDiscovered(SimpleBLE::Safe::Peripheral& peripheral) {
    auto id = peripheral.identifier().value_or("UNKNOWN");
    auto address = peripheral.address().value_or("UNKNOWN");
    auto rssi = peripheral.rssi().value_or(0);
    auto oriType = M1OrientationDeviceType::M1OrientationManagerDeviceTypeBLE;

    std::cout << "[BLE] Found device: " << id << " [" << address << "] " << rssi << " dBm" << std::endl;

    M1OrientationDeviceInfo device(id, oriType, address, rssi);

    if (m_devices.find(device) != m_devices.end()) {
        return;
    }

    if (device.isDeviceName("Nx Tracker")) {
        for(auto* listener: m_named_device_listeners) {
            AddEntry(device, peripheral);
            listener->OnNxTrackerDiscovered(device, peripheral);
        }
    }

    else if (device.isDeviceName("MetaWear")) {
        for(auto* listener: m_named_device_listeners) {
            AddEntry(device, peripheral);
            listener->OnMetaWearDiscovered(device, peripheral);
        }
    }

    else if (device.isDeviceName("Mach1-M")) {
        for(auto* listener: m_named_device_listeners) {
            AddEntry(device, peripheral);
            listener->OnMach1MDiscovered(device, peripheral);
        }
    }

    else if (!displayOnlyKnownIMUs) {
        for(auto* listener: m_named_device_listeners) {
            AddEntry(device, peripheral);
            listener->OnUnknownDeviceDiscovered(device, peripheral);
        }
    }
}

void Mach1::BLEDeviceMap::OnScanStarted() {
    std::cout << "[BLE] Scan started." << std::endl;
}

void Mach1::BLEDeviceMap::OnScanStopped() {
    std::cout << "[BLE] Scan stopped." << std::endl;
}

void Mach1::BLEDeviceMap::RegisterListener(BLEDeviceMap::Listener *device_listener) {

    for (auto* listener: m_named_device_listeners) {
        if (listener == device_listener) {
            return;
        }
    }

    // Add it to the list of device listeners only if it's not already in there.
    m_named_device_listeners.push_back(device_listener);
}

void Mach1::BLEDeviceMap::AddEntry(const M1OrientationDeviceInfo& device, SimpleBLE::Safe::Peripheral& peripheral) {
    std::scoped_lock<std::mutex> lock(m_refresh_mutex);

    m_devices.emplace(device, peripheral);
}

void Mach1::BLEDeviceMap::Clear() {
    std::scoped_lock<std::mutex> lock(m_refresh_mutex);

    if (!IsConnected()) {
        m_devices.clear();
        return;
    }

    for(auto iter = m_devices.begin(); iter != m_devices.end(); ) {
        if (iter->first != m_connected_device) {
            iter = m_devices.erase(iter);
        } else {
            ++iter;
        }
    }
}

M1OrientationDeviceInfo Mach1::BLEDeviceMap::GetConnectedDeviceInfo() {
    return m_connected_device;
}

bool Mach1::BLEDeviceMap::IsConnected() const {
    return m_connected;
}

