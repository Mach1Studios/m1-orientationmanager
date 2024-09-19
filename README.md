# m1-orientationmanager
External orientation device manager and utilities geared toward aggregating different headtracking methods.

## Setup

### Build via CMake
- `cmake -Bbuild` Create project files by adding the appropriate `-G Xcode` or `-G "Visual Studio 16 2019"` to the end of this line
- `cmake --build build -DENABLE_DEBUG_EMULATOR_DEVICE=ON`

#### CMake Options
- `ENABLE_DEBUG_EMULATOR_DEVICE` compiler flag when building a non-release build type will enable an extra "emulator" device which can be useful for debug

### Build via .jucer
- Compile the dependencies: `simpleble` and `metawear`:
- `cd Builds/MacOSX/` or `cd Builds/VisualStudio2019`
- `cmake ../../Source/SimpleBLE/simpleble -B../../Source/SimpleBLE/simpleble -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"`
- `cmake --build ../../Source/SimpleBLE/simpleble`
- `cmake ../../Source/Devices/MetaWear -B../../Source/Devices/MetaWear/build -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"`
- `cmake --build ../../Source/Devices/MetaWear/build --config Release`
- Open the `m1-orientationmanager.jucer` and compile as needed

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
- Custom Input OSC
- [Supperware IMU](https://supperware.co.uk/)
- [MetaWear/mBientLab IMUs](https://mbientlab.com/)
- [Waves Nx Tracker IMU](https://www.waves.com/hardware/nx-head-tracker)
- [M1 IMU](https://www.mach1.tech/products)
- [WitMotion IMUs](https://www.wit-motion.com/) _[WIP]_

_Please feel free to open an issue requesting a new device with device details_

## OSC Tool
This repo also includes a flexible and simple UI app to show an example of interfacing with the m1-orientationmanager background service and output the calculated orientation via OSC to any IP + port for use with other software. 

This can be found in the [osc_client](osc_client) directory.

### Setup
The setup for the OSC Tool can be [found here](https://github.com/Mach1Studios/m1-orientationmanager/tree/main/osc_client#build-via-cmake)

## Credits
This project references and implements some design, UI and UX concepts from [nvsonic-head-tracker](https://github.com/trsonic/nvsonic-head-tracker) utility and expands upon it by handling and aggregating more connection types as well as creating a server/client design to handle dual direction communication between a network of clients.
