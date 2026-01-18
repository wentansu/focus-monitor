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

set(LIBRARY_NAME VideoSource_Camera)

set(LIBRARY_SOURCES
        camera_absl.cpp
        camera_opencv.cpp
        capture_video_source.cpp
        camera_opencv_resolution.cpp
)

set(LIBRARY_PUBLIC_HEADERS
        camera.hpp
        capture_video_source.hpp
        camera_opencv.hpp
        camera_v4l2.hpp
)

if (HAVE_LINUX_VIDEODEV2_H)
    list(APPEND LIBRARY_SOURCES camera_v4l2.cpp)
    list(APPEND LIBRARY_PUBLIC_HEADERS camera_v4l2.hpp)
endif ()

add_library(${LIBRARY_NAME} STATIC)
add_library(SmartSpectra::VideoSource_Camera ALIAS ${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
        PRIVATE ${LIBRARY_SOURCES}
        PUBLIC FILE_SET HEADERS FILES ${LIBRARY_PUBLIC_HEADERS} BASE_DIRS ${PROJECT_SOURCE_DIR}
)

target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (HAVE_LINUX_VIDEODEV2_H)
    target_link_libraries(${LIBRARY_NAME} PRIVATE v4l2)
endif ()
target_link_libraries(${LIBRARY_NAME} PUBLIC ${PROJECT_NAME}::VideoInterface)

install(TARGETS ${LIBRARY_NAME}
        EXPORT ${PROJECT_NAME}Targets
        FILE_SET HEADERS
)

