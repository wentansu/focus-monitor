###########################################################
# SmartSpectra_protobuf.cmake
# Created by Greg on 1/27/2025.
# Copyright (C) 2025 Presage Security, Inc.
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


# Outputs:
#       MAIN_EDGE_INCLUDE       primary (first) include directory of the Edge target
function(SmartSpectra_get_main_edge_include)
    # Edge contains some source protos that we need.
    get_target_property(EDGE_INCLUDES Physiology::Edge HEADER_DIRS)
    list(GET EDGE_INCLUDES 0 MAIN_EDGE_INCLUDE)
    set(MAIN_EDGE_INCLUDE ${MAIN_EDGE_INCLUDE} PARENT_SCOPE)
endfunction()


# Output:
#       PYTHON_IS_VENV           TRUE if the Python executable is running in a virtual environment, FALSE otherwise
#       Python3_EXECUTABLE       Path to the Python executable
function(SmartSpectra_check_python_venv)
    # Find Python
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(Python3_EXECUTABLE ${Python3_EXECUTABLE} PARENT_SCOPE)
    # This small Python script checks if the Python executable is running in a virtual environment
    set(CHECK_VENV_SCRIPT "
import sys
if hasattr(sys, 'real_prefix') or (hasattr(sys, 'base_prefix') and sys.base_prefix != sys.prefix):
    print('venv')
else:
    print('non-venv')
"   )

    # Execute the Python script
    execute_process(
            COMMAND "${Python3_EXECUTABLE}" -c "${CHECK_VENV_SCRIPT}"
            RESULT_VARIABLE RESULT
            OUTPUT_VARIABLE OUTPUT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if("${OUTPUT}" STREQUAL "venv")
        set(PYTHON_IS_VENV TRUE PARENT_SCOPE)
    else ()
        set(PYTHON_IS_VENV FALSE PARENT_SCOPE)
    endif()
endfunction()

# Output: sets BUILD_PYTHON_VENV_BASE to the base directory of the virtual environment
function(SmartSpectra_get_venv_base_directory)
    if(NOT DEFINED Python3_EXECUTABLE)
        message(FATAL_ERROR "Python3_EXECUTABLE is not defined")
    endif()

    # Assuming the virtual environment path is the directory containing the Python executable
    get_filename_component(VENV_PATH ${Python3_EXECUTABLE} DIRECTORY)

    # Run the Python script and capture the output
    execute_process(
            COMMAND bash -c "VIRTUAL_ENV_PATH=${VENV_PATH} ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/cmake/scripts/get_venv_base.py"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            RESULT_VARIABLE GET_VENV_SCRIPT_RESULT
            OUTPUT_VARIABLE GET_VENV_SCRIPT_OUTPUT
            ERROR_VARIABLE GET_VENV_SCRIPT_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Check if the script executed successfully
    if(NOT GET_VENV_SCRIPT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to get the virtual environment base directory: ${GET_VENV_SCRIPT_ERROR}")
    endif()

    # Store the output in the CMake variable
    set(BUILD_PYTHON_VENV_BASE ${GET_VENV_SCRIPT_OUTPUT} PARENT_SCOPE)
endfunction()

# Output: sets PIP_INSTALL_OPTIONS to the options to pass to pip install. Run SmartSpectra_check_python_venv first.
function(SmartSpectra_declare_pip_install_options PYTHON_IS_VENV)
    set(PIP_INSTALL_OPTIONS)
    if (NOT PYTHON_IS_VENV)
        list(APPEND PIP_INSTALL_OPTIONS "--user")
        # If using system-managed pip, assume running as root within docker, so no need to worry about breakage
        list(APPEND PIP_INSTALL_OPTIONS "--break-system-packages")
    endif ()
    set(PIP_INSTALL_OPTIONS ${PIP_INSTALL_OPTIONS} PARENT_SCOPE)
endfunction()

# Define target that generates C++ protobuf files
# Inputs:
#   name:               name of the protobuf (object) library to create, which will generate all protobuf C++ files
#   PROTO_SOURCES:      list of protobuf source files
#   GRPC_PROTO_SOURCES: list of gRPC protobuf source files
# Outputs:
#   TARGET name:        protobuf (object) library; set other libraries to depend on it for it to actually compile the C++ files
function(SmartSpectra_add_cpp_protobuf name)
    cmake_parse_arguments(arg "" "" "PROTO_SOURCES;GRPC_PROTO_SOURCES" ${ARGN})

    # This will be applied ton the generate script template for inclusion of .proto files from Edge
    SmartSpectra_get_main_edge_include()

    set(PROTOC_EXECUTABLE ${HOST_PROTOC_PATH})
    set(GRPC_EXECUTABLE ${HOST_GRPC_PLUGIN_PATH})
    get_filename_component(GRPC_DIRECTORY ${GRPC_EXECUTABLE} DIRECTORY)
    set(PROTOBUF_GENERATION_PATH ${CMAKE_CURRENT_BINARY_DIR}/smartspectra/cpp)
    file(MAKE_DIRECTORY ${PROTOBUF_GENERATION_PATH})

    set(GENERATED_FILES)
    foreach (PROTOBUF_SOURCE_FILE IN LISTS arg_PROTO_SOURCES arg_GRPC_PROTO_SOURCES)
        get_filename_component(PROTO_NAME ${PROTOBUF_SOURCE_FILE} NAME_WE)

        set(GRPC_PLUGIN_OPTIONS "")
        set(OUTPUT_CPP_FILE "${PROTOBUF_GENERATION_PATH}/${PROTO_NAME}.pb.cc")
        set(OUTPUT_H_FILE "${PROTOBUF_GENERATION_PATH}/${PROTO_NAME}.pb.h")
        list(APPEND GENERATED_FILES ${OUTPUT_CPP_FILE} ${OUTPUT_H_FILE})

        if ("${PROTOBUF_SOURCE_FILE}" IN_LIST arg_GRPC_PROTO_SOURCES)
            set(OUTPUT_GRPC_CPP_FILE "${PROTOBUF_GENERATION_PATH}/${PROTO_NAME}.grpc.pb.cc")
            set(OUTPUT_GRPC_H_FILE "${PROTOBUF_GENERATION_PATH}/${PROTO_NAME}.grpc.pb.h")
            list(APPEND GENERATED_FILES ${OUTPUT_GRPC_CPP_FILE} ${OUTPUT_GRPC_H_FILE})
            set(GRPC_PLUGIN_OPTIONS "--grpc_out=${PROTOBUF_GENERATION_PATH} --plugin=protoc-gen-grpc=${GRPC_EXECUTABLE}")
        endif ()

        if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTOBUF_SOURCE_FILE})
            file(RELATIVE_PATH PROTOBUF_SOURCE_FILE_RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${PROTOBUF_SOURCE_FILE})
        else ()
            set(PROTOBUF_SOURCE_FILE_RELATIVE ${PROTOBUF_SOURCE_FILE})
        endif ()

        # Not sure why, but directly using COMMAND doesn't work here -- doesn't find grpc plugin... But works in shell.
        # So we generate a bash script first, then run it.
        configure_file(${PROJECT_SOURCE_DIR}/cmake/templates/generate_cpp_protobuf.sh.in
                ${CMAKE_CURRENT_BINARY_DIR}/generate_${PROTO_NAME}_cpp_proto.sh
                @ONLY
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
        )

        add_custom_command(
                OUTPUT ${OUTPUT_CPP_FILE} ${OUTPUT_H_FILE} ${OUTPUT_GRPC_CPP_FILE} ${OUTPUT_GRPC_H_FILE}
                COMMAND ${CMAKE_CURRENT_BINARY_DIR}/generate_${PROTO_NAME}_cpp_proto.sh
                DEPENDS ${PROTOBUF_SOURCE_FILE}
                COMMENT "Generating C++ protobuf and gRPC files for ${PROTOBUF_SOURCE_FILE}"
        )

    endforeach ()

    if (SMART_SPECTRA_LOCAL_BUILD)
        if (CMAKE_CROSSCOMPILING)
            set(GRPC_DEPENDENCY host_grpc)
            set(PROTOBUF_DEPENDENCY host_protobuf)
        else ()
            set(GRPC_DEPENDENCY external_grpc)
            set(PROTOBUF_DEPENDENCY external_protobuf)
        endif ()
    endif ()

    add_custom_target(${name}_generate ALL DEPENDS ${PROTOBUF_DEPENDENCY} ${GRPC_DEPENDENCY} Physiology::Edge ${GENERATED_FILES})
    add_library(${name} OBJECT ${GENERATED_FILES})

    if (PROJECT_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
        set(PROTO_HEADER_INSTALL_PATH "${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}/smartspectra/cpp")
    else ()
        file(RELATIVE_PATH PROTO_HEADER_INSTALL_PATH_RELATIVE ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
        set(PROTO_HEADER_INSTALL_PATH "${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}/smartspectra/cpp/${PROTO_HEADER_INSTALL_PATH_RELATIVE}")
    endif ()

    target_include_directories(${name} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
            $<INSTALL_INTERFACE:${PROTO_HEADER_INSTALL_PATH}>
    )
    set(GENERATED_HEADERS)
    foreach (GEN_FILE IN LISTS GENERATED_FILES)
        if (GEN_FILE MATCHES ".+\\.h$")
            list(APPEND GENERATED_HEADERS ${GEN_FILE})
        endif ()
    endforeach ()
    install(FILES ${GENERATED_HEADERS}
            DESTINATION ${PROTO_HEADER_INSTALL_PATH}
    )
    # needed to link the protobuf library headers, as well as other generated proto headers that are part of Physiology Edge
    target_link_libraries(${name} PUBLIC Physiology::Edge)
    add_dependencies(${name} ${name}_generate)
endfunction()

# Define target that generates Python protobuf files
# Inputs:
#   name:               name of the protobuf (object) library to create, which will generate all protobuf Python files
#   PACKAGE_DIRECTORY:  package directory for the generated Python package root; defaults to ${CMAKE_CURRENT_BINARY_DIR}
#   PREPEND_PATHS:      list of module paths to prepend w/ "physiology." in the generated Python files, delimited by ":"
#   PROTO_SOURCES:      list of protobuf source files
#   GRPC_PROTO_SOURCES: list of gRPC protobuf source files
# Outputs:
#   TARGET name:        protobuf (object) library; set other libraries to depend on it for it to actually compile the Python files
function(SmartSpectra_add_py_protobuf name)
    cmake_parse_arguments(arg "" "PACKAGE_DIRECTORY;PREPEND_PATHS" "PROTO_SOURCES;GRPC_PROTO_SOURCES" ${ARGN})

    if (NOT arg_PACKAGE_DIRECTORY)
        set(arg_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif ()

    # This will be applied ton the generate script template for inclusion of .proto files from Edge
    SmartSpectra_get_main_edge_include()

    # Path to the requirements.txt file
    set(REQUIREMENTS_TXT_PATH "${PROJECT_SOURCE_DIR}/requirements_grpc.txt")

    # necessary to compile PIP_INSTALL_OPTIONS
    # Python3_EXECUTABLE, PYTHON_IS_VENV =
    SmartSpectra_check_python_venv()

    # PIP_INSTALL_OPTIONS =
    SmartSpectra_declare_pip_install_options(${PYTHON_IS_VENV})

    message(STATUS "Installing Python requirements from ${REQUIREMENTS_TXT_PATH} if necessary...")
    # Command to install Python requirements
    execute_process(
            COMMAND ${Python3_EXECUTABLE} -m pip install ${PIP_INSTALL_OPTIONS} -r ${REQUIREMENTS_TXT_PATH} -qq
            RESULT_VARIABLE pip_result
    )

    # Check if the pip install command was successful
    if(NOT pip_result EQUAL "0")
        message(FATAL_ERROR "Failed to install Python requirements from ${REQUIREMENTS_TXT_PATH}")
    endif()

    set(GENERATED_FILES)
    foreach (PROTOBUF_SOURCE_FILE IN LISTS arg_PROTO_SOURCES arg_GRPC_PROTO_SOURCES)
        get_filename_component(PROTO_NAME ${PROTOBUF_SOURCE_FILE} NAME_WE)

        if (NOT EXISTS ${PROJECT_SOURCE_DIR}/${PROTOBUF_SOURCE_FILE})
            if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${PROTOBUF_SOURCE_FILE})
                file(RELATIVE_PATH PROTOBUF_SOURCE_FILE_RELATIVE ${PROJECT_SOURCE_DIR}
                        ${CMAKE_CURRENT_SOURCE_DIR}/${PROTOBUF_SOURCE_FILE})
            else ()
                message(FATAL_ERROR "Could not find ${PROTOBUF_SOURCE_FILE} in "
                        "either ${PROJECT_SOURCE_DIR} or ${CMAKE_CURRENT_SOURCE_DIR}")
            endif ()
        else ()
            set(PROTOBUF_SOURCE_FILE_RELATIVE ${PROTOBUF_SOURCE_FILE})
        endif ()
        get_filename_component(PROTOBUF_SOURCE_DIRECTORY_RELATIVE ${PROTOBUF_SOURCE_FILE_RELATIVE} DIRECTORY)
        set(OUTPUT_DIRECTORY "${arg_PACKAGE_DIRECTORY}/${PROTOBUF_SOURCE_DIRECTORY_RELATIVE}")

        set(GRPC_PLUGIN_OPTIONS "")
        set(OUTPUT_PY_FILE "${OUTPUT_DIRECTORY}/${PROTO_NAME}_pb2.py")
        set(OUTPUT_PYI_FILE "${OUTPUT_DIRECTORY}/${PROTO_NAME}_pb2.pyi")
        list(APPEND GENERATED_FILES ${OUTPUT_PY_FILE} ${OUTPUT_PYI_FILE})

        if ("${PROTOBUF_SOURCE_FILE}" IN_LIST arg_GRPC_PROTO_SOURCES)
            set(OUTPUT_GRPC_PY_FILE "${OUTPUT_DIRECTORY}/${PROTO_NAME}_pb2_grpc.py")
            list(APPEND GENERATED_FILES ${OUTPUT_GRPC_PY_FILE})
            set(GRPC_PLUGIN_OPTIONS "--grpc_python_out=${arg_PACKAGE_DIRECTORY}")
        endif ()

        # prepend all PREPEND_PATHS with this module
        set(PHYSIOLOGY_PYTHON_MODULE_NAME "physiology")
        # Not sure why, but directly using COMMAND doesn't work here -- doesn't find grpc plugin... But works in shell.
        # So we generate a bash script first, then run it.
        configure_file(${PROJECT_SOURCE_DIR}/cmake/templates/generate_py_protobuf.sh.in
                ${CMAKE_CURRENT_BINARY_DIR}/generate_${PROTO_NAME}_py_proto.sh
                @ONLY
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ
        )

        add_custom_command(
                OUTPUT ${OUTPUT_PY_FILE} ${OUTPUT_PYI_FILE} ${OUTPUT_GRPC_PY_FILE}
                COMMAND ${CMAKE_CURRENT_BINARY_DIR}/generate_${PROTO_NAME}_py_proto.sh
                DEPENDS ${PROTOBUF_SOURCE_FILE}
                COMMENT "Generating Python protobuf and gRPC files for ${PROTOBUF_SOURCE_FILE}"
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
    endforeach ()
    add_custom_target(${name}_generate ALL DEPENDS Physiology::Edge ${GENERATED_FILES})
    add_library(${name} OBJECT ${GENERATED_FILES})
    target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    # needed to link the protobuf library headers, as well as other generated proto headers that are part of Physiology Edge
    target_link_libraries(${name} PUBLIC Physiology::Edge)
    add_dependencies(${name} ${name}_generate)
endfunction()
