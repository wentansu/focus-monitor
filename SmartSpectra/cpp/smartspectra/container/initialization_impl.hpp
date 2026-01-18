//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initialization_impl.h
// Created by Greg on 2/12/2024.
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
#include <string>
#include <regex>
// === third-party includes (if any) ===
#include <physiology/modules/graph_tweaks.h>
#include <physiology/graph/stream_and_packet_names.h>
#include <physiology/modules/geometry.hpp>
#include <mediapipe/framework/port/logging.h>
#include <mediapipe/framework/port/file_helpers.h>
#include <mediapipe/framework/port/status_macros.h>
#include <mediapipe/framework/port/parse_text_proto.h>
#include <mediapipe/framework/calculator.pb.h>
#include <absl/status/statusor.h>
#include <opencv2/highgui.hpp>
// === local includes (if any) ===
#include "initialization.hpp"
#include "configuration.hpp"
#ifdef ENABLE_CUSTOM_SERVER
#include "custom_rest_settings.hpp"
#endif
// @formatter:off
#ifdef __linux__
#include <smartspectra/video_source/camera/camera_v4l2.hpp>
namespace pcam_v4l2 = presage::camera::v4l2;
#endif
// @formatter:on
#include <smartspectra/video_source/camera/camera_opencv.hpp>

namespace presage::smartspectra::container::initialization {

namespace pcam = presage::camera;
namespace pcam_cv = presage::camera::opencv;
namespace pe = physiology::edge;

static void AddGeneralSidePackets(
    std::map<std::string, mediapipe::Packet>& input_side_packets,
    const settings::GeneralSettings& settings
) {
    if (settings.enable_phasic_bp.has_value()) {
        input_side_packets[pe::graph::input_side_packets::kEnablePhasicBp] =
                mediapipe::MakePacket<bool>(settings.enable_phasic_bp.value());
    }
    if (settings.enable_eda.has_value()) {
        input_side_packets[pe::graph::input_side_packets::kEnableEda] =
                mediapipe::MakePacket<bool>(settings.enable_eda.value());
    }
    input_side_packets[pe::graph::input_side_packets::kEnableDenseFaceMeshPoints] =
            mediapipe::MakePacket<bool>(settings.enable_dense_facemesh_points);
    input_side_packets[pe::graph::input_side_packets::kEnableEdgeMetrics] =
            mediapipe::MakePacket<bool>(settings.enable_edge_metrics);
    input_side_packets[pe::graph::input_side_packets::kModelDirectory] =
            mediapipe::MakePacket<std::string>(PHYSIOLOGY_EDGE_MODEL_DIRECTORY);
    if (settings.use_full_range_face_detection.has_value()){
        input_side_packets[pe::graph::input_side_packets::kUseFullRangeFaceDetection] =
            mediapipe::MakePacket<bool>(settings.use_full_range_face_detection.value());
    }
    if (settings.use_full_pose_landmarks.has_value()) {
        input_side_packets[pe::graph::input_side_packets::kUseFullPoseLandmarks] =
            mediapipe::MakePacket<bool>(settings.use_full_pose_landmarks.value());
    }
    if (settings.enable_pose_landmark_segmentation.has_value()) {
        input_side_packets[pe::graph::input_side_packets::kEnablePoseLandmarkSegmentation] =
            mediapipe::MakePacket<bool>(settings.enable_pose_landmark_segmentation.value());
    }
    if (settings.enable_micromotion.has_value()){
        input_side_packets[pe::graph::input_side_packets::kEnableMicromotion] =
            mediapipe::MakePacket<bool>(settings.enable_micromotion.value());
    }
    input_side_packets[pe::graph::input_side_packets::kLogTransferTimingInfo] =
            mediapipe::MakePacket<bool>(settings.log_transfer_timing_info);
}

template<settings::OperationMode TOperationMode, settings::IntegrationMode TIntegrationMode, bool TLog>
inline absl::StatusOr<mediapipe::CalculatorGraphConfig> InitializeGraphConfig(
    const std::string& graph_file_path,
    const settings::Settings<TOperationMode, TIntegrationMode>& settings,
    bool binary_graph
) {
    std::string calculator_graph_config_contents;
    MP_RETURN_IF_ERROR(
        mediapipe::file::GetContents(
            graph_file_path,
            &calculator_graph_config_contents,
            /*read_as_binary=*/binary_graph
        )
    );
    if (TLog) {
        LOG(INFO) << "Scaling input in graph: " << (settings.scale_input ? "true" : "false");
    }
    mediapipe::CalculatorGraphConfig config;
    if (binary_graph) {
        RET_CHECK(config.ParseFromArray(calculator_graph_config_contents.c_str(),
                                        calculator_graph_config_contents.length()));
    } else {
        if (!settings.scale_input) {
            // get rid of input scaling
            presage::graph_tweaks::SetOutputWidthAndHeightToZeroIfPresent(calculator_graph_config_contents);
        }
        if (TLog) {
            if (settings.print_graph_contents) {
                LOG(INFO) << "Get calculator graph config contents: " << calculator_graph_config_contents;
            }
        }
        config = mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(calculator_graph_config_contents);
    }

    config.add_executor();

    return config;
}


template<settings::IntegrationMode TIntegrationMode>
inline absl::Status SupplyGraphIntegrationSettings(
    std::map<std::string, mediapipe::Packet>& input_side_packets,
    const settings::IntegrationSettings<TIntegrationMode>& integration_settings
);

template<>
inline absl::Status SupplyGraphIntegrationSettings(
    std::map<std::string, mediapipe::Packet>& input_side_packets,
    const settings::IntegrationSettings<settings::IntegrationMode::Grpc>& integration_settings
) {
    input_side_packets[pe::graph::input_side_packets::grpc::kGrpcCorePortNumber] = mediapipe::MakePacket<uint16_t>(
        integration_settings.port_number);
    return absl::OkStatus();
}

template<>
inline absl::Status SupplyGraphIntegrationSettings(
    std::map<std::string, mediapipe::Packet>& input_side_packets,
    const settings::IntegrationSettings<settings::IntegrationMode::Rest>& integration_settings
) {
    input_side_packets[pe::graph::input_side_packets::kApiKey] = mediapipe::MakePacket<std::string>(integration_settings.api_key);
    
#ifdef ENABLE_CUSTOM_SERVER
    // Apply custom server configuration if enabled
    if (integration_settings.continuous_server_url.has_value()) {
        settings::CustomServerConfiguration config;
        config.continuous_server_url = integration_settings.continuous_server_url;
        
        MP_RETURN_IF_ERROR(settings::ApplyCustomServerConfig(config));
    }
#endif
    
    return absl::OkStatus();
}

template<typename TSettings>
inline absl::Status InitializeGraphWithConfig(
    mediapipe::CalculatorGraph& graph,
    const mediapipe::CalculatorGraphConfig& config,
    const TSettings& settings
);

template<settings::IntegrationMode TIntegrationMode>
inline absl::Status InitializeGraphWithConfig(
    mediapipe::CalculatorGraph& graph,
    const mediapipe::CalculatorGraphConfig& config,
    const settings::Settings<settings::OperationMode::Continuous, TIntegrationMode>& settings
) {
    std::map<std::string, mediapipe::Packet> input_side_packets;
    AddGeneralSidePackets(input_side_packets, settings);
    input_side_packets[pe::graph::input_side_packets::continuous::kPreprocessedDataBufferDuration] =
        mediapipe::MakePacket<double>(settings.continuous.preprocessed_data_buffer_duration_s);

    const double lower_buffer_duration_threshold_s = 0.2;
    if (settings.continuous.preprocessed_data_buffer_duration_s < lower_buffer_duration_threshold_s &&
        lower_buffer_duration_threshold_s-settings.continuous.preprocessed_data_buffer_duration_s > 1e-6) {
        return absl::InvalidArgumentError(
            "The preprocessed data buffer duration is set to less than "
            + std::to_string(lower_buffer_duration_threshold_s) +
            " seconds. This currently may cause Physiology Core to fail in producing metrics.");
    }
    MP_RETURN_IF_ERROR(SupplyGraphIntegrationSettings(input_side_packets, settings.integration));
    MP_RETURN_IF_ERROR(graph.Initialize(config, input_side_packets));
    return absl::OkStatus();
}

template<settings::IntegrationMode TIntegrationMode>
inline absl::Status InitializeGraphWithConfig(
    mediapipe::CalculatorGraph& graph,
    const mediapipe::CalculatorGraphConfig& config,
    const settings::Settings<settings::OperationMode::Spot, TIntegrationMode>& settings
) {
    std::map<std::string, mediapipe::Packet> input_side_packets;
    input_side_packets[pe::graph::input_side_packets::spot::kSpotDurationS] =
        mediapipe::MakePacket<double>(settings.spot.spot_duration_s);
    AddGeneralSidePackets(input_side_packets, settings);
    MP_RETURN_IF_ERROR(SupplyGraphIntegrationSettings(input_side_packets, settings.integration));
    MP_RETURN_IF_ERROR(graph.Initialize(config, input_side_packets));
    return absl::OkStatus();
}

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode,
    bool TLog
>
absl::Status InitializeGraph(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<TOperationMode, TIntegrationMode>& settings,
    bool binary_graph
) {
    if (TLog) {
        LOG(INFO) << "Initialize the calculator graph.";
        LOG(INFO) << "OpenGl buffers used in graph: "
                  << (TDeviceType == platform_independence::DeviceType::OpenGl ? "true" : "false");
    }
    auto status_or_config =
        InitializeGraphConfig<TOperationMode, TIntegrationMode, TLog>(graph_file_path, settings, binary_graph);

    if (!status_or_config.ok()) {
        return status_or_config.status();
    }
    mediapipe::CalculatorGraphConfig config = status_or_config.value();
    return InitializeGraphWithConfig(graph, config, settings);
}

template<platform_independence::DeviceType TDeviceType>
inline absl::Status InitializeComputingDevice_Internal(
    mediapipe::CalculatorGraph& graph,
    platform_independence::DeviceContext<TDeviceType>& device_context
);

template<>
inline absl::Status InitializeComputingDevice_Internal<platform_independence::DeviceType::Cpu>(
    mediapipe::CalculatorGraph& graph,
    platform_independence::DeviceContext<platform_independence::DeviceType::Cpu>& device_context
) {
    return absl::OkStatus();
}

#ifdef WITH_OPENGL

template<>
inline absl::Status InitializeComputingDevice_Internal<platform_independence::DeviceType::OpenGl>(
    mediapipe::CalculatorGraph& graph,
    platform_independence::DeviceContext<platform_independence::DeviceType::OpenGl>& device_context
) {
    MP_ASSIGN_OR_RETURN(auto gpu_resources, mediapipe::GpuResources::Create());
    MP_RETURN_IF_ERROR(graph.SetGpuResources(std::move(gpu_resources)));
    device_context.gpu_helper.InitializeForTest(graph.GetGpuResources().get());
    return absl::OkStatus();
}

#endif

template<platform_independence::DeviceType TDeviceType, bool TLog>
absl::Status InitializeComputingDevice(
    mediapipe::CalculatorGraph& graph,
    platform_independence::DeviceContext<TDeviceType>& device_context
) {
    if (TLog) {
        LOG(INFO) << "Initialize the compute device.";
    }
    return InitializeComputingDevice_Internal(graph, device_context);
}

static std::string SubstituteVideoSinkTemplate(
    const std::string& template_string,
    const cv::Size& output_resolution,
    const float output_fps
) {
    std::string result = template_string;

    // Create regular expressions for the placeholders
    std::regex width_regex("%width%");
    std::regex height_regex("%height%");
    std::regex fps_regex("%fps%");

    // Perform the substitutions
    result = std::regex_replace(result, width_regex, std::to_string(output_resolution.width));
    result = std::regex_replace(result, height_regex, std::to_string(output_resolution.height));
    result = std::regex_replace(result, fps_regex, std::to_string(output_fps));

    return result;
}

template<platform_independence::DeviceType TDeviceType, bool TLog>
absl::Status InitializeVideoSink(
    cv::VideoWriter& stream_writer,
    const cv::Size& input_resolution,
    const std::string& destination,
    const float output_fps,
    settings::VideoSinkMode video_sink_mode
) {
    if (!destination.empty() && video_sink_mode != settings::VideoSinkMode::Unknown_EnumEnd) {
        if (TLog) {
            LOG(INFO) << "Initialize the video sink.";
        }
        cv::Size output_resolution = input_resolution;
        switch (video_sink_mode) {
            case settings::VideoSinkMode::MJPG:
                stream_writer.open(
                    destination, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                    output_fps, output_resolution, true
                );
                break;
            case settings::VideoSinkMode::GSTREAMER_TEMPLATED:
                stream_writer.open(
                    SubstituteVideoSinkTemplate(destination, output_resolution, output_fps),
                    cv::CAP_GSTREAMER, 0, output_fps, output_resolution, true
                );
                break;
            default:
                return absl::InvalidArgumentError(absl::StrCat("Unsupported video sink mode with int code ",
                                                  static_cast<int>(video_sink_mode)));
        }
        RET_CHECK(stream_writer.isOpened());
    }
    return absl::OkStatus();
}

template<bool TLog>
absl::Status InitializeGui(const settings::GeneralSettings& settings, const std::string& window_name) {
    if (TLog) {
        LOG(INFO) << "Initialize the graphical user interface.";
    }
    // only display when (1) live, OR (2) prerecorded and !headless.  Only permit headless when prerecorded
    if (!settings.headless) {
        cv::namedWindow(window_name, /*flags=WINDOW_AUTOSIZE*/ 1);
    }
    return absl::OkStatus();
}

} // presage::smartspectra::container::initialization
