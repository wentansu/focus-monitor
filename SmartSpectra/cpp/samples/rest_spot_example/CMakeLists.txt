set(EXECUTABLE_NAME rest_spot_example)

add_executable(${EXECUTABLE_NAME} main.cc)

target_link_libraries(${EXECUTABLE_NAME}
        SmartSpectra::Container
)

if (INSTALL_SAMPLES)
    install(TARGETS ${EXECUTABLE_NAME}
            EXPORT ${PROJECT_NAME}Targets
            FILE_SET HEADERS
    )
endif ()
