# Refractor — behavioural reproduction of a polyphonic pitch-warping pedal (see HANDOFF.md)
cmake_minimum_required(VERSION 3.22)
project(Refractor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(REFRACTOR_BUILD_PLUGIN "Build the JUCE VST3/Standalone plugin" ON)
option(REFRACTOR_BUILD_TESTS  "Build test executables" ON)
option(REFRACTOR_BUILD_TOOLS  "Build demo renderer and editor snapshot tools" OFF)
option(REFRACTOR_SANITIZE     "Build with ASan+UBSan (Linux/macOS, Debug)" OFF)

if(REFRACTOR_SANITIZE AND NOT MSVC)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

include(cmake/Dependencies.cmake)

# ---- Pure C++20 DSP core (no JUCE) ----
add_library(refractor_dsp STATIC
  src/dsp/Params.cpp
  src/dsp/Mapping.cpp
  src/dsp/PitchShifter.cpp
  src/dsp/ClockDomain.cpp
  src/dsp/DualModeSwitch.cpp
  src/dsp/Engine.cpp)
target_include_directories(refractor_dsp PUBLIC src)
if(MSVC)
  target_compile_options(refractor_dsp PRIVATE /W4)
else()
  target_compile_options(refractor_dsp PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(REFRACTOR_BUILD_PLUGIN)
  # Sources shared by the plugin and the JUCE-linked test executable (compiled into each consumer).
  add_library(refractor_plugin_shared INTERFACE)
  target_sources(refractor_plugin_shared INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/PluginProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin/PluginEditor.cpp)
  target_include_directories(refractor_plugin_shared INTERFACE src)
  target_compile_definitions(refractor_plugin_shared INTERFACE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
  target_link_libraries(refractor_plugin_shared INTERFACE
    refractor_dsp juce::juce_audio_utils juce::juce_dsp)

  juce_add_plugin(Refractor
    COMPANY_NAME "qualitycoding"
    PLUGIN_MANUFACTURER_CODE Qcdg
    PLUGIN_CODE Rfrc
    FORMATS VST3 Standalone
    PRODUCT_NAME "Refractor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
  target_link_libraries(Refractor
    PRIVATE refractor_plugin_shared
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
endif()

if(REFRACTOR_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(bench)
endif()

if(REFRACTOR_BUILD_TOOLS)
  add_subdirectory(tools)
endif()
