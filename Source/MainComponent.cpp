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
	//m1OrientationManagerServer.initFromSettings(settingsFilePath);
	m1OrientationManagerServer.init(6345);
}

//==============================================================================
void MainComponent::render()
{
	m.startFrame();
	m.setScreenScale((float)openGLContext.getRenderingScale());

	m.clear(20);
	m.setColor(255);
//	m.setFontFromRawData("Proxima Nova Reg.ttf", juce::BinaryData::ProximaNovaReg_ttf, juce::BinaryData::ProximaNovaReg_ttfSize, 10);

	m.begin();

	float yaw = sin(juce::Time::currentTimeMillis() / 100000.0);
	float pitch = cos(juce::Time::currentTimeMillis() / 100000.0 - 0.3);
	float roll = sin(juce::Time::currentTimeMillis() / 100000.0 + 0.1);

	m1OrientationManagerServer.update();
	m1OrientationManagerServer.setOrientation(yaw, pitch, roll);

	int offsetY = 0;
	int offsetX = 0;
	
	offsetX = 10;
	offsetY = 20;

	m.getCurrentFont()->drawString("orientation: ", offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("yaw: " + std::to_string(yaw), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("pitch: " + std::to_string(pitch), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("roll: " + std::to_string(roll), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

	m.getCurrentFont()->drawString("tracking: ", offsetX, offsetY);
	offsetY += 40;
	m.getCurrentFont()->drawString("tracking: " + std::to_string(m1OrientationManagerServer.getTracking()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("yaw: " + std::to_string(m1OrientationManagerServer.getTrackingYaw()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("pitch: " + std::to_string(m1OrientationManagerServer.getTrackingPitch()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("roll: " + std::to_string(m1OrientationManagerServer.getTrackingRoll()), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

	m.getCurrentFont()->drawString("device: " + m1OrientationManagerServer.getCurrentDevice(), offsetX, offsetY);
	offsetY += 40;

	m.getCurrentFont()->drawString("devices: ", offsetX, offsetY);
	offsetY += 40;
	std::vector<std::string> devices = m1OrientationManagerServer.getDevices();
	for (auto& device : devices) {
		m.getCurrentFont()->drawString("> " + device, offsetX, offsetY);
		offsetY += 40;
	}

	offsetX = 220;
	offsetY = 20;
	
	auto clients = m1OrientationManagerServer.getClients();
	m.getCurrentFont()->drawString("clients: " + std::to_string(clients.size()), offsetX, offsetY);
	offsetY += 40;
	for (int i = 0; i < clients.size(); i++) {
		m.getCurrentFont()->drawString("> " + std::to_string(clients[i].port), offsetX, offsetY);
		offsetY += 30;
	}
	offsetY += 40;

	offsetX = 400;
	offsetY = 20;

	auto& refreshDeviceButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("refresh devices");
	refreshDeviceButton.commit();
	if (refreshDeviceButton.pressed) {
		m1OrientationManagerServer.command_refreshDevices();
	}
	offsetY += 50;

	auto& selectDevice1Button = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("select device 1");
	selectDevice1Button.commit();
	if (selectDevice1Button.pressed) {
		std::vector<std::string> devices = m1OrientationManagerServer.getDevices();
		if (devices.size() > 0) {
			m1OrientationManagerServer.command_selectDevice(devices[0]);
		}
	}
	offsetY += 50;

	auto& selectDevice2Button = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("select device 2");
	selectDevice2Button.commit();
	if (selectDevice2Button.pressed) {
		std::vector<std::string> devices = m1OrientationManagerServer.getDevices();
		if (devices.size() > 0) {
			m1OrientationManagerServer.command_selectDevice(devices[1]);
		}
	}
	offsetY += 50;

	auto& toogleTrackingButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("toogle tracking");
	toogleTrackingButton.commit();
	if (toogleTrackingButton.pressed) {
		m1OrientationManagerServer.command_setTracking(!m1OrientationManagerServer.getTracking());
	}
	offsetY += 50;

	auto& toogleYawButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("toogle yaw");
	toogleYawButton.commit();
	if (toogleYawButton.pressed) {
		m1OrientationManagerServer.command_setTrackingYaw(!m1OrientationManagerServer.getTrackingYaw());
	}
	offsetY += 50;

	auto& tooglePitchButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("toogle pitch");
	tooglePitchButton.commit();
	if (tooglePitchButton.pressed) {
		m1OrientationManagerServer.command_setTrackingPitch(!m1OrientationManagerServer.getTrackingPitch());
	}
	offsetY += 50;

	auto& toogleRollButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("toogle roll");
	toogleRollButton.commit();
	if (toogleRollButton.pressed) {
		m1OrientationManagerServer.command_setTrackingRoll(!m1OrientationManagerServer.getTrackingRoll());
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
