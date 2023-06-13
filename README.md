# M1-OrientationManager
This repo contains all the components relating to the M1-OrientationManager

## Components
- [Server](Server): Executable that finds and connects to various IMU devices and sends the orientation response to any available clients.
- [Watcher](Watcher): Executable responsible for checking that the server is still active and operating and otherwise attempt to relaunch it properly.
- [Client](Client): Module for adding an interface to any app/plugin to properly communicate with the server.

## Setup

### Build via CMake
- `mkdir cmake-build && cd cmake-build`
- `cmake ..` Create project files by adding the appropriate `-G Xcode` or `-G "Visual Studio 16 2019"` to the end of this line
- `cmake --build .`

### Build via .jucer
- Open each components .jucer file and build one at a time

## Install
- The M1-OrientationManager or M1-OrientationManager-Watcher should both be built and installed into the same directory.