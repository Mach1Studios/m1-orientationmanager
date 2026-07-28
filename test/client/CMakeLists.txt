# Unit/regression tests for M1OrientationManager device connection handling.
# Enabled from the project root with -DBUILD_UNIT_TESTS=ON, run with ctest.

juce_add_console_app(m1-orientationmanager-tests
        PRODUCT_NAME "m1-orientationmanager-tests")

juce_generate_juce_header(m1-orientationmanager-tests)

target_sources(m1-orientationmanager-tests PRIVATE
        OrientationManagerReconnectTests.cpp
        ${CMAKE_SOURCE_DIR}/Source/M1OrientationManager.cpp)

target_include_directories(m1-orientationmanager-tests PRIVATE
        ${CMAKE_SOURCE_DIR}/Source)

target_compile_definitions(m1-orientationmanager-tests PRIVATE
        JUCE_APPLICATION_NAME_STRING="m1-orientationmanager-tests"
        JUCE_APPLICATION_VERSION_STRING="1.0.0"
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0)

target_compile_features(m1-orientationmanager-tests PRIVATE cxx_std_17)

target_link_libraries(m1-orientationmanager-tests PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics # required by the m1_orientation_client module
        juce::juce_osc
        m1_orientation_client
        m1_mathematics)

add_test(NAME m1-orientationmanager-unit COMMAND m1-orientationmanager-tests)
