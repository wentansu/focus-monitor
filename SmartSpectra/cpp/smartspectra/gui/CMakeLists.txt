###########################################################
# CMakeLists.txt
# Created by Greg on 12/13/2024.
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

set(LIBRARY_NAME Gui)
add_library(${LIBRARY_NAME} STATIC)

target_sources(${LIBRARY_NAME}
    PRIVATE
        opencv_trace_plotter.cpp
        confidence_thresholding.cpp
        opencv_value_indicator.cpp
        opencv_hud.cpp
        opencv_element_fits.cpp
        opencv_label.cpp
    PUBLIC FILE_SET HEADERS FILES
        opencv_trace_plotter.hpp
        confidence_thresholding.hpp
        opencv_value_indicator.hpp
        opencv_hud.hpp
        opencv_element_fits.hpp
        opencv_label.hpp
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

add_library(SmartSpectra::Gui ALIAS ${LIBRARY_NAME})
