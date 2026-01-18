###########################################################
# SmartSpectra_dependencies.cmake
# Created by Greg on 9/3/2024.
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
if (NOT SMART_SPECTRA_LOCAL_BUILD)
    find_package(PhysiologyEdge REQUIRED)
endif ()

if (LINUX)
    find_package(V4L REQUIRED)
endif ()

# PhysiologyEdge does not support GPU on Apple machines yet
if (NOT APPLE AND ENABLE_GPU)
    find_package(OpenGL REQUIRED OpenGL GLES3)
endif ()

if (BUILD_TESTS)
    if (USE_SYSTEM_CATCH2)
        find_package(Catch2)
        if (TARGET Catch2::Catch2)
            message(STATUS "Using installed third-party library Catch2")
            set(CATCH2_TARGET "Catch2::Catch2")
        else ()
            message(STATUS "Unable to find third-party library Catch2 installed on system.
            Setting USE_SYSTEM_CATCH2 to OFF and building from source instead.")
            set(USE_SYSTEM_CATCH2 OFF)
        endif ()
    endif ()
    if (NOT USE_SYSTEM_CATCH2)
        include(FetchContent)
        FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.7.0
        )
        FetchContent_MakeAvailable(Catch2)
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
        include(CTest)
        include(Catch)
        set(CATCH2_TARGET "Catch2::Catch2")
    endif ()
endif ()
