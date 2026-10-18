cmake_minimum_required(VERSION 3.22)

project(LaxersazOrgan VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Laxersaz Organ"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
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
target_link_libraries(ToyOrgan PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Automatically bundle factory Soundsets into Standalone target
if (TARGET ToyOrgan_Standalone)
    if (APPLE)
        add_custom_command(TARGET ToyOrgan_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Soundsets
            $<TARGET_FILE_DIR:ToyOrgan_Standalone>/../Resources/Soundsets
        )
    else()
        add_custom_command(TARGET ToyOrgan_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Soundsets
            $<TARGET_FILE_DIR:ToyOrgan_Standalone>/Soundsets
        )
    endif()
endif()

