###########################################################
# CMakeLists.txt
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

configure_file(test_data_paths.hpp.in test_data_paths.hpp @ONLY)

add_library(
        TestUtilities STATIC
        test_utilities.hpp
        test_utilities_impl.hpp
        test_utilities.cpp
        compile_time_string_concatenation.hpp
)

set(PARENT_PATH_RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/..)
cmake_path(ABSOLUTE_PATH PARENT_PATH_RELATIVE OUTPUT_VARIABLE _PARENT_PATH)


target_include_directories(TestUtilities PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${_PARENT_PATH})
