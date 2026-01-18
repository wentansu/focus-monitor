set(EXECUTABLE_NAME rest_continuous_example)

add_executable(${EXECUTABLE_NAME} main.cc)

target_link_libraries(${EXECUTABLE_NAME}
        SmartSpectra::Container
        SmartSpectra::Gui
)

# Add build directory to include paths when building locally to find generated configuration.hpp
if (SMART_SPECTRA_SAMPLE_LOCAL_BUILD)
    target_include_directories(${EXECUTABLE_NAME} PRIVATE 
        ${SMART_SPECTRA_BINARY_DIRECTORY}
    )
endif ()

if (INSTALL_SAMPLES)
    install(TARGETS ${EXECUTABLE_NAME}
            EXPORT ${PROJECT_NAME}Targets
            FILE_SET HEADERS
    )
endif ()
