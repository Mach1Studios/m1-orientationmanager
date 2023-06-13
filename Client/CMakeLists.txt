cmake_minimum_required(VERSION 3.16)
file(STRINGS VERSION CURRENT_VERSION)
project(M1-OrientationClient VERSION ${CURRENT_VERSION})

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

#First, we'll add the CMake folder, incase we'll need to find_package later:
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

### IDE Generator pre-config ###
# Xcode: Disable automatic build scheme generation globally.
#        Instead, we explicitely specify which targets require schemes.
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
# Enable to build universal binaries on macOS, increasing build time
# This only affects local builds, GitHub actions always builds Universals
set(CMAKE_OSX_ARCHITECTURES arm64 x86_64)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Support macOS down to High Sierra")
# IDEs:  Enable grouping of source files into folders in IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# IDEs:  Create a folder in the IDE with the JUCE Module code.
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

#static linking in Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# add JUCE and other custom modules
if(PROJECT_IS_TOP_LEVEL)
    include(FetchContent)
    set(LIB_JUCE_TAG "7.0.5")

    # Keep dependencies outside of the "Build" directory.
    # This allows to do a clean build of the project without re-downloading or
    # rebuilding the dependencies.
    set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/Libs" CACHE PATH "External dependencies path." FORCE)

    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        ${LIB_JUCE_TAG}
        GIT_SHALLOW    TRUE
        GIT_CONFIG     advice.detachedHead=false    # Disable detached HEAD warning for fetching a specific tag
        SOURCE_DIR     "${FETCHCONTENT_BASE_DIR}/JUCE"
        SUBBUILD_DIR   "${FETCHCONTENT_BASE_DIR}/JUCE-Subbuild"
        BINARY_DIR     "${FETCHCONTENT_BASE_DIR}/JUCE-Build")

    FetchContent_MakeAvailable(juce)
else()
    # add JUCE and other custom modules
    #find_package(JUCE CONFIG REQUIRED)
    #add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Modules)
endif()

# PLUGIN/APP NAME (for dynamic naming schemes)
set(CUSTOM_BUNDLE_ID "com.mach1.orientationclient")
set(CUSTOM_APP_CODE "M1OC")

# add the plugin targets
juce_add_gui_app(M1-OrientationClient
                VERSION ${CURRENT_VERSION}
                COMPANY_NAME "Mach1"
                COMPANY_WEBSITE "https://mach1.tech"
                COMPANY_EMAIL "whatsup@mach1.tech"
                PLUGIN_MANUFACTURER_CODE "Mac1"
                PLUGIN_CODE ${CUSTOM_APP_CODE}
                BUNDLE_ID ${CUSTOM_BUNDLE_ID}
                PLUGIN_NAME M1-OrientationClient
                PRODUCT_NAME M1-OrientationClient
                IS_SYNTH FALSE)

# add required flags
juce_generate_juce_header(M1-OrientationClient)

target_compile_definitions(M1-OrientationClient
    PUBLIC 
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:M1-OrientationClient,JUCE_PROJECT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:M1-OrientationClient,JUCE_VERSION>"
    JUCE_WEB_BROWSER=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

# Set the C++ language standard requirenment for the "shared code" library target.
# Setting this to PUBLIC ensures that all dependent targets will inherit the specified C++ standard.
target_compile_features("M1-OrientationClient" PUBLIC cxx_std_17)
# Disable compiler extensions for the project targets (e.g. use -std=c++17 instead of -std=gnu++17).
get_property(project_targets DIRECTORY "${PROJECT_SOURCE_DIR}" PROPERTY BUILDSYSTEM_TARGETS)
set_target_properties(${project_targets} PROPERTIES CXX_EXTENSIONS OFF)

# add the sources
add_subdirectory(Source)
add_subdirectory(Resources)

# Required for Linux happiness:
# See https://forum.juce.com/t/loading-pytorch-model-using-binarydata/39997/2
set_target_properties(Resources PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_target_properties(Resources PROPERTIES FOLDER "Targets")
target_link_libraries(M1-OrientationClient PRIVATE Resources)

# definitions to replace the `JucePluginDefines.h`
set(JucePlugin_Name M1-OrientationClient)
set(JucePlugin_Desc M1-OrientationClient)

target_link_libraries(M1-OrientationClient PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_opengl
        juce::juce_osc
        juce_murka
        m1_orientation_client
        )

### IDE Generator post-config ###
# IDEs:  Move the "shared code" source group (main target) out of the "Targets" folder
#        to the top level.
set_target_properties("M1-OrientationClient" PROPERTIES FOLDER "")
# IDEs:  Organise source file grouping.
get_target_property(project_sources "M1-OrientationClient" SOURCES)
#   If JuceHeader.h is generated, remove it from the source file list and handle it individually.
get_target_property(juce_library_code "M1-OrientationClient" JUCE_GENERATED_SOURCES_DIRECTORY)
set(juce_header "${juce_library_code}/JuceHeader.h")
list(REMOVE_ITEM project_sources "${juce_header}")

#   Place JuceHeader.h and the associated CMake Rule file into a "JUCE Library Code" folder.
source_group("JUCE Library Code" FILES "${juce_header}")
source_group("JUCE Library Code/CMake Rules" FILES "${juce_header}.rule")

#   Generate source groups that follow the organisation of source file directories.
source_group("" FILES ${project_sources})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX "" FILES SOURCES)

#   Place Binary Data related source files into the root of their target folder.
if(TARGET Resources)
    get_target_property(project_assets Resources SOURCES)
    source_group("" FILES ${project_assets})
endif()

# Color our warnings and errors
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_compile_options(-fcolor-diagnostics)
endif()