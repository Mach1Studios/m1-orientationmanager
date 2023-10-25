option(PLUGIN_FONT "Set the plugin's primary text font" "InterRegular.ttf")
set(FOUND_FONT InterRegular)
set(FOUND_FONT_TYPE ttf)
set(DEFAULT_FONT_SIZE 11)
juce_add_binary_data(Resources NAMESPACE BinaryData SOURCES mach1logo.png ../../Modules/m1_orientation_client/Resources/bluetooth_icon.png ../../Modules/m1_orientation_client/Resources/tech_support_icon.png ../../Modules/m1_orientation_client/Resources/wi-fi_icon.png ../../Modules/m1_orientation_client/Resources/usb_icon.png ../../Modules/m1_orientation_client/Resources/camera_icon.png InterRegular.ttf)

set(PLUGIN_FONT "${FOUND_FONT}.${FOUND_FONT_TYPE}")
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC PLUGIN_FONT="${PLUGIN_FONT}" LOCAL_FONT=${FOUND_FONT} LOCAL_FONT_TYPE=${FOUND_FONT_TYPE} DEFAULT_FONT_SIZE=${DEFAULT_FONT_SIZE} BINARYDATA_FONT=BinaryData::${FOUND_FONT}_${FOUND_FONT_TYPE} BINARYDATA_FONT_SIZE=BinaryData::${FOUND_FONT}_${FOUND_FONT_TYPE}Size)
message(STATUS "Font: ${PLUGIN_FONT}")
message(STATUS "Font (default) size: ${DEFAULT_FONT_SIZE}")

if(APPLE)
	# MacOS: Add privacy permissions for access to bluetooth
	#target_link_options(${CMAKE_PROJECT_NAME} PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
endif()