#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    juce::OpenGLAppComponent::setSize(800, 600);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::initialise()
{
    murka::JuceMurkaBaseComponent::initialise();

	//std::string settingsFilePath = (juce::File::getCurrentWorkingDirectory().getFullPathName() + "/settings.json").toStdString();
	//m1OrientationOSCServer.initFromSettings(settingsFilePath);
	m1OrientationOSCServer.init(6345);

	m1OrientationHardwareBluetooth.setup();
	m1OrientationOSCServer.addHardwareImplementation(M1OrientationManagerDeviceTypeBLE, &m1OrientationHardwareBluetooth);
}

//==============================================================================
void MainComponent::render()
{
	m.startFrame();
	m.setScreenScale((float)openGLContext.getRenderingScale());

	m.clear(20);
	m.setColor(255);
	m.setFontFromRawData("Proxima Nova Reg.ttf", BinaryData::ProximaNovaReg_ttf, BinaryData::ProximaNovaReg_ttfSize, 10);

	m.begin();

	m1OrientationOSCServer.update();

	int offsetY = 0;
	int offsetX = 0;
	
	offsetX = 10;
	offsetY = 20;

	Orientation orientation = m1OrientationOSCServer.getOrientation();

	m.getCurrentFont()->drawString("orientation: ", offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("yaw: " + std::to_string(orientation.getYPR().yaw), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("pitch: " + std::to_string(orientation.getYPR().pitch), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("roll: " + std::to_string(orientation.getYPR().roll), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

	m.getCurrentFont()->drawString("tracking enabled: ", offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("yaw: " + std::to_string(m1OrientationOSCServer.getTrackingYawEnabled()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("pitch: " + std::to_string(m1OrientationOSCServer.getTrackingPitchEnabled()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("roll: " + std::to_string(m1OrientationOSCServer.getTrackingRollEnabled()), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

	M1OrientationDevice device = m1OrientationOSCServer.getCurrentDevice();
	m.getCurrentFont()->drawString("device: " + device.name + ":" + M1OrientationDeviceTypeName[device.type], offsetX, offsetY);
	offsetY += 40;

	m.getCurrentFont()->drawString("devices: ", offsetX, offsetY);
	offsetY += 40;
	std::vector<M1OrientationDevice> devices = m1OrientationOSCServer.getDevices();
	for (auto& device : devices) {
		m.getCurrentFont()->drawString("> " + device.name + ":" + M1OrientationDeviceTypeName[device.type], offsetX, offsetY);
		offsetY += 40;
	}

	offsetX = 220;
	offsetY = 20;
	
	auto clients = m1OrientationOSCServer.getClients();
	m.getCurrentFont()->drawString("clients: " + std::to_string(clients.size()), offsetX, offsetY);
	offsetY += 40;
	for (int i = 0; i < clients.size(); i++) {
		m.getCurrentFont()->drawString("> " + std::to_string(clients[i].port), offsetX, offsetY);
		offsetY += 30;
	}
	offsetY += 40;

	offsetX = 400;
	offsetY = 20;

	auto& refreshDeviceButton = m.draw<murka::Button>({ offsetX, offsetY, 130, 30 }).text("refresh devices");
	refreshDeviceButton.commit();
	if (refreshDeviceButton.pressed) {
		m1OrientationOSCServer.command_refreshDevices();
	}
	offsetY += 50;

	auto& selectDevice1Button = m.draw<murka::Button>({ offsetX, offsetY, 130, 30 }).text("select device 1");
	selectDevice1Button.commit();
	if (selectDevice1Button.pressed) {
		std::vector<M1OrientationDevice> devices = m1OrientationOSCServer.getDevices();
		if (devices.size() > 0) {
			m1OrientationOSCServer.command_startTrackingUsingDevice(devices[0]);
		}
	}
	offsetY += 50;

	auto& selectDevice2Button = m.draw<murka::Button>({ offsetX, offsetY, 130, 30 }).text("select device 2");
	selectDevice2Button.commit();
	if (selectDevice2Button.pressed) {
		std::vector<M1OrientationDevice> devices = m1OrientationOSCServer.getDevices();
		if (devices.size() > 0) {
			m1OrientationOSCServer.command_startTrackingUsingDevice(devices[1]);
		}
	}
	offsetY += 50;

	auto& toogleYawButton = m.draw<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toogle yaw");
	toogleYawButton.commit();
	if (toogleYawButton.pressed) {
		m1OrientationOSCServer.command_setTrackingYawEnabled(!m1OrientationOSCServer.getTrackingYawEnabled());
	}
	offsetY += 50;

	auto& tooglePitchButton = m.draw<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toogle pitch");
	tooglePitchButton.commit();
	if (tooglePitchButton.pressed) {
		m1OrientationOSCServer.command_setTrackingPitchEnabled(!m1OrientationOSCServer.getTrackingPitchEnabled());
	}
	offsetY += 50;

	auto& toogleRollButton = m.draw<murka::Button>({ offsetX, offsetY, 130, 30 }).text("toogle roll");
	toogleRollButton.commit();
	if (toogleRollButton.pressed) {
		m1OrientationOSCServer.command_setTrackingRollEnabled(!m1OrientationOSCServer.getTrackingRollEnabled());
	}
	offsetY += 50;


	m.end();
}

void MainComponent::paint(juce::Graphics& g)
{
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
