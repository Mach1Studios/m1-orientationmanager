file(GLOB source_files  Config.h
                        httplib.h # TODO: refactor as submodule
                        nlohmann/json.hpp # TODO: refactor as submodule
                        Main.cpp
                        MainComponent.h
                        MainComponent.cpp
                        M1OrientationManager.h
                        M1OrientationManager.cpp
                        M1OrientationManagerService.h
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
                        Devices/M1Interface.cpp
                        Devices/NxTrackerInterface.h
                        Devices/NxTrackerInterface.cpp)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${source_files})

# Add simpleble
message(STATUS "Adding SimpleBLE")
add_subdirectory(SimpleBLE/simpleble ${CMAKE_BINARY_DIR}/SimpleBLE/simpleble)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE SimpleBLE/simpleble/include SimpleBLE/simpleble/export)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC simpleble::simpleble)

# Add rs232
message(STATUS "Adding RS232 (serial)")
if(MSVC OR WIN32)
    target_sources(${CMAKE_PROJECT_NAME} PUBLIC rs232/rs232-win.c)
else()
    target_sources(${CMAKE_PROJECT_NAME} PUBLIC rs232/rs232-linux.c)
endif()
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE rs232)

# Add Metawear
message(STATUS "Device: Adding MetaMotion Support")
add_subdirectory(Devices/MetaWear)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE Devices/MetaWear/src)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC metawear)

# Add Supperware
message(STATUS "Device: Adding Supperware Support")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE Devices/ht-api-juce/supperware Devices/ht-api-juce/supperware/headpanel Devices/ht-api-juce/supperware/midi)