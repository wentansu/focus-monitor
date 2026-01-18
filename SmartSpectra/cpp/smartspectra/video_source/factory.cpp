//
// Created by greg on 3/1/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/status_macros.h>
// === local includes (if any) ===
#include "factory.hpp"
#include "camera/capture_video_source.hpp"
#include "file_stream/file_stream.hpp"

namespace presage::smartspectra::video_source {

absl::StatusOr<std::unique_ptr<VideoSource>> BuildVideoSource(const VideoSourceSettings& settings) {
    std::unique_ptr<VideoSource> video_source;
    if (!settings.input_video_path.empty()) {
        // if timestamp txt file was provided
        if (!settings.input_video_time_path.empty()) {
            video_source = std::make_unique<capture::CaptureVideoAndTimeStampFile>();
        } else {
            video_source = std::make_unique<capture::CaptureVideoFileSource>();
        }
    } else if (!settings.file_stream_path.empty()) {
        video_source = std::make_unique<file_stream::FileStreamVideoSource>();
    } else {
        video_source = std::make_unique<capture::CaptureCameraSource>();
    }
    MP_RETURN_IF_ERROR(video_source->Initialize(settings));
    return video_source;
}

} // namespace presage::smartspectra::video_source
