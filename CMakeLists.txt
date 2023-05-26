cmake_minimum_required(VERSION 3.16)
project(M1-OrientationManager VERSION 0.0.1)

# c++17 required for BLE
set(CMAKE_CXX_STANDARD 17)

#First, we'll add the CMake folder, incase we'll need to find_package later:
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#Compile commands, useful for some IDEs like VS-Code
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

#Minimum MacOS target, set globally
if (${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "Minimum OS X deployment version" FORCE)

    #code signing to run on an iOS device:
#    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" CACHE STRING "" FORCE)
#    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "XXXXXXXXXX" CACHE STRING "" FORCE)
else()
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)
endif()

option(UniversalBinary "Build universal binary for mac" ON)

if (UniversalBinary)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE INTERNAL "")
endif()

#static linking in Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# add JUCE and other custom modules
add_subdirectory(JUCE)
add_subdirectory(Modules)
add_subdirectory(Source/SimpleBLE/simpleble)
add_subdirectory(Source/Devices/MetaWear)

juce_add_gui_app(${PROJECT_NAME} PRODUCT_NAME "${PROJECT_NAME}")
juce_generate_juce_header(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/Source/Main.cpp
        ${PROJECT_SOURCE_DIR}/Source/MainComponent.cpp
        ${PROJECT_SOURCE_DIR}/Source/M1OrientationOSCServer.cpp
        ${PROJECT_SOURCE_DIR}/Source/Devices/MetaWearInterface.cpp
        ${PROJECT_SOURCE_DIR}/Source/Devices/SupperwareInterface.cpp
        ${PROJECT_SOURCE_DIR}/Source/Devices/WitMotionInterface.cpp
        ${PROJECT_SOURCE_DIR}/Source/Devices/M1Interface.cpp
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_PROJECT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:${TargetName},JUCE_VERSION>")

target_include_directories(${PROJECT_NAME} PRIVATE 
        ${PROJECT_SOURCE_DIR}/Source/SimpleBLE/simpleble/include
        ${PROJECT_SOURCE_DIR}/Source/SimpleBLE/simpleble/export
        ${PROJECT_SOURCE_DIR}/Source/Devices/MetaWear/src
        ${PROJECT_SOURCE_DIR}/Source/Devices/ht-api-juce/supperware
        ${PROJECT_SOURCE_DIR}/Source/Devices/ht-api-juce/supperware/headpanel
        ${PROJECT_SOURCE_DIR}/Source/Devices/ht-api-juce/supperware/midi
        ${PROJECT_SOURCE_DIR}/Source/rs323
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_opengl
        juce::juce_osc
        juce_murka
        m1_orientation_client)

option(BUILD_UNIT_TESTS "Build JUCE prototype examples" OFF)

if (BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(test/client)
endif()