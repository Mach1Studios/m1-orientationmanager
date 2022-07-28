# M1-OrientationManager
External orientation device manager and utilities geared toward aggregating different headtracking methods.

## Setup

### Build via CMake
- `mkdir cmake-build && cd cmake-build`
- `cmake ..` Create project files by adding the appropriate `-G Xcode` or `-G "Visual Studio 16 2019"` to the end of this line
- `cmake --build .`

### Build via .jucer
- Copy the [m1_orientation_manager](modules/) JUCE module to your global JUCE user module directory
- Copy the [juce_murka](modules/) JUCE module to your global JUCE user module directory
- Open the `M1-OrientationManager-Server.jucer` and compile as needed

## Credits
This project heavily references and implements several design, UI and UX concepts from [nvsonic-head-tracker](https://github.com/trsonic/nvsonic-head-tracker) utility and expands upon it by handling and aggregating more connection types as well as creating a server/client design to handle dual direction communication between a network of clients.