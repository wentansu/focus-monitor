# Building SmartSpectra SDK on macOS {#build_macos}

**NOTICE**: If you are interested in using SmartSpectra C++ on macOS systems, please contact support (<[support@presagetech.com](mailto:support@presagetech.com)>). If you are interested in developing iOS/iPadOS applications, or macOS applications built with [Swift](https://www.swift.org/), please use our [Swift SDK](../../swift/README.md).

If you work with a partner of with Presage Technologies and have dependency sources or packages from us for [platforms for which package are not publicly available](../README.md#supported-systems--architectures), you _will_ need to build the SDK from source.

All commands below demonstrate how to build from a terminal (e.g., Applications → Terminal). We encourage you to use IDEs, editors, and other GUI tools that wrap around any of the given terminal commands at your discretion.

## Table of Contents

- [Installing Build Tools](#installing-build-tools)
- [Setting Up Build Dependencies](#setting-up-build-dependencies)
- [Configuring & Building](#configuring--building)
- [CMake Options](#cmake-options)

## Installing Build Tools

1. Git is required (`brew install git` on macOS).
2. CMake 3.27.0 or newer is required.
    - **macOS** developers can install CMake via Homebrew with `brew install cmake`.
3. Ninja 1.10 (or newer) or `make` will work to build the SDK build on macOS.
   - **macOS** developers can install Ninja via Homebrew with `brew install ninja`.
   - `make` should be available after installing Xcode command-line-tools via `xcode-select --install`.

## Setting Up Build Dependencies

Contact support (<[support@presagetech.com](mailto:support@presagetech.com)>) to obtain a partner license, source package, and build instructions. Public packages coming soon.

## Configuring & Building

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

   - Using Make:

       ```shell
       mkdir build
       cd build
       cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SAMPLES=ON ..
       make -j$(sysctl -n hw.ncpu)
       ```

## CMake Options

Adjust CMake build flags in the above `cmake` calls as needed.

- If you don't want to build the examples, change `-DBUILD_SAMPLES=ON` to `-DBUILD_SAMPLES=OFF`.
- For a debug build, change `-DCMAKE_BUILD_TYPE=Release` to `-DCMAKE_BUILD_TYPE=Debug`.
- The [macOS CMake application from Kitware](https://cmake.org/download/) is a graphical counterpart of the command-line `cmake` tool that will display all available CMake options when provided the source (e.g., `SmartSpectra/cpp`) and build (e.g., `SmartSpectra/cpp/build`) directories.
