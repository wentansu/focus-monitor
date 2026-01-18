//
// Created by greg on 2/28/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
#include <exception>
#include <filesystem>
#include <map>
#include <string>
#include <thread>
// === third-party includes (if any) ===
#include <mediapipe/framework/port/status_macros.h>
#include <physiology/modules/filesystem_absl.h>
// === local includes (if any) ===
#include "file_stream.hpp"

namespace presage::smartspectra::video_source::file_stream {


void FileStreamVideoSource::ProducePreTransformFrame(cv::Mat& frame) {
    if (this->loop) {
        if (std::filesystem::exists(this->end_of_stream_path)) {
            frame = cv::Mat();
            return;
        }
        frame = cv::imread(current_frame_data->second.string(), cv::IMREAD_UNCHANGED);
        auto next_frame_data = std::next(current_frame_data);
        if (next_frame_data == this->loop_frame_filenames.end()) {
            this->current_frame_data = this->loop_frame_filenames.begin();
        } else {
            this->current_frame_data = next_frame_data;
        }
        this->current_frame_timestamp = this->current_frame_data->first;
        std::this_thread::sleep_for(std::chrono::milliseconds(this->retry_delay_ms));
    } else {
        bool confirmed_next_frame_or_end_of_stream_written = false;

        while (!confirmed_next_frame_or_end_of_stream_written) {
            std::map<int64_t, std::filesystem::path> file_paths = this->ScanInputDirectory();
            int64_t latest_frame_present_timestamp = -1;
            if (file_paths.empty() || (this->current_frame_timestamp == kTimestampNotYetSet && file_paths.size() == 1)) {
                // folder empty or only the first frame being written, wait and retry
                std::this_thread::sleep_for(std::chrono::milliseconds(this->retry_delay_ms));
            } else {
                latest_frame_present_timestamp = std::prev(file_paths.end())->first;
                std::map<int64_t, std::filesystem::path>::iterator frame_data_pair;

                if (this->current_frame_timestamp == kTimestampNotYetSet) {
                    // just began processing, first frame not guaranteed to have 0 timestamp,
                    // start with first frame found -- so long as there is more than one frame written
                    frame_data_pair = file_paths.begin();
                } else {
                    frame_data_pair = std::next(file_paths.find(this->current_frame_timestamp));
                }

                if (frame_data_pair != file_paths.end()) {
                    // If we have frames left, it's safe to assume the current one was written entirely
                    // and can be read. Likewise for end_of_stream token, which is assumed to have been written after all
                    // the frames (if any).
                    if (latest_frame_present_timestamp > frame_data_pair->first || end_of_stream_encountered) {
                        this->current_frame_timestamp = frame_data_pair->first;
                        frame = cv::imread(frame_data_pair->second.string(), cv::IMREAD_UNCHANGED);
                        confirmed_next_frame_or_end_of_stream_written = true;
                    } else {
                        // need to wait to read current frame.
                        std::this_thread::sleep_for(std::chrono::milliseconds(this->retry_delay_ms));
                    }
                } else if (end_of_stream_encountered) {
                    // no more frames left, but end_of_stream token:
                    // write empty cv::Mat to signify end of stream, erase the token file.
                    frame = cv::Mat();
                    // have to max out the current timestamp for erasure to work correctly
                    this->current_frame_timestamp = std::numeric_limits<int64_t>::max();
                    if (this->erase_read_files) {
                        // erase end-of-stream marker for good measure
                        std::filesystem::remove(this->end_of_stream_path);
                    }
                    confirmed_next_frame_or_end_of_stream_written = true;
                } else {
                    // no frames or end-of-stream token in folder yet
                    std::this_thread::sleep_for(std::chrono::milliseconds(this->retry_delay_ms));
                }

                if (this->erase_read_files) {
                    // erase everything up to the current frame
                    for (auto frame_data_pair = file_paths.begin();
                         frame_data_pair != file_paths.end() && frame_data_pair->first < this->current_frame_timestamp;
                         frame_data_pair++) {
                        std::filesystem::remove(frame_data_pair->second);
                    }
                }
            }
        }
    }
}

absl::StatusOr<std::regex> FileStreamVideoSource::BuildFrameFileNameRegex(const std::string& wildcard_filename_mask) {
    std::regex wildcard_mask_parse("([^0-9]+)?([0-9]+)([^0-9]+)?[.](.+)");
    std::cmatch match;
    if (!std::regex_match(wildcard_filename_mask.c_str(), match, wildcard_mask_parse)) {
        return absl::InvalidArgumentError(
            "Invalid wildcard filename mask: " + wildcard_filename_mask +
            ". Expected the filename mask to be in following form: <optional_prefix>0[0...]<optional_postfix>.<extension>"
        );
    }
    std::string prefix = match[1].str();
    size_t number_of_digits = match[2].str().size();
    std::string postfix = match[3].str();
    std::string extension = match[4].str();
    return std::regex(prefix + "([0-9]{" + std::to_string(number_of_digits) + "})" + postfix + "[.]" + extension);
}

bool FileStreamVideoSource::SupportsExactFrameTimestamp() const {
    return false;
}

int64_t FileStreamVideoSource::GetFrameTimestamp() const {
    return this->current_frame_timestamp;
}

absl::Status FileStreamVideoSource::Initialize(const VideoSourceSettings& settings) {
    MP_RETURN_IF_ERROR(VideoSource::Initialize(settings));
    MP_ASSIGN_OR_RETURN(
        this->frame_filename_regex,
        BuildFrameFileNameRegex(std::filesystem::path(settings.file_stream_path).filename())
    );
    this->directory = std::filesystem::path(settings.file_stream_path).parent_path();
    this->end_of_stream_filename = settings.end_of_stream_filename;
    this->retry_delay_ms = settings.rescan_retry_delay_ms;
    this->erase_read_files = settings.erase_read_files;
    this->end_of_stream_path = this->directory / this->end_of_stream_filename;
    this->loop = settings.loop;
    if (settings.erase_read_files && settings.loop) {
        return absl::InvalidArgumentError("Cannot erase read files when looping.");
    }
    MP_RETURN_IF_ERROR(filesystem::abseil::CreateDirectoryIfMissing(this->directory));
    std::filesystem::path first_frame_path;
    if (loop) {
        this->loop_frame_filenames = ScanInputDirectory();
        this->current_frame_data = this->loop_frame_filenames.begin();
        if (!this->loop_frame_filenames.empty()) {
            first_frame_path = this->loop_frame_filenames.begin()->second;
        }
    } else {
        auto file_paths = ScanInputDirectory();
        if (!file_paths.empty()) {
            first_frame_path = file_paths.begin()->second;
        }
    }

    if (!first_frame_path.empty()) {
        cv::Mat first_frame = cv::imread(first_frame_path.string(), cv::IMREAD_UNCHANGED);
        this->first_frame_width = first_frame.cols;
        this->first_frame_height = first_frame.rows;
    }
    return absl::OkStatus();
}

std::map<int64_t, std::filesystem::path> FileStreamVideoSource::ScanInputDirectory() {
    std::map<int64_t, std::filesystem::path> file_paths;

    // Iterate over files in directory,
    // guarantee frame sorting by frame timestamp & check for end of stream token via map
    for (auto const& entry: std::filesystem::directory_iterator{this->directory}) {
        std::smatch match;
        std::string filename = entry.path().filename().string();
        if (!this->end_of_stream_encountered && filename == this->end_of_stream_filename) {
            this->end_of_stream_encountered = true;
            continue;
        }
        if (entry.is_regular_file() && std::regex_match(filename, match, this->frame_filename_regex)) {
            int64_t i_frame = std::stoll(match[1].str());
            file_paths[i_frame] = entry.path();
        }
    }
    return file_paths;
}

int FileStreamVideoSource::GetWidth() {
    return this->first_frame_width;
}

int FileStreamVideoSource::GetHeight() {
    return this->first_frame_height;
}

} // namespace presage::smartspectra::video_source::file_stream
