set(CMAKE_SYSTEM_NAME "Linux")
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

# requires the following Debian packages on Debian-based systems (use sudo apt install ... if needed):
# crossbuild-essential-arm64 gcc-arm-linux-gnueabi binutils-arm-linux-gnueabi

set(GCC_TRIPLE_PREFIX "aarch64-linux-gnu")
set(GCC_PREFIX "/usr/bin")
set(CMAKE_C_COMPILER ${GCC_PREFIX}/${GCC_TRIPLE_PREFIX}-gcc CACHE FILEPATH "C compiler")
set(CMAKE_CXX_COMPILER ${GCC_PREFIX}/${GCC_TRIPLE_PREFIX}-g++ CACHE FILEPATH "C++ compiler")
set(CMAKE_Fortran_COMPILER ${GCC_PREFIX}/${GCC_TRIPLE}-gfortran CACHE FILEPATH "Fortran compiler")
set(CMAKE_STRIP ${GCC_PREFIX}/${GCC_TRIPLE}-strip CACHE FILEPATH "strip executable")

# Automatically use the cross-wrapper for pkg-config when available.
set(PKG_CONFIG_EXECUTABLE aarch64-linux-gnu-pkg-config CACHE FILEPATH "pkg-config executable")

# *** find_xxx() command behavior: ***
set(CMAKE_FIND_ROOT_PATH "/usr/aarch64-linux-gnu;/usr")

# never search programs (i.e. binaries) in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment only
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE aarch64-linux-gnu)

set(TARGET_ARCHITECTURE "linux_arm64")
