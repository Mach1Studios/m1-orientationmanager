# M1-OrientationManager
External orientation device manager and utilities geared toward aggregating different headtracking methods.

## Setup

### Build via CMake
- `mkdir cmake-build && cd cmake-build`
- `cmake ..` Create project files by adding the appropriate `-G Xcode` or `-G "Visual Studio 16 2019"` to the end of this line
- `cmake --build .`

### Build via .jucer
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