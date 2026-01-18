//
// Created by greg on 2/28/24.
// Copyright (c) 2024 Presage Technologies
//

#pragma once
// === standard library includes (if any) ===
#include <string>
#include <regex>
#include <filesystem>
#include <map>
// === third-party includes (if any) ===
#include <absl/status/statusor.h>
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <mediapipe/framework/port/opencv_imgcodecs_inc.h>
// === local includes (if any) ===
#include <smartspectra/video_source/video_source.hpp>


namespace presage::smartspectra::video_source::file_stream {

class FileStreamVideoSource : public VideoSource {
public:
    absl::Status Initialize(const VideoSourceSettings& settings);

    [[nodiscard]] bool SupportsExactFrameTimestamp() const override;

    [[nodiscard]] int64_t GetFrameTimestamp() const override;

    int GetWidth() override;
    int GetHeight() override;
protected:
    void ProducePreTransformFrame(cv::Mat& frame) override;
private:
    const int64_t kTimestampNotYetSet = -1;

    // static
    static absl::StatusOr<std::regex> BuildFrameFileNameRegex(const std::string& wildcard_filename_mask);

    std::map<int64_t, std::filesystem::path> ScanInputDirectory();

    // parameters
    std::regex frame_filename_regex;
    std::filesystem::path directory;
    std::string end_of_stream_filename;
    int retry_delay_ms = 10;
    bool erase_read_files;
    bool loop;

    // state
    int64_t  i_frame = 0;
    int64_t current_frame_timestamp = kTimestampNotYetSet;
    bool end_of_stream_encountered = false;
    // only used in loop mode
    std::map<int64_t, std::filesystem::path> loop_frame_filenames;
    std::map<int64_t, std::filesystem::path>::iterator current_frame_data;
    std::filesystem::path end_of_stream_path;

    int first_frame_width = -1;
    int first_frame_height = -1;
};


} // namespace presage::smartspectra::video_source::file_stream
