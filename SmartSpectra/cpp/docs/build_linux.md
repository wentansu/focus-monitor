# Building & Packaging SmartSpectra C++ SDK on Ubuntu / Linux Mint {#build_linux}

This document describes how to build the SmartSpectra SDK from source on Ubuntu 22.04 or Mint 21. Note that on these systems, since installable packages are available, this step is _not_ required if you just want to develop a standalone C++ application using the SDK (see [Supported Systems & Architectures](../README.md#supported-systems--architectures) for more details on supported platforms and package availability). However, building from source allows you to build the SDK examples, understand how SDK source code works, and even contribute your own desired features to the SDK.

All commands below demonstrate how to build from a terminal (e.g., Ctrl+Alt+T by default on Ubuntu or Activities (Super key) → Terminal). We encourage you to use IDEs, editors, and other GUI tools that wrap around any of the given terminal commands at your discretion.

## Prerequisites

Before building from source, ensure you have the following tools available:

### Required Tools

- **GPG**: For verifying package signatures
- **curl**: For downloading repository keys and package lists  
- **CMake**: Version 3.27.0 or higher for building applications

**Note**: If you just want to use the SDK (not build from source), see the main [README](../README.md) for package installation instructions.

## Table of Contents

- [Installing Build Tools](#installing-build-tools)
- [Setting Up Build Dependencies](#setting-up-build-dependencies)
- [Building for the Host System](#building-for-the-host-system)
- [CMake Options](#cmake-options)
- [Cross-compiling for Linux Arm64](#cross-compiling-for-linux-arm64)


## Installing Build Tools

1. Install essential build tools (required for CMAKE_MAKE_PROGRAM and CMAKE_CXX_COMPILER):

   ```shell
   sudo apt update
   sudo apt install -y build-essential git lsb-release libcurl4-openssl-dev libssl-dev pkg-config libv4l-dev libgles2-mesa-dev libunwind-dev
   ```

2. CMake 3.27.0 or newer is required. **Ubuntu 22.04** comes with an older version:

   **Option A: Install from Kitware Repository (Recommended)**

   ```shell
   # Remove old cmake if installed
   sudo apt remove --purge --auto-remove cmake
   
   # Install dependencies
   sudo apt install -y software-properties-common lsb-release wget gnupg
   
   # Download and install Kitware's signing key
   wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
   
   # Add Kitware repository
   echo "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
   sudo apt update
   
   # Install CMake
   sudo apt install cmake
   ```

   **Option B: Direct Download from GitHub**

   ```shell
   # Download and install CMake 3.27.0 directly
   curl -L -o cmake-3.27.0-linux-x86_64.sh https://github.com/Kitware/CMake/releases/download/v3.27.0/cmake-3.27.0-linux-x86_64.sh
   chmod +x cmake-3.27.0-linux-x86_64.sh
   sudo ./cmake-3.27.0-linux-x86_64.sh --skip-license --prefix=/usr/local
   ```

3. Install Ninja (recommended) or verify Make is available:

   ```shell
   # Install Ninja for faster builds
   sudo apt install ninja-build
   
   # Verify tools are installed
   cmake --version  # Should be 3.27.0+
   gcc --version    # Should show GCC compiler
   ninja --version  # Should show Ninja version
   ```

## Setting Up Build Dependencies

Install or build the Physiology Edge library.

_Side Note_: the **Physiology Edge** library, packaged as `libphysiologyedge-dev`, is the C++ backend that _all_ SmartSpectra SDKs rely on to provide a uniform experience across all languages and platforms. This is where some of the computation and communication with Physiology Core API on the cloud takes place. Meanwhile,
 the **SmartSpectra C++ SDK**, packages as `libsmartspectra-dev`, is the C++ open-source library that wraps around the Physiology Edge library to provide a convenient, simple API for C++ developers.

- _Ubuntu 22.04 / Mint 21_: you can install via Debian package [from the Presage Debian Repository](../README.md#setting-up-the-presage-debian-repository). _Note:_ be sure to follow the below directions to only grab what's needed for the build, so that the SDK itself does not get installed from the repository.

1. Update the `apt` database:

    ```shell
    sudo apt update
    ```

2. Install (or upgrade) the Physiology Edge library:

    ```shell
    sudo apt install libphysiologyedge-dev
    ```

- _Other linux systems_: (partners-only) contact support (<[support@presagetech.com](mailto:support@presagetech.com)>) to obtain a source package and build instructions.

## Building for the Host System

1. Clone this repository.

    ```shell
    git clone https://github.com/Presage-Security/SmartSpectra/
     ```

2. Navigate to the C++ SDK root within the cloned repository, e.g.:

    ```shell
    cd SmartSpectra/cpp
    ```

3. Build the SDK with examples using either Ninja or Make.
    - Using Ninja:

        ```shell
        mkdir build
        cd build
        cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON ..
        ninja
        ```

    - Using Make (on macOS, substitute `$(sysctl -n hw.ncpu)` for `$(nproc)` below):

        ```shell
        mkdir build
        cd build
        cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON ..
        make -j$(nproc)
        ```

## CMake Options

Adjust CMake build flags in the above `cmake` calls as needed.

- If you don't want to build the examples, change `-DBUILD_SAMPLES=ON` to `-DBUILD_SAMPLES=OFF`.
- For a debug build, change `-DCMAKE_BUILD_TYPE=Release` to `-DCMAKE_BUILD_TYPE=Debug`.
- The CMake GUI application (`sudo apt install cmake-gui`) is the graphical counterpart of the command-line `cmake` tool that will display all available CMake options when provided the source (e.g., `SmartSpectra/cpp`) and build (e.g., `SmartSpectra/cpp/build`) directories.

### Cross-compiling for Linux Arm64

You can cross-compile the SDK and examples for the Linux `arm64` architecture when building on an `amd64` Linux machine.

1. Install the required cross-compilation tools:

    ```shell
    sudo apt install crossbuild-essential-arm64 gcc-arm-linux-gnueabi binutils-arm-linux-gnueabi
    ```

2. Specify the provided toolchain file to CMake. E.g., from within `SmartSpectra/cpp` (assuming `SmartSpectra` is the root of the repository), run :

    ```shell
    mkdir build-arm64
    cd build-arm64
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON -DCMAKE_TOOLCHAIN_FILE=../toolchains/linux_arm64_toolchain.cmake ..
    make -j$(nproc)
    ```

## Packaging

Use `cpack` to generate the desired package in the build folder. E.g., from within `SmartSpectra/cpp` (assuming `SmartSpectra` is the root of the repository), run:

```shell
cd build
cpack -G DEB
```

For the above example, if there are no errors in the output, you should see a `*.deb` package file appear in the `build` directory.
