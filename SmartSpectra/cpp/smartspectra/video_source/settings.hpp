//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// settings.h
// Created by Greg on 2/29/2024.
// Copyright (C) 2024 Presage Security, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
// === local includes (if any) ===
#include "resolution_selection_mode.hpp"
#include "input_transform.hpp"
#include "camera/camera.hpp"


namespace presage::smartspectra::video_source {

/**
 * @brief Configuration options for constructing a VideoSource.
 * \ingroup video_source
 */
struct VideoSourceSettings {
    // === webcam / camera stream, priority #3
    int device_index = 0;
    ResolutionSelectionMode resolution_selection_mode = ResolutionSelectionMode::Range;
    int capture_width_px = -1;
    int capture_height_px = -1;
    camera::CameraResolutionRange resolution_range = camera::CameraResolutionRange::Mid;
    camera::CaptureCodec codec = camera::CaptureCodec::MJPG;
    bool auto_lock = true;
    InputTransformMode input_transform_mode = InputTransformMode::None;

    // === video file, priority #1, unless path empty
    std::string input_video_path;
    std::string input_video_time_path;
    // === file stream, priority #2, unless path empty
    /**
     * path to files in file stream, e.g. "/path/to/files/frame0000000000000.png".
     * @details The zero padding signifies the digit count in frame timestamp and can be preceded by a non-digit prefix
     * and/or followed by a non-digit postfix and extension. The timestamp is assumed to use whole microseconds as units.
     * The extension is mandatory. Any extension and its corresponding image codec that is supported by the OpenCV
     * dependency is also supported here (commonly, .png and .jpg are among those).
     */
    std::string file_stream_path;

    std::string end_of_stream_filename = "end_of_stream";
    int rescan_retry_delay_ms = 10;
    /**
     * erase file(s) that have already been read in as soon as a newer file appears
     */
    bool erase_read_files = true;
    /**
     * loop -- loop around after reaching the maximum frame index in directory.
     * @details loop=true is incompatible with erase_read_files=true argument.
     */
    bool loop = false;
};

} // namespace presage::smartspectra::video_source
