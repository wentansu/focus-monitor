# Example Applications

## Table of Contents
- [Overview](#overview)
- [Running Example Applications](#running-example-applications)
  - [Command Line Interface](#command-line-interface)
  - [Keyboard Shortcuts](#keyboard-shortcuts)

## Overview

**Note**: all below applications are built together with the SDK at this time, but they can be set up as independent CMake projects if desired.

- [Smart Spectra C++ Rest Continuous Example App](rest_continuous_example): This example app continuously reads from a video stream (connected camera or file), generates vitals output at fixed intervals, and plots that directly on top of the video feed being output to the user. The installed executable file for this example is `rest_continuous_example`.
- [Smart Spectra C++ Rest Spot Example App](rest_spot_example): This example app can process a preset interval (30 seconds by default) of a video stream (connected camera or file) and output vital readings to standard output and a file on disk. The installed executable file for this example is `rest_spot_example`.
- [Smart Spectra C++ Minimal Spot Example App](minimal_rest_spot_example): This example app can process 30 seconds of a video stream (connected camera or file) and output vital readings to standard output. The installed executable file for this example is `minimal_rest_spot_example`.

## Running Example Applications
1. To build the examples, you have a few options: 
   1. [Install](../README.md#installing-prebuilt-sdk-packages-from-debian-repository-ubuntu--linux-mint) the SDK together with the examples.
   2. [build](../README.md#building-the-sdk) the SDK together with the examples.
   3. You can install the SDK and only build the examples using `SmartSpectra/cpp/samples` as your `cmake` source directory.    
2. If you don't have one already, register and obtain a Presage Technologies Physiology API key from https://physiology.presagetech.com/.
3. Run the C++ rest example or any other app (substitute `<YOUR_API_KEY>` with your Physiology REST API key):
   - if you have installed the SDK with the examples:
       ```bash
        rest_continuous_example --also_log_to_stderr \
         --camera_device_index=0 --auto_lock=false --api_key=<YOUR_API_KEY_HERE>
       ```
   - if you have built from source:
       ```bash
       ./cmake-build-release/samples/rest_continuous_example/rest_continuous_example --also_log_to_stderr \
         --camera_device_index=0 --auto_lock=false --api_key=<YOUR_API_KEY_HERE>
       ```
### Command Line Interface
To list and read about all available command line options for an example, pass `--help=main` to terminal command, e.g.:
```bash
    ./cmake-build-release/samples/rest_spot_example/rest_spot_example --help=main
```

- Note: In the above and below commands, substitute the path `/cmake-build-release/samples/rest_spot_example/rest_spot_example` with the path of the example you're interested in and adjust based on whether it is installed or built from source, e.g. `rest_continuous_example` or the like.

To read about a specific command line option, pass `--help=<OPTION_NAME>` to terminal command, e.g.:
```bash
    ./cmake-build-release/samples/rest_spot_example/rest_spot_example --help=use_gpu
```

For a more details, please see the Command Line Reference [here](/smartspectra/cpp/docs/command_line_reference.md).

### Keyboard Shortcuts
During the run of any example, use the following keyboard shortcuts:
- `q` or `ESC`: exit
- `s`: start/stop recording data (**webcam input / streaming** mode only)
- `e`: lock/unlock exposure (**webcam input / streaming** mode only)
- `-` and `=`: decrease or increase exposure (**webcam input / streaming** mode only, and only when exposure is locked)

See documentation for the `--auto_loc` option by passing `--help=auto_loc` for more details about locking the camera exposure.
