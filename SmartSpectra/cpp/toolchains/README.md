# Toolchain File Management
Currently, we have to pair the toolchain files for each architecture with slightly modified "_externals" toolchain file (with that exact postfix). The only difference is that the "_externals" toolchain *should not set* `CMAKE_SYSTEM_PREFIX_PATH` or `CMAKE_STAGING_PREFIX` variables.

The `CMAKE_SYSTEM_PREFIX_PATH` and `CMAKE_STAGING_PREFIX` variables have to be explicitly set to the target platform
in the default toolchain files to control where things get installed during cross-compilation (i.e. arm64).

# Note re: ExternalProject_add Calls

The `ABSOLUTE_EXTERNALS_TOOLCHAIN_FILE` variable points to the "_externals" toolchain and is supplied as value of the `CMAKE_TOOLCHAIN_FILE:FILEPATH` variable under the CMAKE_VARS parameter of each `ExternalProject_add` call; this pattern should be followed for any new ExternalProject_add calls throughout the codebase until a better solution presents itself.

## Detailed Rationale.

The installation directories of the external projects are always within the build directory. After they get installed there, the installation results are picked up by other CMake functions and "exported", as needed, for installation with the Physiology code output. Where applicable, this means they will get installed to the `CMAKE_STAGING_PREFIX` that is specified in the "main" toolchain file.

However, if `CMAKE_SYSTEM_PREFIX_PATH` and `CMAKE_STAGING_PREFIX` are set in the toolchain files passed to the `CMAKE_TOOLCHAIN_FILE:FILEPATH` parameters of ExternalProject_add (i.e. if we pass in the default toolchain files there), they interfere with the `<INSTALL_DIR>` specified by any other `ExternalProject_add` arguments, i.e. the location within the build directory where they are actually supposed to end up during the build stage. This is why we need a separate toolchain file for those, until a better solution presents itself.





