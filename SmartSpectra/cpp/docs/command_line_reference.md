# Command Line Reference for C++ Example Applications {#command_line_reference}

*Note*: The below list includes all physiology_core_process_options for the [C++ Sample Applications](../samples/README.md), which is the most exhaustive and includes almost all physiology_core_process_options for other examples. For other C++ examples, the physiology_core_process_options that don't make sense are explicitly omitted from the implementation, and additional physiology_core_process_options (if any) are documented elsewhere. Run the example of interest with `--help=main` to see the exact subset of supported physiology_core_process_options.

- `--also_log_to_stderr` (If true, log to stderr as well.); default: false; currently: true;
- `--auto_lock` (If true, will try to use auto-exposure before recording and lock exposure when recording starts. If false, doesn't do this automatically.); default: true;
- `--buffer_duration` (Duration of preprocessing buffer in seconds. Recommended values currently are between 0.2 and 1.0. "
  "Shorter values will mean more frequent updates and higher Core processing loads.); default: 0.5;
- `--camera_device_index` (The index of the camera device to use in streaming capture mode.); default: 0;
- `--capture_height_px` (The capture height in pixels. Set to 720 if resolution_selection_mode is set to 'auto' and no resolution range is specified.); default: -1;
- `--capture_width_px` (The capture width in pixels. Set to 1280 if resolution_selection_mode is set to 'auto' and no resolution range is specified.); default: -1;
- `--codec` (Video codec to use in streaming capture mode. Possible values: MJPG, UYVY); default: MJPG;
- `--end_of_stream` (This is the file that will be placed as a token signalling "end of stream" to preprocessing.); default: "end_of_stream";
- `--erase_read_files` (Erase frame image files that were already read in. Incompatible with ``--loop``.); default: true;
- `--file_stream_path` (Path to files in file stream, e.g. "/path/to/files/frame0000000000000.png" The zero padding signifies the digit count in frame timestamp and can be preceded by a non-digit prefix and/or followed by a non-digit postfix. and/or followed by a non-digit postfix and extension. The timestamp is assumed to use whole microseconds as units. The extension is mandatory. Any extension and its corresponding image codec that is supported by the OpenCV dependency is also supported here (commonly, .png and .jpg are among those).); default: "";
- `--file_stream_rescan_delay` (Delay, in milliseconds, before re-scanning the input folder for more frames. Decrease to accommodate faster streaming. Conversely, if input streaming is slow, decreasing the delay will just hog the application.); default: 5;
- `--headless` (If true, no GUI will be displayed.); default: false;
- `--input_video_path` (Full path of video to load. Signifies prerecorded video mode will be used. When not provided, the app will attempt to use a webcam / stream.); default: "";
- `--input_video_time_path` (Full path of video timestamp txt file, where each row represents the timestamp of each frame in milliseconds.); default: "";
- `--interframe_delay` (Delay, in milliseconds, before capturing the next frame: higher values may free up more processing capacity for the graph, i.e. give it more time to process what it already has and drop fewer frames, resulting in more robust output metrics.); default: 20;
- `--loop` (Loop around the folder. Presumes static input, i.e. folder will not be rescanned. Incompatible with ``--erase_read_files``.); default: false;
- `--output_directory` (Path where to save preprocessed analysis data as JSON. If it does not exist, the app will attempt to make one.); default: "out";
- `--print_graph_contents` (If true, print the graph contents.); default: false;
- `--resolution_range` (The resolution range to attempt to use. Possible values: low, mid, high, ultra, 4k, giant, complete); default: unspecified;
- `--resolution_selection_mode` (A flag to specify the resolution selection mode when both a range and exact resolution are specified.Possible values: exact, range); default: auto;
- `--scale_input` (If true, uses input scaling in the ImageTransformationCalculator within the graph.); default: true;
- `--start_time_offset_ms` (Offset, in milliseconds, before capturing the first frame: 0 starts from beginning. 30000 starts at 30s mark. Not functional for streaming mode, as start is disabled until this offset.); default: 0;
- `--start_with_recording_on` (Attempt to switch data recording on at the start (even in streaming mode).); default: false;
- `--status_file_directory_path` (**[File continuous example only]** Path to the directory where to write files with preprocessing status codes. When the argument is assigned a non-empty string with a well-formed path, the status codes will be written only when the status of preprocessing changes. Status codes will be written as empty files named in `<epoch_microsecond>_<status_code>` format, where epoch microsecond is a 16-character zero-padded string holding an unsigned integer value representing the current time, and the status code is a two-character string holding a zero-padded unsigned integer value. E.g. 0000000000000000_00 would be produced by a machine with it's internal clock back in January 1, 1970 that produces a 0 status code while running this application.); default: "out";
- `--passthrough_video` (If true, output video will just use the input video frames directly (see destination documentation), without passing through any processing (which might contain rendered visual content from the graph).); default: false;
- `--verbosity` (Verbosity level -- raise to print more.); default: 1;
