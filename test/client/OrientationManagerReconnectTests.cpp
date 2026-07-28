// Regression tests for M1OrientationManager device connection handling.
//
// Guards against two field-reported failure modes:
//  1. A reconnect request for the device the manager already considered
//     "current" was silently ignored, so after a physical connection drop the
//     only recovery was restarting the whole session.
//  2. A dropped device was never detected (hardware update() returns -1 on
//     failure, and the old `!update()` check treated -1 as success), leaving
//     stale "connected" state forever.

#include <JuceHeader.h>
#include "M1OrientationManager.h"
#include "HardwareAbstract.h"

#include <iostream>
#include <string>

static int failures = 0;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (!(cond)) {                                                                \
            ++failures;                                                               \
            std::cout << "FAILED: " #cond " (" << __FILE__ << ":" << __LINE__ << ")"  \
                      << std::endl;                                                   \
        }                                                                             \
    } while (0)

namespace {

class FakeHardware : public HardwareAbstract {
public:
    int connectAttempts = 0;
    bool connectShouldSucceed = true;
    int updateResult = 1; // mirrors real implementations: 1 = ok, -1 = failure
    M1OrientationDeviceInfo connectedDevice;

    int setup() override { return 1; }
    int update() override { return updateResult; }
    int close() override
    {
        connectedDevice = M1OrientationDeviceInfo();
        return 1;
    }
    std::vector<M1OrientationDeviceInfo> getDevices() override { return {}; }
    void refreshDevices() override {}
    M1OrientationDeviceInfo getConnectedDevice() override { return connectedDevice; }
    M1OrientationTrackingResult getOrientation() override
    {
        M1OrientationTrackingResult result;
        result.success = true;
        return result;
    }
    void calibrateDevice() override {}
    void recenter() override {}
    void startTrackingUsingDevice(M1OrientationDeviceInfo device, TrackingCallback callback) override
    {
        ++connectAttempts;
        if (connectShouldSucceed) {
            connectedDevice = device;
            callback(true, "ok", device.getDeviceName(), device.getDeviceType(), device.getDeviceAddress());
        } else {
            callback(false, "failed", "", 0, "");
        }
    }
    void setAdditionalDeviceSettings(std::string) override {}
};

M1OrientationDeviceInfo makeTestDevice()
{
    return { "TestIMU", M1OrientationManagerDeviceTypeBLE, "aa:bb:cc" };
}

void testConnectSetsCurrentDevice()
{
    M1OrientationManager manager;
    FakeHardware fake;
    manager.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &fake);

    manager.command_startTrackingUsingDevice(makeTestDevice());

    CHECK(fake.connectAttempts == 1);
    CHECK(manager.getConnectedDevice() == makeTestDevice());
}

void testReconnectToSameDeviceIsHonored()
{
    M1OrientationManager manager;
    FakeHardware fake;
    manager.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &fake);

    manager.command_startTrackingUsingDevice(makeTestDevice());
    CHECK(fake.connectAttempts == 1);

    // Regression: a second request for the same device must perform a real
    // reconnect instead of being ignored because currentDevice matches.
    manager.command_startTrackingUsingDevice(makeTestDevice());
    CHECK(fake.connectAttempts == 2);
    CHECK(manager.getConnectedDevice() == makeTestDevice());
}

void testDeviceDropClearsConnectionState()
{
    M1OrientationManager manager;
    FakeHardware fake;
    manager.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &fake);

    manager.command_startTrackingUsingDevice(makeTestDevice());
    CHECK(manager.getConnectedDevice() == makeTestDevice());

    // Regression: sustained hardware update failure (-1) must clear the
    // connection state so clients can see the disconnect and reconnect.
    fake.updateResult = -1;
    for (int i = 0; i < M1OrientationManager::DEVICE_UPDATE_FAILURE_LIMIT; ++i)
        manager.update();

    CHECK(manager.getConnectedDevice().getDeviceType() == M1OrientationManagerDeviceTypeNone);

    // ...and a subsequent reconnect request must go through.
    fake.updateResult = 1;
    manager.command_startTrackingUsingDevice(makeTestDevice());
    CHECK(fake.connectAttempts == 2);
    CHECK(manager.getConnectedDevice() == makeTestDevice());
}

void testTransientFailureDoesNotDisconnect()
{
    M1OrientationManager manager;
    FakeHardware fake;
    manager.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &fake);

    manager.command_startTrackingUsingDevice(makeTestDevice());

    // A short failure burst (e.g. while a device connection is settling)
    // must not tear the connection down.
    fake.updateResult = -1;
    for (int i = 0; i < M1OrientationManager::DEVICE_UPDATE_FAILURE_LIMIT / 2; ++i)
        manager.update();

    fake.updateResult = 1;
    manager.update();

    CHECK(manager.getConnectedDevice() == makeTestDevice());

    // The failure counter must have been reset by the successful update.
    fake.updateResult = -1;
    for (int i = 0; i < M1OrientationManager::DEVICE_UPDATE_FAILURE_LIMIT - 1; ++i)
        manager.update();

    CHECK(manager.getConnectedDevice() == makeTestDevice());
}

} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    testConnectSetsCurrentDevice();
    testReconnectToSameDeviceIsHonored();
    testDeviceDropClearsConnectionState();
    testTransientFailureDoesNotDisconnect();

    if (failures == 0) {
        std::cout << "All m1-orientationmanager unit tests passed" << std::endl;
        return 0;
    }

    std::cout << failures << " check(s) failed" << std::endl;
    return 1;
}
