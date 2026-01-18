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
set(PROJECT_NAME SmartSpectra)
string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)

# === Project Basic Configuration & Language Standards ===
cmake_minimum_required(VERSION 3.27.0)
project(${PROJECT_NAME} CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/extensions")

set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard to build with")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build unit tests." ON)
option(SKIP_DEPS "Try to skip dependency discovery & installation (Useful for Doc-only builds)." OFF)
option(BUILD_DOCS "Build documentation." OFF)
option(BUILD_SAMPLES "Build examples." ON)
option(INSTALL_SAMPLES "Install examples." ON)
option(USE_SYSTEM_CATCH2 "Use Catch2 library installed on system instead of downloading and building from source." OFF)
option(ENABLE_GPU "Enable GPU support." ON)

if (BUILD_TESTS)
    enable_testing()
endif ()

if (CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_CROSSCOMPILING TRUE)
else ()
    set(CMAKE_CROSSCOMPILING FALSE)
endif ()

set(SMART_SPECTRA_SAMPLE_LOCAL_BUILD TRUE)
set(SMART_SPECTRA_BINARY_DIRECTORY ${PROJECT_BINARY_DIR})

include(${PROJECT_NAME}_version)
# (SMART_SPECTRA_VERSION, SMART_SPECTRA_VERSION_PLAIN,
#  SMART_SPECTRA_VERSION_MAJOR, SMART_SPECTRA_VERSION_MINOR, SMART_SPECTRA_VERSION_PATCH
#  SMART_SPECTRA_VERSION_STATUS) =
SmartSpectra_get_version_from_py_file(SMART_SPECTRA "${PROJECT_SOURCE_DIR}/version.py")

# Generate version header file
SmartSpectra_generate_version_header(SMART_SPECTRA)

# === Toolchain Configuration ==
if (NOT SMART_SPECTRA_LOCAL_BUILD)
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/toolchains")
    include(pick_architecture)
endif ()

# === Installation Paths ==
set(CMAKE_STATIC_LIBRARY_PREFIX_CXX "lib${PROJECT_NAME}")
if (UNIX OR CYGWIN)
    include(GNUInstallDirs)
    set(${PROJECT_NAME}_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
    set(${PROJECT_NAME}_INSTALL_BIN_DIR "${CMAKE_INSTALL_BINDIR}")
    set(${PROJECT_NAME}_INSTALL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")
    set(${PROJECT_NAME}_INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
else ()
    set(PROJECT_EXTERNAL_INCLUDE_DIRECTORY include)
    set(${PROJECT_NAME}_INSTALL_INCLUDE_DIR include/${PROJECT_NAME_LOWER})
    set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
    set(${PROJECT_NAME}_INSTALL_LIB_DIR lib)
    set(${PROJECT_NAME}_INSTALL_CMAKE_DIR lib/cmake)
endif ()
set(PROJECT_INSTALL_INCLUDE_DIRECTORY ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR})
cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH ${PROJECT_NAME_UPPER}_ROOT_DIR)
cmake_path(GET ${PROJECT_NAME_UPPER}_ROOT_DIR PARENT_PATH ${PROJECT_NAME_UPPER}_PARENT_DIR)

if (APPLE AND SMART_SPECTRA_LOCAL_BUILD)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_SKIP_INSTALL_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_BUILD_RPATH ${PHYSIOLOGY_EDGE_CPACKAGE_LIBRARY_DIRECTORY})
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

# === Dependencies ===
include(cmake/extensions/SmartSpectra_dependencies.cmake)

# === Subdirectories / Targets ===
add_subdirectory(smartspectra)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lab")
    option(BUILD_LAB "Build lab" ON)
    if (BUILD_LAB)
        add_subdirectory(lab)
    endif ()
endif()

set(HAS_ON_PREM FALSE)
# TODO: find a way to build OnPrem without a local build
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/on_prem")
    set(HAS_ON_PREM TRUE)
    option(BUILD_ON_PREM "Build OnPrem." ON)
    if (BUILD_ON_PREM)
        add_subdirectory(on_prem)
    endif ()
endif ()

if (BUILD_TESTS)
    include(cmake/extensions/SmartSpectra_testing.cmake)
    add_subdirectory(tests)
endif ()

if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif ()

# === Installation / Packaging / Uninstallation ===
include(${PROJECT_NAME}_install)
include(${PROJECT_NAME}_uninstall)
# must be used after _version and _install (uses *VERSION* family of variables and *PACKAGE_DESCRIPTION)
include(${PROJECT_NAME}_packaging)

# === Documentation ===
if (BUILD_DOCS)
    include(FetchContent)
    FetchContent_Declare(
            doxygen-awesome-css
            URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)

    # Save the location the files were cloned into
    # This allows us to get the path to doxygen-awesome.css
    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    # Generate the Doxyfile
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    find_package(Doxygen
            REQUIRED dot
            OPTIONAL_COMPONENTS mscgen dia
    )

    doxygen_add_docs(
        doxygen
        ALL
        CONFIG_FILE ${DOXYFILE_OUT}
    )

endif ()
