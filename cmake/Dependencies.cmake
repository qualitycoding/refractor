# FROZEN DEPENDENCY PINS — full commit SHAs only (checked by tests/scripts/check_pins.py, T-050)
include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

if(REFRACTOR_BUILD_PLUGIN)
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        91ad83ae34a81e0833b1a2b0866f54846370ae53 # 8.0.15
    GIT_SHALLOW    FALSE)
  FetchContent_MakeAvailable(juce)
endif()

if(REFRACTOR_BUILD_TESTS)
  FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        317ac1ed4c0bb6e6b91eafc817e05c488feffcb3 # v3.16.0
    GIT_SHALLOW    FALSE)
  FetchContent_MakeAvailable(Catch2)
endif()
