# === Subdirectories ===
if (NOT SMART_SPECTRA_SAMPLE_LOCAL_BUILD)
    cmake_minimum_required(VERSION 3.27.0)
    project(SmartSpectraExamples CXX)
    find_package(SmartSpectra REQUIRED)
endif ()

add_subdirectory(minimal_rest_spot_example)
add_subdirectory(rest_spot_example)
add_subdirectory(rest_continuous_example)




