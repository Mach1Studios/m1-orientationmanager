cmake_minimum_required(VERSION 3.16)
file(STRINGS VERSION CURRENT_VERSION)
project(M1-Orientation-Suite VERSION ${CURRENT_VERSION})

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

# c++17 required for BLE
set(CMAKE_CXX_STANDARD 17)

#First, we'll add the CMake folder, incase we'll need to find_package later:
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# add JUCE and other custom modules
add_subdirectory(JUCE)
add_subdirectory(Modules)

set(OM_APP_COMPONENTS "")
list(APPEND OM_APP_COMPONENTS "M1-OrientationManager")
list(APPEND OM_APP_COMPONENTS "M1-OrientationWatcher")

add_subdirectory(Server)
add_subdirectory(Watcher)

option(BUILD_CLIENT_EXAMPLE "Build M1-Orientation-Client example" OFF)
if (BUILD_CLIENT_EXAMPLE)
    list(APPEND OM_APP_COMPONENTS "M1-OrientationClient")
    add_subdirectory(Client)
endif()

option(BUILD_SERVER_DEBUG_UI "Build with debug GUI window" OFF)
if (BUILD_SERVER_DEBUG_UI)
    target_compile_definitions(M1-OrientationManager PUBLIC BUILD_SERVER_DEBUG_UI=${BUILD_SERVER_DEBUG_UI})
endif()

# Color our warnings and errors
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_compile_options(-fcolor-diagnostics)
endif()