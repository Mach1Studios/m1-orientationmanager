# Adds the Mach1 math
# (Please ensure this is added before the `m1_orientation_client` JUCE module)
add_subdirectory(m1_orientation_client/libs/m1-mathematics)

juce_add_modules(
        juce_murka
        m1_orientation_client)