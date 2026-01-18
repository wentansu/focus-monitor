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
# region ======================== Video Interface ======================================================================

set(LIBRARY_NAME VideoInterface)
add_library(${LIBRARY_NAME} STATIC)
add_library(SmartSpectra::VideoInterface ALIAS VideoInterface)

target_sources(${LIBRARY_NAME}
        PRIVATE video_source.cpp resolution_selection_mode.cpp input_transform.cpp input_transformer.cpp
        PUBLIC FILE_SET HEADERS FILES
        video_source.hpp
        settings.hpp
        camera/camera.hpp
        resolution_selection_mode.hpp
        input_transform.hpp
        input_transformer.hpp
        BASE_DIRS ${PROJECT_SOURCE_DIR}
)

target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${LIBRARY_NAME} PUBLIC Physiology::Edge)

install(TARGETS ${LIBRARY_NAME}
        EXPORT ${PROJECT_NAME}Targets
        FILE_SET HEADERS
)
# endregion ============================================================================================================
# region ======================== Video Source =========================================================================
add_subdirectory(camera)
add_subdirectory(file_stream)

set(LIBRARY_NAME VideoSource)

add_library(${LIBRARY_NAME} STATIC)
add_library(SmartSpectra::VideoSource ALIAS ${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
        PRIVATE factory.cpp
        PUBLIC FILE_SET HEADERS FILES factory.hpp BASE_DIRS ${PROJECT_SOURCE_DIR}
)

target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${LIBRARY_NAME} PUBLIC SmartSpectra::VideoSource_Camera SmartSpectra::VideoSource_FileStream)

install(TARGETS ${LIBRARY_NAME}
        EXPORT ${PROJECT_NAME}Targets
        FILE_SET HEADERS
)
# endregion ============================================================================================================
