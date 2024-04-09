#ifndef M1_ORIENTATIONMANAGER_BLEDEVICEMAP_H
#define M1_ORIENTATIONMANAGER_BLEDEVICEMAP_H

#include "m1_orientation_client/M1OrientationTypes.h"

#include <simpleble/SimpleBLE.h>

#include <iostream>

namespace Mach1 {

    static constexpr int BLEDeviceScanTimeout = 2000;

    class BLEDeviceMap {
    public:

        BLEDeviceMap();

        using TrackingCallback = std::function<void(bool success, std::string message, std::string connectedDeviceName, int connectedDeviceType, std::string connectedDeviceAddress)>;

        class Listener {
        public:
            virtual ~Listener() = default;
            Listener() = default;
            Listener(const Listener&) = default;
            Listener& operator=(const Listener&) = default;
            Listener(Listener&&) noexcept = default;
            Listener& operator=(Listener&&) noexcept = default;

            virtual void OnNxTrackerDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnMetaWearDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnMach1MDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnUnknownDeviceDiscovered(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;

            virtual void OnNxTrackerConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral, TrackingCallback trackingCallback) = 0;
            virtual void OnMetaWearConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral, TrackingCallback trackingCallback) = 0;
            virtual void OnMach1MConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral, TrackingCallback trackingCallback) = 0;
            virtual void OnUnknownDeviceConnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral, TrackingCallback trackingCallback) = 0;

            virtual void OnNxTrackerDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnMetaWearDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnMach1MDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnUnknownDeviceDisconnected(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;

            virtual void OnNxTrackerUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnMetaWearUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnMach1MUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
            virtual void OnUnknownDeviceUpdated(M1OrientationDeviceInfo info, SimpleBLE::Safe::Peripheral& peripheral) = 0;
        };

        void RegisterListener(BLEDeviceMap::Listener* device_listener);

        void Refresh();

        void Disconnect();

        void Clear();

        void ConnectDevice(const std::string& device_name, M1OrientationDeviceType type, const std::string& device_address, TrackingCallback tracking_callback);

        std::vector<M1OrientationDeviceInfo> GetCurrentDevices();

        M1OrientationDeviceInfo GetConnectedDeviceInfo();

        bool IsConnected() const;

        void UpdateConnectedDevice();

    protected:

        void OnScanStarted();
        void OnScanStopped();
        void OnDeviceDiscovered(SimpleBLE::Safe::Peripheral& peripheral);
        void AddEntry(const M1OrientationDeviceInfo& device, SimpleBLE::Safe::Peripheral& peripheral);

    private:

        std::vector<SimpleBLE::Safe::Adapter> m_adapters;
        SimpleBLE::Safe::Adapter* m_active_adapter;

        std::unordered_map<M1OrientationDeviceInfo, SimpleBLE::Safe::Peripheral, M1OrientationDeviceInfo::Hash> m_devices;

        M1OrientationDeviceInfo m_connected_device;
        bool m_connected = false;
        bool displayOnlyKnownIMUs = true;

        std::mutex m_refresh_mutex;
        std::vector<BLEDeviceMap::Listener*> m_named_device_listeners;

    };

} // namespace Mach1


#endif //M1_ORIENTATIONMANAGER_BLEDEVICEMAP_H
