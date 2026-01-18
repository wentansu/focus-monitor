//
// Created by greg on 2/12/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <physiology/modules/configuration.h>
// === local includes (if any) ===
#include "initialization_impl.hpp"
#include "initialization.hpp"


namespace presage::smartspectra::container::initialization {
// region ====== CPU ======

// *** Spot ***
template absl::Status InitializeGraph<platform_independence::DeviceType::Cpu, settings::OperationMode::Spot, settings::IntegrationMode::Rest, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Spot, settings::IntegrationMode::Rest>& settings,
    bool binary_graph
);

template absl::Status InitializeGraph<platform_independence::DeviceType::Cpu, settings::OperationMode::Spot, settings::IntegrationMode::Grpc, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Spot, settings::IntegrationMode::Grpc>& settings,
    bool binary_graph
);

// *** Continuous ***
template absl::Status InitializeGraph<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Rest, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Rest>& settings,
    bool binary_graph
);

template absl::Status InitializeGraph<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Grpc, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Grpc>& settings,
    bool binary_graph
);

// *** computing device / video sink ***
template absl::Status InitializeComputingDevice<platform_independence::DeviceType::Cpu, true>(
    mediapipe::CalculatorGraph& graph,
    platform_independence::DeviceContext<platform_independence::DeviceType::Cpu>& device_context
);

template absl::Status InitializeVideoSink<platform_independence::DeviceType::Cpu, true>(
    cv::VideoWriter& stream_writer,
    const cv::Size& input_resolution,
    const std::string& destination,
    const float output_fps,
    settings::VideoSinkMode video_sink_mode
);

template absl::Status InitializeGui<true>(
    const settings::GeneralSettings& settings, const std::string& window_name
);

// endregion ================
// region ===== OpenGL ======
#ifdef WITH_OPENGL
// *** Spot ***
template absl::Status InitializeGraph<platform_independence::DeviceType::OpenGl, settings::OperationMode::Spot, settings::IntegrationMode::Rest, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Spot, settings::IntegrationMode::Rest>& settings,
    bool binary_graph
);

template absl::Status InitializeGraph<platform_independence::DeviceType::OpenGl, settings::OperationMode::Spot, settings::IntegrationMode::Grpc, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Spot, settings::IntegrationMode::Grpc>& settings,
    bool binary_graph
);

// *** Continuous ***
template absl::Status InitializeGraph<platform_independence::DeviceType::OpenGl, settings::OperationMode::Continuous, settings::IntegrationMode::Rest, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Rest>& settings,
    bool binary_graph
);

template absl::Status InitializeGraph<platform_independence::DeviceType::OpenGl, settings::OperationMode::Continuous, settings::IntegrationMode::Grpc, true>(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<settings::OperationMode::Continuous, settings::IntegrationMode::Grpc>& settings,
    bool binary_graph
);

// *** computing device / video sink ***
template absl::Status InitializeComputingDevice<platform_independence::DeviceType::OpenGl, true>(
    mediapipe::CalculatorGraph& graph,
    platform_independence::DeviceContext<platform_independence::DeviceType::OpenGl>& device_context
);

template absl::Status InitializeVideoSink<platform_independence::DeviceType::OpenGl, true>(
    cv::VideoWriter& stream_writer,
    const cv::Size& input_resolution,
    const std::string& destination,
    const float output_fps,
    settings::VideoSinkMode video_sink_mode
);
#endif
// endregion =================
} // presage::smartspectra::container::initialization
