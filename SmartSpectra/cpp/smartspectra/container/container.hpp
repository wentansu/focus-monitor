//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// container.h
// Created by Greg on 2/16/2024.
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
// === configuration header ===
#include <physiology/modules/configuration.h>
// === standard library includes ===
#include <functional>
#include <filesystem>
// === third-party includes (if any) ===
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <mediapipe/framework/calculator_graph.h>
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <physiology/modules/device_type.h>
#include <physiology/modules/device_context.h>
#include <physiology/modules/messages/status.h>
#include <physiology/modules/messages/metrics.h>
// === local includes (if any) ===
#include "settings.hpp"
#include "operation_context.hpp"

/**
 * @defgroup container Containers
 * @brief Core processing containers used to run SmartSpectra graphs.
 *
 * These classes manage the MediaPipe graph, handle device initialization
 * and provide callbacks for metrics and video output.
 * @{
 */

/** Primary namespace for Container classes and related helpers */
namespace presage::smartspectra::container {

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
/**
 * @brief Base container encapsulating the MediaPipe graph and common callbacks.
 *
 * @tparam TDeviceType    Target device type (CPU/OpenGL/etc).
 * @tparam TOperationMode Operation mode (Spot or Continuous).
 * @tparam TIntegrationMode Integration mode (REST or gRPC).
 */
class Container {
public:
    typedef container::settings::Settings<TOperationMode, TIntegrationMode> SettingsType;

    /**
     * Construct a container with the provided settings.
     */
    explicit Container(SettingsType settings);

    /**
     * Set callback invoked whenever the preprocessing status changes.
     */
    absl::Status SetOnStatusChange(const std::function<absl::Status(physiology::StatusValue)>& on_status_change);

    absl::Status SetOnEdgeMetricsOutput(
        const std::function<absl::Status(const physiology::Metrics&)>& on_edge_metrics_output
    );

    /**
     * Set callback invoked when metrics are produced by the core processing pipeline.
     */
    absl::Status SetOnCoreMetricsOutput(
        const
        std::function<absl::Status(const physiology::MetricsBuffer&, int64_t input_timestamp)>& on_core_metrics_output
    );

    absl::Status SetOnVideoOutput(
        const std::function<absl::Status(cv::Mat& output_frame, int64_t input_timestamp)>& on_video_output
    );

    /**
     * Set callback used for frame drop diagnostics.
     */
    absl::Status SetOnFrameSentThrough(
        const std::function<absl::Status(bool frame_sent_through, int64_t input_timestamp)>& on_dropped_frame
    );

    /**
     * Set callback for benchmarking effective core FPS and latency.
     */
    absl::Status SetOnCorePerformanceTelemetry(
        const std::function<absl::Status(double, double, int64_t)>& on_effective_core_fps_output
    );

    /**
     * Initialize the underlying MediaPipe graph and device context.
     */
    virtual absl::Status Initialize();

protected:
    /** Retrieve the suffix used for the optional third graph file. */
    virtual std::string GetThirdGraphFileSuffix() const;

    /** Prefix used when searching for graph files on disk. */
    virtual std::string GetGraphFilePrefix() const;

    /**
     * Resolve the path to the MediaPipe graph file on disk.
     *
     * @param binary_graph true to use the binary form of the graph
     */
    absl::StatusOr<std::filesystem::path> GetGraphFilePath(bool binary_graph = true) const;

    /** Compute FPS and latency information from a metrics buffer. */
    absl::Status ComputeCorePerformanceTelemetry(const physiology::MetricsBuffer& metrics_buffer);

    /** Track the timestamp of each frame added to the graph for benchmarking. */
    void AddFrameTimestampToBenchmarkingInfo(const mediapipe::Timestamp& timestamp);

// ==== settings
// TODO: maybe figure out how to make `settings` `const` again?
    SettingsType settings;

// ==== state
    mediapipe::CalculatorGraph graph;
// == fixed/static after initialization

    // if needed, set to a callback that handles preprocessing status changes
    std::function<absl::Status(physiology::StatusValue)> OnStatusChange =
        [](physiology::StatusValue status) { return absl::OkStatus(); };

    // if needed, set to a callback that handles new metrics output from edge / local processing
    std::function<absl::Status(const physiology::Metrics&)> OnEdgeMetricsOutput =
        [](const physiology::Metrics&) { return absl::OkStatus(); };

    // if needed, set to a callback that handles new metrics output from core / API
    std::function<absl::Status(const physiology::MetricsBuffer&, int64_t input_timestamp)> OnCoreMetricsOutput =
        [](const physiology::MetricsBuffer&, int64_t input_timestamp) { return absl::OkStatus(); };

    // if needed, set to a callback that handles video frames after they get preprocessed on the edge / in SDK
    std::function<absl::Status(cv::Mat& output_frame, int64_t input_timestamp)> OnVideoOutput =
        [](cv::Mat& output_frame, int64_t input_timestamp) { return absl::OkStatus(); };

    // if needed, set to a callback that handles frame sent-through-graph / dropped-from-graph events,
    // e.g., logs them somewhere.
    std::function<absl::Status(bool frame_sent_through, int64_t input_timestamp)> OnFrameSentThrough =
        [](bool frame_sent_through, int64_t input_timestamp) { return absl::OkStatus(); };

    // for benchmarking
    std::optional<std::function<absl::Status(double fps, double latency_s, int64_t input_timestamp)>>
        OnCorePerformanceTelemetry = std::nullopt;

    platform_independence::DeviceContext<TDeviceType> device_context;
    bool initialized = false;
    bool running = false;
// == dynamic/changing during runtime
    physiology::StatusValue status;
    bool recording = false;

    // for video output (optional)
    cv::Mat output_frame_bgr;
    OperationContext<TOperationMode> operation_context;

private:
    // benchmarking
    std::set<int64_t> frames_in_graph_timestamps;
    const int64_t fps_averaging_window_microseconds = 3 * 1000000; // 3 seconds
    struct MetricsBufferBenchmarkingInfo {
        int64_t first_timestamp = 0;
        int64_t last_timestamp = 0;
        int32_t frame_count = 0;
        double latency_seconds = 0;
    };
    std::vector<MetricsBufferBenchmarkingInfo> metrics_buffer_benchmarking_info_buffer;
    std::optional<double> offset_from_system_time = std::nullopt;
};

} // namespace presage::smartspectra::container
/** @}*/
