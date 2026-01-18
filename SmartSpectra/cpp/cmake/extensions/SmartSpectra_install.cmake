###########################################################
# SmartSpectra_install.cmake
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
set(SMART_SPECTRA_PACKAGE_DESCRIPTION "Presage Technologies SmartSpectra C++ SDK is an easy-to-use C++ framework for physiological metric computation from live video / webcams, video files, or image video frames.")

# needed for write_basic_package_version_file
set(PROJECT_VERSION ${SMART_SPECTRA_VERSION})
set(CONFIG_VERSION_FILE ${PROJECT_NAME}ConfigVersion.cmake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_VERSION_FILE}
        COMPATIBILITY AnyNewerVersion
)

# === PkgConfig setup
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/${PROJECT_NAME}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
)

install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR}/pkgconfig
)

# === Target & Config setup
install(
        # Takes care of the Targets file install
        EXPORT ${PROJECT_NAME}Targets
        NAMESPACE SmartSpectra::
        DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR}
)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        @ONLY
)

# === Install Config file itself, the version file, and other supporting cmake files
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_VERSION_FILE}
        DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR}
)

# === Install version header file
install(FILES
        ${SMART_SPECTRA_VERSION_HEADER_PATH}
        DESTINATION ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}/smartspectra
)

# === Install generated configuration header file
install(FILES
        ${PROJECT_BINARY_DIR}/smartspectra/container/configuration.hpp
        DESTINATION ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}/smartspectra/container
)

install(FILES
        ${PROJECT_SOURCE_DIR}/cmake/modules/FindV4L.cmake
        DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKE_DIR}/modules
)
