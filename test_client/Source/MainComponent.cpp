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

	std::string resourcesPath;
	if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
		resourcesPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("Application Support/Mach1 Spatial System/resources").getFullPathName().toStdString();
	}
	else {
		resourcesPath = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile("Mach1 Spatial System/resources").getFullPathName().toStdString();
	}
	printf("Resources Loaded From: %s \n", resourcesPath.c_str());
	m.setResourcesPath(resourcesPath);

	//std::string settingsFilePath = (juce::File::getCurrentWorkingDirectory().getFullPathName() + "/settings.json").toStdString();
	//m1OrientationManagerOSCClient.initFromSettings(settingsFilePath);
	m1OrientationOSCClient.init(6345);

	m1OrientationOSCClient.setStatusCallback(std::bind(&MainComponent::setStatus, this, std::placeholders::_1, std::placeholders::_2));
}

//==============================================================================
void MainComponent::render()
{
	m.setFont("Proxima Nova Reg.ttf", 10);

	m.startFrame();
	m.setScreenScale((float)openGLContext.getRenderingScale());

	m.clear(20);
	m.setColor(255);

	m.begin();

	int offsetX = 0;
	int offsetY = 0;

	offsetX = 10;
	offsetY = 20;

	m.getCurrentFont()->drawString("connected: " + std::to_string(m1OrientationOSCClient.isConnectedToServer()), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

	Orientation orientation = m1OrientationOSCClient.getOrientation();

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
	m.getCurrentFont()->drawString("yaw: " + std::to_string(m1OrientationOSCClient.getTrackingYawEnabled()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("pitch: " + std::to_string(m1OrientationOSCClient.getTrackingPitchEnabled()), offsetX, offsetY);
	offsetY += 30;
	m.getCurrentFont()->drawString("roll: " + std::to_string(m1OrientationOSCClient.getTrackingRollEnabled()), offsetX, offsetY);
	offsetY += 30;

	offsetY += 20;

	M1OrientationDevice device = m1OrientationOSCClient.getCurrentDevice();
	m.getCurrentFont()->drawString("device: " + device.name + ":" + M1OrientationDeviceTypeName[device.type], offsetX, offsetY);
	offsetY += 40;

	m.getCurrentFont()->drawString("devices: ", offsetX, offsetY);
	offsetY += 40;
	std::vector<M1OrientationDevice> devices = m1OrientationOSCClient.getDevices();
	for (auto& device : devices) {
		m.getCurrentFont()->drawString("> " + device.name + ":" + M1OrientationDeviceTypeName[device.type], offsetX, offsetY);
		offsetY += 40;
	}

	offsetX = 400;
	offsetY = 20;

	auto& refreshDeviceButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("refresh devices");
	refreshDeviceButton.commit();
	if (refreshDeviceButton.pressed) {
		m1OrientationOSCClient.command_refreshDevices();
	}
	offsetY += 50;

	auto& selectDevice1Button = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("select device 1");
	selectDevice1Button.commit();
	if (selectDevice1Button.pressed) {
		std::vector<M1OrientationDevice> devices = m1OrientationOSCClient.getDevices();
		if (devices.size() > 0) {
			m1OrientationOSCClient.command_startTrackingUsingDevice(devices[0]);
		}
	}
	offsetY += 50;

	auto& selectDevice2Button = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("select device 2");
	selectDevice2Button.commit();
	if (selectDevice2Button.pressed) {
		std::vector<M1OrientationDevice> devices = m1OrientationOSCClient.getDevices();
		if (devices.size() > 0) {
			m1OrientationOSCClient.command_startTrackingUsingDevice(devices[1]);
		}
	}
	offsetY += 50;

	auto& toogleYawButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("toogle yaw");
	toogleYawButton.commit();
	if (toogleYawButton.pressed) {
		m1OrientationOSCClient.command_setTrackingYawEnabled(!m1OrientationOSCClient.getTrackingYawEnabled());
	}
	offsetY += 50;

	auto& tooglePitchButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("toogle pitch");
	tooglePitchButton.commit();
	if (tooglePitchButton.pressed) {
		m1OrientationOSCClient.command_setTrackingPitchEnabled(!m1OrientationOSCClient.getTrackingPitchEnabled());
	}
	offsetY += 50;

	auto& toogleRollButton = m.draw<Button>({ offsetX, offsetY, 130, 30 }).text("toogle roll");
	toogleRollButton.commit();
	if (toogleRollButton.pressed) {
		m1OrientationOSCClient.command_setTrackingRollEnabled(!m1OrientationOSCClient.getTrackingRollEnabled());
	}
	offsetY += 50;

	m.drawString("status: " + this->status, offsetX, offsetY);
	offsetY += 50;

	m.end();
}

void MainComponent::setStatus(bool success, std::string message)
{
	this->status = message;
	std::cout << success << " , " << message << std::endl;
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
