//
// Created by greg on 2/16/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/logging.h>
// === local includes (if any) ===
#include "benchmarking.hpp"

namespace presage::smartspectra::container::benchmarking {

// === local includes (if any) ===
absl::Status HandleCameraBenchmarking(
    int64_t& i_frame,
    std::chrono::duration<double>& interval_capture_time,
    std::chrono::duration<double>& interval_frame_time,
    std::chrono::time_point<std::chrono::high_resolution_clock> frame_loop_start,
    std::chrono::time_point<std::chrono::high_resolution_clock> frame_capture_end,
    int64_t frame_interval,
    int interframe_delay_ms,
    int verbosity_level
) {
    i_frame++;
    if (verbosity_level > 0) {
        auto frame_iteration_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> frame_capture_duration = frame_capture_end - frame_loop_start;
        std::chrono::duration<double> frame_iteration_duration =
            frame_iteration_end - frame_loop_start - std::chrono::milliseconds(interframe_delay_ms);

        if (verbosity_level > 1) {
            LOG(INFO) << "Current FPS: " << 1.0 / frame_iteration_duration.count();
            if (verbosity_level > 2) {
                LOG(INFO) << "Current frame took " << frame_capture_duration.count() * 1000
                          << " ms to capture (without capture delay).";
                LOG(INFO) << "Current frame took " << frame_iteration_duration.count() * 1000
                          << " ms total to process.";
            }
        } else { // verbosity_level == 1
            interval_capture_time += frame_capture_duration;
            interval_frame_time += frame_iteration_duration;
            if (i_frame % frame_interval == 0) {
                double average_capture_time_s = interval_capture_time.count() / frame_interval;
                double average_frame_iteration_time_s = interval_frame_time.count() / frame_interval;
                LOG(INFO) << "Average FPS over last " << frame_interval << " frames: "
                          << 1.0 / average_frame_iteration_time_s;
                LOG(INFO) << "Average capture time over last " << frame_interval << " frames: "
                          << average_capture_time_s * 1000 << " ms";
                LOG(INFO) << "Average frame processing time over last " << frame_interval
                          << " frames (sans cap. delay): "
                          << average_frame_iteration_time_s * 1000 << " ms";
                // reset
                interval_capture_time = std::chrono::duration<double>(0);
                interval_frame_time = std::chrono::duration<double>(0);
            }
        }
    }
    return absl::OkStatus();
}

} // namespace presage::smartspectra::container::benchmarking
