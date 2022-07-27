# M1-OrientationManager
External orientation device manager and utilities geared toward aggregating different headtracking methods.

## Setup
- Copy the [m1_orientation_manager](modules/) JUCE module to your global JUCE user module directory
- Copy the [juce_murka](modules/) JUCE module to your global JUCE user module directory

## Credits
This project heavily references and implements several design, UI and UX concepts from [nvsonic-head-tracker](https://github.com/trsonic/nvsonic-head-tracker) utility and expands upon it by handling and aggregating more connection types as well as creating a server/client design to handle dual direction communication between a network of clients.