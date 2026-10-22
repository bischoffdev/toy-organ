cmake_minimum_required(VERSION 3.22)

if (NOT DEFINED ORGAN_VERSION OR ORGAN_VERSION STREQUAL "")
    set(ORGAN_VERSION "0.1.0")
endif()

project(LaxersazOrgan VERSION ${ORGAN_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch JUCE framework automatically
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Define the audio plugin target
juce_add_plugin(ToyOrgan
    COMPANY_NAME "BischoffDev"
    PLUGIN_MANUFACTURER_CODE Bsch
    PLUGIN_CODE Lxso
    FORMATS VST3 AU Standalone LV2
    LV2URI "https://bischoff.dev/plugins/ToyOrgan"
    PRODUCT_NAME "Laxersaz Organ"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon_512.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon_128.png"
)

# Plugin source files
target_sources(ToyOrgan PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# Compiler definitions
target_compile_definitions(ToyOrgan PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link required JUCE modules (including audio_formats for WAV/AIFF sample loading)
juce_add_binary_data(ToyOrganResources
    SOURCES
        Resources/icon_128.png
)

target_link_libraries(ToyOrgan PRIVATE
    ToyOrganResources
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Automatically bundle factory Soundsets and icon into Standalone target
if (TARGET ToyOrgan_Standalone)
    if (APPLE)
        add_custom_command(TARGET ToyOrgan_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Soundsets
            $<TARGET_FILE_DIR:ToyOrgan_Standalone>/../Resources/Soundsets
        )
        add_custom_command(TARGET ToyOrgan_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Icon.icns
            $<TARGET_FILE_DIR:ToyOrgan_Standalone>/../Resources/Icon.icns
        )
    else()
        add_custom_command(TARGET ToyOrgan_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Soundsets
            $<TARGET_FILE_DIR:ToyOrgan_Standalone>/Soundsets
        )
    endif()
endif()

# Custom target for local developers: cmake --build build --target install_user
if (WIN32)
    add_custom_target(install_user
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/install.bat"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Installing Laxersaz Organ plugins and soundsets..."
    )
else()
    add_custom_target(install_user
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/install.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Installing Laxersaz Organ plugins and soundsets..."
    )
endif()

# Utility executable to generate clean screenshot of the UI
add_executable(render_screenshot Source/RenderScreenshot.cpp)
target_link_libraries(render_screenshot PRIVATE 
    ToyOrgan
    juce::juce_gui_extra
    juce::juce_gui_basics
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_formats
)



