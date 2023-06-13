file(GLOB source_files  Config.h
                        Main.cpp
                        MainComponent.h
                        MainComponent.cpp
                        M1OrientationOSCServer.h
                        M1OrientationOSCServer.cpp
                        HardwareAbstract.h
                        HardwareSerial.h
                        HardwareBLE.h
                        HardwareOSC.h
                        HardwareCamera.h
                        HardwareEmulator.h
                        Devices/MetaWearInterface.h
                        Devices/MetaWearInterface.cpp
                        Devices/SupperwareInterface.h
                        Devices/SupperwareInterface.cpp
                        Devices/WitMotionInterface.h
                        Devices/WitMotionInterface.cpp
                        Devices/M1Interface.h
                        Devices/M1Interface.cpp)

target_sources(M1-OrientationManager PRIVATE ${source_files})

# Add simpleble
message(STATUS "Adding SimpleBLE")
add_subdirectory(SimpleBLE/simpleble ${CMAKE_BINARY_DIR}/SimpleBLE/simpleble)
target_include_directories(M1-OrientationManager PRIVATE SimpleBLE/simpleble/include SimpleBLE/simpleble/export)
target_link_libraries(M1-OrientationManager PUBLIC simpleble::simpleble)

# Add rs232
message(STATUS "Adding RS232 (serial)")
if(MSVC OR WIN32)
    target_sources(M1-OrientationManager PUBLIC rs232/rs232-win.c)
else()
    target_sources(M1-OrientationManager PUBLIC rs232/rs232-linux.c)
endif()
target_include_directories(M1-OrientationManager PRIVATE rs232)

# Add Metawear
message(STATUS "Device: Adding MetaMotion Support")
add_subdirectory(Devices/MetaWear)
target_include_directories(M1-OrientationManager PRIVATE Devices/MetaWear/src)
target_link_libraries(M1-OrientationManager PUBLIC metawear metawear::metawear)

# Add Supperware
message(STATUS "Device: Adding Supperware Support")
target_include_directories(M1-OrientationManager PRIVATE Devices/ht-api-juce/supperware Devices/ht-api-juce/supperware/headpanel Devices/ht-api-juce/supperware/midi)