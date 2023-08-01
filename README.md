# M1-OrientationManager & M1-ServerWatcher
External orientation device manager and utilities geared toward aggregating different headtracking methods. This repository also contains another helper executabel responsible for relaunching the Orientation Manager if needed or crashed via a client.

## Project Goal
This repository is designed to be crossplatform and can interface with several device connection protocols. There is a [HardwareAbstract.h](server/Source/HardwareAbstract.h) designed for any developer to extend as a new device protocol if needed.

## Device Comm Protocols
The server already contains the following device protocols and can be extended for more if needed: 
- [HardwareSerial](server/Source/HardwareSerial.h) _This can include bluetooth paired devices over tty/cu_
- [HardwareBLE](server/Source/HardwareBLE.h)
- [HardwareOSC](server/Source/HardwareOSC.h)
- [HardwareCamera](server/Source/HardwareCamera.h) _WIP_
- [HardwareEmulator](server/Source/HardwareEmulator.h) _For debug and testing_

## Devices
[Devices](server/Source/Devices) directory is where 3rd party orientation devices can be defined for connection and handling.

## Setup
Change your current directory to either `server/`, `/test_client` or `watcher/` and use one of the following methods to build the executable.

### Build via CMake
- `mkdir cmake-build && cd cmake-build`
- `cmake ..` Create project files by adding the appropriate `-G Xcode` or `-G "Visual Studio 16 2019"` to the end of this line
- `cmake --build .`

### Build via .jucer
- Open the `M1-OrientationManager-Server.jucer` and compile as needed

## Contributions
Please feel free to open a pull request to add a new 3rd devices as needed, currently there is an interfacing class for each device to encourage easier organization and reading.

## Notes
Some of this project contains communication for the Mach1 Spatial System, this will designed segmented in the near future to encourage developers to use this as needed for their own headtracking or orientation management needs.