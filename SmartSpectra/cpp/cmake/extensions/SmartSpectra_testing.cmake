###########################################################
# SmartSpectra_testing.cmake
#
# Based on https://github.com/Algomorph/NeuralTracking/blob/main/cpp/tests/CMakeLists.txt
# by Gregory Kramida (Algomorph), Apache 2 License, made Nov 16 2022 - Jul 19 2023
#
# Added by Gregory Kramida to SmartSpectra C++ SDK on Sep 3, 2024
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

function(smartspectra_add_test)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs DEPENDENCIES SOURCES LIBRARIES)
    cmake_parse_arguments(ADD_TEST "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN})
    list(LENGTH ARGN ARGC)

    if (${ARGC} EQUAL 0)
        message(FATAL_ERROR "smartspectra_add_test needs at least a single positional argument.")
    else ()
        list(GET ARGN 0 name)
    endif ()

    if (ADD_TEST_SOURCES)
        add_executable(${name} ${ADD_TEST_SOURCES})
    else ()
        add_executable(${name} ${name}.cpp)
    endif ()

    target_link_libraries(${name} PUBLIC Physiology::Edge TestUtilities)
    target_link_libraries(${name} PRIVATE Catch2::Catch2)
    target_compile_options(${name} PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda>")
    target_link_options(${name} PRIVATE -no-pie)


    if (ADD_TEST_DEPENDENCIES)
        add_dependencies(${name} ${ADD_TEST_DEPENDENCIES})
    endif ()

    if (ADD_TEST_LIBRARIES)
        target_link_libraries(${name} PUBLIC ${ADD_TEST_LIBRARIES})
    endif ()

    if (UNIX)
        target_link_libraries(${name} PUBLIC rt)
    endif ()

    add_test(NAME cmake_test_${name} COMMAND ${name})
endfunction()


function(smartspectra_add_test_data_download_target)
    set(options)
    set(oneValueArgs URL SHA256 DESTINATION)
    set(multiValueArgs EXTRACTED_NAMES)
    cmake_parse_arguments(DOWNLOAD_DATA "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN})

    list(LENGTH ARGN ARGC)
    if (${ARGC} EQUAL 0)
        message(FATAL_ERROR "smartspectra_download_test_data needs at least a single positional argument.")
    else ()
        list(GET ARGN 0 target_name)
    endif ()

    if (NOT DOWNLOAD_DATA_URL)
        message(FATAL_ERROR "smartspectra_download_test_data needs the URL one-value argument.")
    endif ()
    if (NOT DOWNLOAD_DATA_SHA256)
        message(FATAL_ERROR "smartspectra_download_test_data needs the SHA256 one-value argument.")
    endif ()
    if (NOT DOWNLOAD_DATA_DESTINATION)
        message(FATAL_ERROR "smartspectra_download_test_data needs the DESTINATION one-value argument.")
    endif ()
    if (NOT DOWNLOAD_DATA_EXTRACTED_NAMES)
        message(FATAL_ERROR "smartspectra_download_test_data needs the EXTRACTED_NAMES multi-value argument.")
    endif ()

    cmake_path(SET destination_path ${GENERATED_TEST_DATA_DIRECTORY})
    cmake_path(APPEND destination_path ${DOWNLOAD_DATA_DESTINATION})


    set(byproduct_paths)
    foreach (filename ${DOWNLOAD_DATA_EXTRACTED_NAMES})
        cmake_path(GET destination_path PARENT_PATH extracted_path)
        cmake_path(APPEND extracted_path ${DOWNLOAD_DATA_EXTRACTED_NAMES})
        list(APPEND byproduct_paths "${extracted_path}")
        list(LENGTH byproduct_paths byproduct_paths_length)
    endforeach ()

    list(APPEND byproduct_paths ${destination_path})

    add_custom_target(
            ${target_name}
            BYPRODUCTS ${byproduct_paths}
            COMMAND ${CMAKE_COMMAND}
            -DBYPRODUCTS=${byproduct_paths}
            -DURL=${DOWNLOAD_DATA_URL}
            -DDESTINATION=${destination_path}
            -DSHA256=${DOWNLOAD_DATA_SHA256}
            -P ${PROJECT_SOURCE_DIR}/cmake/extensions/download_and_extract_file.cmake
    )
endfunction()

