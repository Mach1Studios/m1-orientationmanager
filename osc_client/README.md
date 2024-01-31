# M1-OrientationOSC Client App
Client GUI app for interfacing with the m1-orientationmanager service. Also can be used to relay the orientation to a software target via OSC.

## Setup

This project is designed to be used 2 ways: 
- [default] Built as is and the M1-OrientationOSC.app/exe will expect to communicate with already installed M1-OrientationManager and M1-System-Helper to avoid conflicts when these are already installed as services
- [Warning this can cause conflict if a locally installed Mach1 Spatial System 2.0+ is installed] Build with the definition `M1_ORIENTATION_MANAGER_EMBEDDED` so that the Client expects to find sibling m1-orientationmanager executable and associated `settings.json` locally. 

### Build via CMake
- `cmake -Bbuild` Create project files by adding the appropriate `-G Xcode` or `-G "Visual Studio 16 2019"` also decide if you are building to sit as a portable client (and not installed to a machine that already has Mach1 Spatial System installed) add `-DM1_ORIENTATION_MANAGER_EMBEDDED=1`
- `cmake --build build`

### Build via .jucer
- `cd Builds/MacOSX/` or `cd Builds/VisualStudio2019`
- if you are building to sit as a portable client (and not installed to a machine that already has Mach1 Spatial System installed) add `M1_ORIENTATION_MANAGER_EMBEDDED=1` to the target definitions.
- Open the `M1-OrientationOSC.jucer` and compile as needed

## Install
By default the m1-orientationmanager service executable is expected in a common data directory of each local machine, and where applicable to be managed by a service agent or LaunchAgent unless built with the `M1_ORIENTATION_MANAGER_EMBEDDED` definition.

**If using `M1_ORIENTATION_MANAGER_EMBEDDED` please make sure the m1-orientationmanager executable and the settings.json are also installed locally**
