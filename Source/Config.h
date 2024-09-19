#pragma once

#define XSTR(x) STR(x)
#define STR(x) #x

#ifndef PI
#define PI       3.14159265358979323846
#endif

/// Static Color Scheme
#define M1_ACTION_YELLOW 255, 198, 30
// 204, 204, 204 seen on ENABLED knobs in legacy as well
#define ENABLED_PARAM 190, 190, 190
#define DISABLED_PARAM 63, 63, 63
#define BACKGROUND_GREY 40, 40, 40

#define GRID_LINES_1_RGBA 68, 68, 68, 51//0.2 opacity //small grid lines
#define GRID_LINES_2 68, 68, 68
#define GRID_LINES_3_RGBA 102, 102, 102, 178//0.7 opacity
#define GRID_LINES_4_RGB 133, 133, 133
#define OVERLAY_YAW_REF_RGBA 93, 93, 93, 51

#define LABEL_TEXT_COLOR 163, 163, 163
#define REF_LABEL_TEXT_COLOR 93, 93, 93
#define HIGHLIGHT_COLOR LABEL_TEXT_COLOR
#define HIGHLIGHT_TEXT_COLOR 0, 0, 0
#define APP_LABEL_TEXT_COLOR GRID_LINES_4_RGB

#define METER_RED 178, 24, 23
#define METER_YELLOW 220, 174, 37
#define METER_GREEN 67, 174, 56

#ifdef PLUGIN_FONT
    #pragma message XSTR(LOCAL_FONT) "." XSTR(LOCAL_FONT_TYPE)
#else
    // default CC0 font
    #define PLUGIN_FONT "InterRegular.ttf"
    #define BINARYDATA_FONT BinaryData::InterRegular_ttf
    #define BINARYDATA_FONT_SIZE BinaryData::InterRegular_ttfSize
    #define DEFAULT_FONT_SIZE 11
#endif
#pragma message XSTR(PLUGIN_FONT)
#pragma message XSTR(BINARYDATA_FONT)
#pragma message XSTR(BINARYDATA_FONT_SIZE)
#pragma message XSTR(DEFAULT_FONT_SIZE)

#ifdef ENABLE_DEBUG_EMULATOR_DEVICE
    #define INCLUDE_HARDWARE_EMULATOR
#endif
