set(LIBRARY_NAME VideoSource_FileStream)

set(LIBRARY_SOURCES
        file_stream.cpp
)

set(LIBRARY_PUBLIC_HEADERS
        file_stream.hpp
)

add_library(${LIBRARY_NAME} STATIC)
add_library(SmartSpectra::VideoSource_FileStream ALIAS ${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
        PRIVATE ${LIBRARY_SOURCES}
        PUBLIC FILE_SET HEADERS FILES ${LIBRARY_PUBLIC_HEADERS} BASE_DIRS ${PROJECT_SOURCE_DIR}
)

target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${LIBRARY_NAME} PUBLIC ${PROJECT_NAME}::VideoInterface)

install(TARGETS ${LIBRARY_NAME}
        EXPORT ${PROJECT_NAME}Targets
        FILE_SET HEADERS
)


