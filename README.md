# M1-OrientationManager
External orientation device manager and utilities geared toward aggregating different headtracking methods.

## Setup

### Build via CMake
- `cmake -Bbuild` Create project files by adding the appropriate `-G Xcode` or `-G "Visual Studio 16 2019"` to the end of this line
- `cmake --build build`

### Build via .jucer
- Compile the dependencies: `simpleble` and `metawear`:
- `cd Builds/MacOSX/` or `cd Builds/VisualStudio2019`
- `cmake ../../Source/SimpleBLE/simpleble -B../../Source/SimpleBLE/simpleble -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"`
- `cmake --build ../../Source/SimpleBLE/simpleble`
- `cmake ../../Source/Devices/MetaWear -B../../Source/Devices/MetaWear/build -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"`
- `cmake --build ../../Source/Devices/MetaWear/build --config Release`
- Open the `M1-OrientationManager-Server.jucer` and compile as needed

## Install
Currently this helper service executable is expected in a common data directory of each local machine, and where applicable to be managed by a service agent or LaunchAgent.

### OSX
- `cmake -Bbuild -G "Xcode" -DCMAKE_INSTALL_PREFIX="/Library/Application Support/Mach1"`
- `cmake --build --configuration Release --install`

### WIN
- `cmake -Bbuild -G "Visual Studio 16 2019" -DCMAKE_INSTALL_PREFIX="%APP_DATA%\Mach1"`
- `cmake --build --configuration Release --install`

## Design
Design and architecture of the project is described in the [Source](Source) directory, currently this project allows data transmission types to be defined via overrides of the [HardwareAbstract.h](Source/HardwareAbstract.h) while new devices using those data transmission types can be defined via the [Devices](Source/Devices) directory. 

### Currently Supports

#### Hardware / Data Transmission
- [Serial](Source/HardwareSerial.h) _using rs232_
- [BLE](Source/HardwareBLE.h) _using simpleble_
- [OSC](Source/HardwareOSC.h)
- [Camera](Source/HardwareCamera.h) _[WIP]_
- [Emulator](Source/HardwareEmulator) _[primarily for debug]_

#### Devices
- [Supperware IMU](https://supperware.co.uk/)
- [MetaWear/mBientLab IMUs](https://mbientlab.com/)
- [M1 IMU](https://www.mach1.tech/products)
- [WitMotion IMUs](https://www.wit-motion.com/) _[WIP]_

_Please feel free to open an issue requesting a new device with device details_

## OSC Tool
This repo also includes a flexible and simple UI app to show an example of interfacing with the M1-OrientationManager background service and output the calculated orientation via OSC to any IP + port for use with other software. 

This can be found in the [osc_client](osc_client) directory.

### Setup
- Build via CMake or JUCE to compile the contents of [osc_client](osc_client) via the same methods described above
- Copy the [settings.json](Resources/settings.json) and built M1-OrientationManager executable to be alongside (as siblings) the M1-OrientationOSC app/exe within the same directory

## TODO
- Finish camera implementation
- Allow camera to be added to any paired IMU for fusion calculations (fixing drift when face orientation is detected periodically)
- Add CI/CD for the OSC tool for users who need a more portable osc output tool
- Add service handling for windows
- Add service handling for linux

## Credits
This project heavily references and implements several design, UI and UX concepts from [nvsonic-head-tracker](https://github.com/trsonic/nvsonic-head-tracker) utility and expands upon it by handling and aggregating more connection types as well as creating a server/client design to handle dual direction communication between a network of clients.