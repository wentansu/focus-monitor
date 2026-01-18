###########################################################
# CMakeLists.txt
# Created by Greg on 8/19/2024.
# Copyright (C) 2024 Presage Security, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
###########################################################

set(LIBRARY_NAME Container)

set(LIBRARY_SOURCES
        container.cpp
        background_container.cpp
        foreground_container.cpp
        operation_context.cpp
        benchmarking.cpp
        initialization.cpp
        image_transfer.cpp
        keyboard_input.cpp
        output_stream_poller_wrapper.cpp
        json_file_io.cpp
        settings.cpp
)

if (SMART_SPECTRA_LOCAL_BUILD)
    list(APPEND LIBRARY_SOURCES custom_rest_settings.cpp)
endif()

set(LIBRARY_PRIVATE_HEADERS
        background_container_impl.hpp
        container_impl.hpp
        foreground_container_impl.hpp
        initialization_impl.hpp
        initialization.hpp
        image_transfer.hpp
        keyboard_input.hpp
        json_file_io.hpp
        packet_helpers.hpp
        benchmarking.hpp

        ${CMAKE_CURRENT_BINARY_DIR}/configuration.hpp
)

if (SMART_SPECTRA_LOCAL_BUILD)
    list(APPEND LIBRARY_PRIVATE_HEADERS custom_rest_settings.hpp)
endif()

if (SMART_SPECTRA_LOCAL_BUILD)
    set(PHYSIOLOGY_EDGE_MODEL_DIRECTORY ${${PROJECT_NAME_UPPER}_PARENT_DIR}/edge/graph/models)
    set(PHYSIOLOGY_EDGE_GRAPH_DIRECTORY ${PHYSIOLOGY_EDGE_CPACKAGE_INSTALL_LOCATION}/share)
    set(ENABLE_CUSTOM_SERVER ON)
endif ()
configure_file(configuration.hpp.in configuration.hpp)

set(LIBRARY_PUBLIC_HEADERS
        container.hpp
        background_container.hpp
        foreground_container.hpp
        settings.hpp
        operation_context.hpp
        output_stream_poller_wrapper.hpp
)

add_library(${LIBRARY_NAME} STATIC)
add_library(SmartSpectra::Container ALIAS ${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
        PRIVATE ${LIBRARY_SOURCES} ${LIBRARY_PRIVATE_HEADERS}
        PUBLIC FILE_SET HEADERS FILES ${LIBRARY_PUBLIC_HEADERS} BASE_DIRS ${PROJECT_SOURCE_DIR}
)

if (SMART_SPECTRA_LOCAL_BUILD)
    target_sources(${LIBRARY_NAME} PRIVATE custom_rest_settings.cpp)
endif ()

target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${LIBRARY_NAME} PUBLIC SmartSpectra::VideoSource)
target_link_libraries(${LIBRARY_NAME} PUBLIC Physiology::Edge)

if (NOT APPLE)
    target_link_libraries(${LIBRARY_NAME} PRIVATE OpenGL::GL OpenGL::GLES3)
endif ()

install(TARGETS ${LIBRARY_NAME}
        EXPORT ${PROJECT_NAME}Targets
        FILE_SET HEADERS
)




