//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// container_impl.h
// Created by Greg on 2/16/24.
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
// === configuration header ===
#include "configuration.hpp"
// === third-party includes (if any) ===
#include <mediapipe/framework/port/logging.h>
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <mediapipe/framework/port/opencv_highgui_inc.h>
#include <mediapipe/framework/formats/image_frame.h>
#include <mediapipe/framework/formats/image_frame_opencv.h>
#include <physiology/graph/stream_and_packet_names.h>
// === local includes (if any) ===
#include "container.hpp"
#include "initialization.hpp"
#include "image_transfer.hpp"
#include "packet_helpers.hpp"
#include "benchmarking.hpp"
#include "keyboard_input.hpp"
#include "json_file_io.hpp"


namespace presage::smartspectra::container {

namespace pi = platform_independence;
namespace init = initialization;
namespace keys = keyboard_input;
namespace ph = packet_helpers;
namespace it = image_transfer;
namespace bench = benchmarking;

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
Container<TDeviceType, TOperationMode, TIntegrationMode>::Container(Container::SettingsType settings) :
    settings(std::move(settings)),
    graph(),
    device_context(),
    operation_context(settings.operation),
    status(physiology::BuildStatusValue(physiology::StatusCode::PROCESSING_NOT_STARTED,
           std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::system_clock::now().time_since_epoch()).count()
           )){};


template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::Initialize() {
    if (this->initialized) {
        // Nothing to do.
        return absl::OkStatus();
    }
    // OpenCV version check needed for some video capture functions / video interface registry
    static_assert(CV_MAJOR_VERSION > 4 || (CV_MAJOR_VERSION >= 4 && CV_MINOR_VERSION >= 2),
                  "OpenCV 4.2 or above is required");

    MP_ASSIGN_OR_RETURN(std::filesystem::path graph_path, GetGraphFilePath());
    MP_RETURN_IF_ERROR(
        init::InitializeGraph<TDeviceType>(this->graph,
                                           graph_path.string(),
                                           this->settings,
                                           this->settings.binary_graph)
    );
    MP_RETURN_IF_ERROR(init::InitializeComputingDevice<TDeviceType>(this->graph, this->device_context));

    initialized = true;
    return absl::OkStatus();
}


template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
std::string Container<TDeviceType, TOperationMode, TIntegrationMode>::GetThirdGraphFileSuffix() const {
    return settings::AbslUnparseFlag(TIntegrationMode);
}


template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
std::string Container<TDeviceType, TOperationMode, TIntegrationMode>::GetGraphFilePrefix() const {
    return "metrics";
}

template<platform_independence::DeviceType TDeviceType, settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode>
absl::StatusOr<std::filesystem::path>
Container<TDeviceType, TOperationMode, TIntegrationMode>::GetGraphFilePath(bool binary_graph) const {
    std::string device_type = pi::AbslUnparseFlag(TDeviceType);
    std::string operation_mode = settings::AbslUnparseFlag(TOperationMode);
    std::string third_graph_suffix = this->GetThirdGraphFileSuffix();
    std::string extension = binary_graph ? ".binarypb" : ".pbtxt";
    std::string prefix = this->GetGraphFilePrefix();
    std::filesystem::path graph_directory = PHYSIOLOGY_EDGE_GRAPH_DIRECTORY;
    auto graph_file_path = graph_directory /
                           (prefix + "_" + device_type + "_" + operation_mode + "_" + third_graph_suffix + extension);
    if (this->settings.verbosity_level > 1) {
        LOG(INFO) << "Retrieving graph from path: " << graph_file_path.string();
    }
    return graph_file_path;
}


template<typename TCallback>
absl::Status CheckCallbackNotNull(const TCallback& callback) {
    if (callback == nullptr) {
        return absl::InvalidArgumentError(
            "Callback cannot be nullptr."
        );
    }
    return absl::OkStatus();
}

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::SetOnStatusChange(
    const std::function<absl::Status(physiology::StatusValue)>& on_status_change
) {
    MP_RETURN_IF_ERROR(CheckCallbackNotNull(on_status_change));
    this->OnStatusChange = on_status_change;
    return absl::OkStatus();
}

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::SetOnEdgeMetricsOutput(
    const std::function<absl::Status(const physiology::Metrics&)>& on_edge_metrics_output
) {
    MP_RETURN_IF_ERROR(CheckCallbackNotNull(on_edge_metrics_output));
    this->OnEdgeMetricsOutput = on_edge_metrics_output;
    return absl::OkStatus();
}

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::SetOnCoreMetricsOutput(
    const std::function<absl::Status(const physiology::MetricsBuffer&, int64_t input_timestamp)>& on_core_metrics_output
) {
    MP_RETURN_IF_ERROR(CheckCallbackNotNull(on_core_metrics_output));
    this->OnCoreMetricsOutput = on_core_metrics_output;
    return absl::OkStatus();
}

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::SetOnVideoOutput(
    const std::function<absl::Status(cv::Mat&, int64_t)>& on_video_output
) {
    MP_RETURN_IF_ERROR(CheckCallbackNotNull(on_video_output));
    this->OnVideoOutput = on_video_output;
    return absl::OkStatus();
}

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::SetOnFrameSentThrough(
    const std::function<absl::Status(bool, int64_t)>& on_dropped_frame
) {
    MP_RETURN_IF_ERROR(CheckCallbackNotNull(on_dropped_frame));
    this->OnFrameSentThrough = on_dropped_frame;
    return absl::OkStatus();
}

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::SetOnCorePerformanceTelemetry(
    const std::function<absl::Status(double, double, int64_t)>& on_effective_core_fps_output
) {
    MP_RETURN_IF_ERROR(CheckCallbackNotNull(on_effective_core_fps_output));
    this->OnCorePerformanceTelemetry = on_effective_core_fps_output;
    return absl::OkStatus();
}


template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
void Container<TDeviceType, TOperationMode, TIntegrationMode>::AddFrameTimestampToBenchmarkingInfo(const mediapipe::Timestamp& timestamp) {
    if (this->OnCorePerformanceTelemetry.has_value() && this->recording) {
        // Calculate the offset of frame capture time from system time
        if (!offset_from_system_time.has_value()) {
            double current_system_seconds =
                std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
            offset_from_system_time = current_system_seconds - timestamp.Seconds();
        }
        this->frames_in_graph_timestamps.insert(timestamp.Value());
    }
}

/**
 * Computes effective fps if OnEffectiveCoreFpsOutput has been set.
 * Relies on this->frames_in_graph_timestamps with timestamps of every frame put into the graph
 * (AddFrameTimestampToBenchmarkingInfo should be used in child classes at every frame)
 * @param metrics_buffer - last output metrics buffer
 * @return status
 */
template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
absl::Status Container<TDeviceType, TOperationMode, TIntegrationMode>::ComputeCorePerformanceTelemetry(
    const physiology::MetricsBuffer& metrics_buffer
) {
    if (this->OnCorePerformanceTelemetry.has_value()) {
        double current_system_seconds =
            std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();

        auto last_buffer_input_timestamp = metrics_buffer.metadata().frame_timestamp();
        auto last_buffer_input_timestamp_location =
            this->frames_in_graph_timestamps.find(last_buffer_input_timestamp);
        auto first_buffer_input_timestamp = *this->frames_in_graph_timestamps.begin();

        // want to be using buffer frame count further (which is captured during send/receive),
        // NOT last_output_timestamp_loc - this->frames_in_graph_timestamps.begin(),
        // because some input frames may have been dropped.

        // erase all frames associated with this buffer (even dropped ones)
        this->frames_in_graph_timestamps.erase(this->frames_in_graph_timestamps.begin(),
                                               last_buffer_input_timestamp_location);

        // compute buffer latency
        double absolute_last_output_system_seconds =
            mediapipe::Timestamp(last_buffer_input_timestamp).Seconds() + offset_from_system_time.value();
        double buffer_latency_seconds = current_system_seconds - absolute_last_output_system_seconds;

        // add buffer benchmarking information to buffer to compute averages later
        this->metrics_buffer_benchmarking_info_buffer.push_back(
            MetricsBufferBenchmarkingInfo{
                first_buffer_input_timestamp,
                last_buffer_input_timestamp,
                metrics_buffer.metadata().frame_count(),
                buffer_latency_seconds
            }
        );

        // clear out benchmarking information from buffer from before the current window (using window duration)
        // (approximate, since we use last output timestamp)
        auto metrics_buffer_benchmarking_info_location =
            this->metrics_buffer_benchmarking_info_buffer.begin();
        auto current_window_start = last_buffer_input_timestamp - this->fps_averaging_window_microseconds;
        while (metrics_buffer_benchmarking_info_location != this->metrics_buffer_benchmarking_info_buffer.end()
               && metrics_buffer_benchmarking_info_location->last_timestamp < current_window_start) {
            metrics_buffer_benchmarking_info_location++;
        }
        if (metrics_buffer_benchmarking_info_location > this->metrics_buffer_benchmarking_info_buffer.begin() + 1) {
            this->metrics_buffer_benchmarking_info_buffer.erase(this->metrics_buffer_benchmarking_info_buffer.begin(),
                                                                metrics_buffer_benchmarking_info_location--);
        }

        // compute total average framerate
        int64_t window_total_microseconds =
            last_buffer_input_timestamp - this->metrics_buffer_benchmarking_info_buffer.begin()->first_timestamp;
        int32_t window_frame_count = 0;
        double aggregate_latency_seconds = 0.0;
        for (const auto& buffer_benchmarking_info: this->metrics_buffer_benchmarking_info_buffer) {
            window_frame_count += buffer_benchmarking_info.frame_count;
            aggregate_latency_seconds += buffer_benchmarking_info.latency_seconds;
        }

        // note: exclude the very last frame from the count, since it's "incomplete" when it's just captured,
        // and the window ends with it being just captured
        double effective_core_fps =
            static_cast<double>(window_frame_count - 1) * 1000000.0 /
            static_cast<double>(window_total_microseconds);

        double effective_core_latency_seconds =
            aggregate_latency_seconds / this->metrics_buffer_benchmarking_info_buffer.size();

        MP_RETURN_IF_ERROR(this->OnCorePerformanceTelemetry.value()(
            effective_core_fps, effective_core_latency_seconds, first_buffer_input_timestamp
        ));

    }
    return absl::OkStatus();
}

} // namespace presage::smartspectra::container
