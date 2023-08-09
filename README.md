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

## Credits
This project heavily references and implements several design, UI and UX concepts from [nvsonic-head-tracker](https://github.com/trsonic/nvsonic-head-tracker) utility and expands upon it by handling and aggregating more connection types as well as creating a server/client design to handle dual direction communication between a network of clients.