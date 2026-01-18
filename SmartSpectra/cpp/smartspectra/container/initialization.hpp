//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initialization.h
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
// === configuration header ===
#include <physiology/modules/configuration.h>
// === third-party includes (if any) ===
#include <absl/status/status.h>
#include <mediapipe/framework/calculator_graph.h>
#include <mediapipe/framework/port/opencv_video_inc.h>
#include <physiology/modules/device_type.h>
#include <physiology/modules/device_context.h>
// === local includes (if any) ===
#include "settings.hpp"
#include <smartspectra/video_source/camera/camera.hpp>

namespace presage::smartspectra::container::initialization {

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode,
    bool TLog = true
>
/**
 * @brief Load and initialize a MediaPipe graph.
 */
absl::Status InitializeGraph(
    mediapipe::CalculatorGraph& graph,
    const std::string& graph_file_path,
    const settings::Settings<TOperationMode, TIntegrationMode>& settings,
    bool binary_graph
);

/**
 * @brief Initialize device-specific compute context.
 */
template<platform_independence::DeviceType TDeviceType, bool TLog = true>
absl::Status InitializeComputingDevice(mediapipe::CalculatorGraph& graph, platform_independence::DeviceContext<TDeviceType>& device_context);

/**
 * @brief Prepare an output video sink.
 */
template<platform_independence::DeviceType TDeviceType, bool TLog = true>
absl::Status InitializeVideoSink(
    cv::VideoWriter& stream_writer,
    const cv::Size& input_resolution,
    const std::string& destination,
    const float output_fps,
    settings::VideoSinkMode video_sink_mode
);

/**
 * @brief Setup optional GUI components used by the foreground container.
 */
template<bool TLog = true>
absl::Status InitializeGui(const settings::GeneralSettings& settings, const std::string& window_name);

} // presage::smartspectra::container::initialization
