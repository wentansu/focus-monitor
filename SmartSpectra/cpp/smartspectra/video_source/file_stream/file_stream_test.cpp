//
// Created by greg on 3/4/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <regex>
// === third-party includes (if any) ===
#include <mediapipe/framework/port/gtest.h>
#include <mediapipe/framework/tool/status_util.h>
#include <mediapipe/framework/port/opencv_imgproc_inc.h>
#include <absl/status/status.h>
// === local includes (if any) ===
#include <smartspectra/video_source/settings.h>
#include <smartspectra/video_source/file_stream/file_stream.h>
#include <smartspectra/video_source/camera/camera.h>


namespace vs = presage::smartspectra::video_source;
namespace pcam = presage::camera;

namespace fs = std::filesystem;

// Function to extract the number from the string
int64_t ExtractNumberFromString(const std::string& string) {
    std::smatch match;
    // Regular expression to find digits at the end of the string (before the extension)
    std::regex reg("\\d+");

    if (std::regex_search(string, match, reg)) {
        // If a match is found, convert the matching string to an integer
        return std::stoll(match[0].str());
    } else {
        // If no number is found, throw an exception or return a default value
        throw std::runtime_error("No number found in the string");
    }
}

absl::Status CopyPngFiles(const std::string& source_dir, const std::string& dest_dir, int pause_duration_ms) {
    try {
        // Ensure the source directory exists
        if (!fs::exists(source_dir) || !fs::is_directory(source_dir)) {
            return absl::NotFoundError(source_dir + " does not exist or is not a directory");
        }
        // Create the destination directory if it does not exist
        if (!fs::exists(dest_dir)) {
            fs::create_directories(dest_dir);
        }

        std::map<int64_t, std::pair<std::filesystem::path, std::filesystem::path>> file_map;
        bool end_of_tream_found = false;
        // iterate over source frames
        for (const auto& entry: fs::directory_iterator(source_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".png") {
                // Construct the destination path
                auto dest_path = fs::path(dest_dir) / entry.path().filename();
                // Get the frame index
                int64_t frame_index = ExtractNumberFromString(entry.path().stem().string());
                file_map[frame_index] = std::make_pair(entry.path(), dest_path);
            } else if (entry.is_regular_file() && entry.path().stem() == "end_of_stream") {
                end_of_tream_found = true;
            }
        }

        // Iterate over each entry in the source directory in order of increasing frame number and copy the files
        // to the destination
        for (auto mapping: file_map) {
            auto source_path = mapping.second.first;
            auto dest_path = mapping.second.second;
            // Copy the file
            fs::copy(source_path, dest_path, fs::copy_options::overwrite_existing);
            // Pause
            std::this_thread::sleep_for(std::chrono::milliseconds(pause_duration_ms));
        }
        if (end_of_tream_found){
            std::filesystem::path source_path = fs::path(source_dir) / "end_of_stream";
            std::filesystem::path dest_path = fs::path(dest_dir) / "end_of_stream";
            fs::copy(source_path, dest_path, fs::copy_options::overwrite_existing);
        }
        return absl::OkStatus();
    } catch (const fs::filesystem_error& error) {
        return absl::InternalError("Filesystem error: " + std::string(error.what()));
    } catch (const std::exception& exception) {
        return absl::UnknownError("Error: " + std::string(exception.what()));
    }
}


TEST(FileStreamTest, TestFileStreamLoop) {
    std::filesystem::path path_to_test_data = "external/test_data/loop/";
    std::filesystem::path path_to_end_of_stream_previous_run = "external/test_data/loop/end_of_stream";
    if (std::filesystem::exists(path_to_end_of_stream_previous_run)) {
        std::filesystem::remove(path_to_end_of_stream_previous_run);
    }
    std::filesystem::path path_to_end_of_stream = "external/test_data/video_based_stream/end_of_stream";

    vs::VideoSourceSettings settings{
        0,
        vs::ResolutionSelectionMode::Auto,
        -1,
        -1,
        pcam::CameraResolutionRange::Unspecified_EnumEnd,
        pcam::CaptureCodec::MJPG,
        false,
        "",
        path_to_test_data / "frame_0000000000000000.png",
        "end_of_stream",
        0,
        false,
        true,
    };
    vs::file_stream::FileStreamVideoSource file_stream;
    file_stream.Initialize(settings);
    cv::Size expected_size{1200, 1800};


    std::vector<std::filesystem::path> frame_files;
    for (const auto& entry: std::filesystem::directory_iterator{path_to_test_data}) {
        if (entry.path().extension() == ".png") {
            frame_files.push_back(entry.path());
        }
    }
    std::sort(frame_files.begin(), frame_files.end());
    const int frame_count = static_cast<int>(frame_files.size());

    for (int i_frame = 0; i_frame < frame_count; i_frame++) {
        cv::Mat frame;
        file_stream >> frame;
        ASSERT_FALSE(frame.empty());
        ASSERT_EQ(frame.rows, expected_size.height);
        ASSERT_EQ(frame.cols, expected_size.width);
        cv::Mat frame_gt = cv::imread(frame_files[i_frame], cv::IMREAD_UNCHANGED);
        cv::Mat comparison_result;
        cv::bitwise_xor(frame, frame_gt, comparison_result);
        cv::cvtColor(comparison_result, comparison_result, cv::COLOR_BGR2GRAY);
        ASSERT_EQ(cv::countNonZero(comparison_result), 0);
    }

    // check that we're looping around
    cv::Mat frame;
    file_stream >> frame;
    cv::Mat frame_gt = cv::imread(frame_files[0], cv::IMREAD_UNCHANGED);
    cv::Mat comparison_result;
    cv::bitwise_xor(frame, frame_gt, comparison_result);
    cv::cvtColor(comparison_result, comparison_result, cv::COLOR_BGR2GRAY);
    ASSERT_EQ(cv::countNonZero(comparison_result), 0);

    std::filesystem::copy(path_to_end_of_stream, path_to_test_data, std::filesystem::copy_options::overwrite_existing);
    file_stream >> frame;
    ASSERT_TRUE(frame.empty());
}

TEST(FileStreamTest, TestFileStreamWithErasure) {
    std::filesystem::path path_to_test_data_source = "external/test_data/video_based_stream/";
    std::filesystem::path path_to_test_data_target = "external/test_data/temporary_emulated_stream/";
    if (std::filesystem::exists(path_to_test_data_target)) {
        std::filesystem::remove(path_to_test_data_target);
    }
    std::filesystem::path path_to_end_of_stream = "external/test_data/video_based_stream/end_of_stream";

    vs::VideoSourceSettings settings{
        0,
        vs::ResolutionSelectionMode::Auto,
        -1,
        -1,
        pcam::CameraResolutionRange::Unspecified_EnumEnd,
        pcam::CaptureCodec::MJPG,
        false,
        "",
        path_to_test_data_target / "frame_0000000000000000.png",
        "end_of_stream",
        5,
        true,
        false,
    };

    vs::file_stream::FileStreamVideoSource file_stream;
    file_stream.Initialize(settings);
    cv::Size expected_size{1200, 1800};

    absl::Status copy_status;
    const int copy_pause_duration_ms = 10;
    std::thread thread([&copy_status, &path_to_test_data_source, &path_to_test_data_target, copy_pause_duration_ms]() {
        copy_status = CopyPngFiles(path_to_test_data_source, path_to_test_data_target, copy_pause_duration_ms);
    });
    std::vector<std::filesystem::path> frame_files_source;
    for (const auto& entry: std::filesystem::directory_iterator{path_to_test_data_source}) {
        if (entry.path().extension() == ".png") {
            frame_files_source.push_back(entry.path());
        }
    }
    std::sort(frame_files_source.begin(), frame_files_source.end());
    const int frame_count = static_cast<int>(frame_files_source.size());
    std::filesystem::path previous_target_frame_path;

    for (int i_frame = 0; i_frame < frame_count; i_frame++) {
        cv::Mat frame;
        file_stream >> frame;
        if (i_frame > 0) {
            // check that the previously-read frame has been deleted.
            ASSERT_FALSE(std::filesystem::exists(previous_target_frame_path));
        }
        ASSERT_FALSE(frame.empty());
        ASSERT_EQ(frame.rows, expected_size.height);
        ASSERT_EQ(frame.cols, expected_size.width);
        cv::Mat frame_gt = cv::imread(frame_files_source[i_frame], cv::IMREAD_UNCHANGED);
        previous_target_frame_path = path_to_test_data_target / frame_files_source[i_frame].filename();
        cv::Mat comparison_result;
        cv::bitwise_xor(frame, frame_gt, comparison_result);
        cv::cvtColor(comparison_result, comparison_result, cv::COLOR_BGR2GRAY);
        auto differing_pixel_count = cv::countNonZero(comparison_result);
        ASSERT_EQ(differing_pixel_count, 0);
    }

    thread.join();
    ASSERT_TRUE(copy_status.ok());

    cv::Mat frame;
    std::filesystem::copy(path_to_end_of_stream, path_to_test_data_target,
                          std::filesystem::copy_options::overwrite_existing);
    std::this_thread::sleep_for(std::chrono::milliseconds(copy_pause_duration_ms));
    file_stream >> frame;
    ASSERT_TRUE(frame.empty());

    int i_last_frame = frame_count - 1;
    std::stringstream target_frame_path;
    target_frame_path << (path_to_test_data_target / "frame").string() << std::setfill('0') << std::setw(5)
                      << i_last_frame << ".png";
    // check that the very last frame has been deleted.
    ASSERT_FALSE(std::filesystem::exists(target_frame_path.str()));
}
