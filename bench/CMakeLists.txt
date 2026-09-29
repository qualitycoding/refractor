# FROZEN — DO NOT MODIFY (see tests/FROZEN_MANIFEST.sha256)
add_executable(refractor_bench bench_main.cpp)
target_link_libraries(refractor_bench PRIVATE refractor_dsp)
