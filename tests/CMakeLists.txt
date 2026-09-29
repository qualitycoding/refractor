# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_executable(refractor_dsp_tests
  dsp/test_params_mapping.cpp dsp/test_dualmode_switch.cpp dsp/test_engine_audio.cpp dsp/test_engine_operational.cpp)
target_include_directories(refractor_dsp_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(refractor_dsp_tests PRIVATE refractor_dsp Catch2::Catch2WithMain)
add_test(NAME dsp_tests COMMAND refractor_dsp_tests --reporter console --reporter JUnit::out=${CMAKE_BINARY_DIR}/dsp_tests.xml)

add_executable(refractor_rt_tests dsp/test_realtime_alloc.cpp)
target_include_directories(refractor_rt_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(refractor_rt_tests PRIVATE refractor_dsp Catch2::Catch2WithMain)
add_test(NAME rt_tests COMMAND refractor_rt_tests)

if(REFRACTOR_BUILD_PLUGIN)
  add_executable(refractor_plugin_tests plugin/test_plugin.cpp)
  target_include_directories(refractor_plugin_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(refractor_plugin_tests PRIVATE refractor_plugin_shared Catch2::Catch2WithMain
    juce::juce_recommended_config_flags)
  target_compile_definitions(refractor_plugin_tests PRIVATE JUCE_STANDALONE_APPLICATION=1 JUCE_MODAL_LOOPS_PERMITTED=0)
  add_test(NAME plugin_tests COMMAND refractor_plugin_tests)
  add_test(NAME pluginval COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_pluginval.sh ${CMAKE_BINARY_DIR})
endif()

add_test(NAME check_pins   COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_pins.py   ${CMAKE_SOURCE_DIR})
add_test(NAME check_no_io  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_no_io.py  ${CMAKE_SOURCE_DIR})
add_test(NAME check_ip     COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_ip.py     ${CMAKE_SOURCE_DIR})
add_test(NAME check_no_stubs COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_no_stubs.py ${CMAKE_SOURCE_DIR})
add_test(NAME check_freeze COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_freeze.py ${CMAKE_SOURCE_DIR})
add_test(NAME perf COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_perf.py $<TARGET_FILE:refractor_bench>)
set_tests_properties(perf PROPERTIES LABELS "perf")
