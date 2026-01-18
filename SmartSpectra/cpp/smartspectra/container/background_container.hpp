//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// background_container.h
// Created by Greg on 4/29/2024.
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
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_core_inc.h>
// === local includes (if any) ===
#include "container.hpp"


namespace presage::smartspectra::container {

template<
    platform_independence::DeviceType TDeviceType,
    settings::OperationMode TOperationMode,
    settings::IntegrationMode TIntegrationMode
>
/**
 * @brief Container for background thread processing.
 * \ingroup container
 */
class BackgroundContainer : public Container<TDeviceType, TOperationMode, TIntegrationMode> {
public:
    typedef container::settings::Settings<TOperationMode, TIntegrationMode> SettingsType;
    using Base = Container<TDeviceType, TOperationMode, TIntegrationMode>;

    /** Construct a background container with the provided settings. */
    explicit BackgroundContainer(SettingsType settings);

    /** Check if the MediaPipe graph is currently running. */
    bool GraphIsRunning() const;
    bool ContainerIsInitialized() const { return this->initialized; };

    /** Initialize the container and prepare the graph. */
    absl::Status Initialize() override;

    /** Start execution of the MediaPipe graph. */
    absl::Status StartGraph();

    /** Block until the graph has finished processing. */
    absl::Status WaitUntilGraphIsIdle();

    /** Toggle recording state within the graph. */
    absl::Status SetRecording(bool on);

    /** Feed a frame into the graph with an explicit timestamp. */
    absl::Status AddFrameWithTimestamp(const cv::Mat& frame_rgb, int64_t frame_timestamp_μs);

    /** Register callback invoked with Bluetooth timestamps from the graph. */
    absl::Status SetOnBluetoothCallback(std::function<absl::Status(double)> on_bluetooth);

    /** Register callback invoked for each output frame from the graph. */
    absl::Status SetOnOutputFrameCallback(std::function<absl::Status(cv::Mat&)> on_output_frame);

    /** Return last observed status code from preprocessing. */
    physiology::StatusCode GetStatusCode() const { return previous_status_code; };

    /** Stop graph execution and clean up resources. */
    absl::Status StopGraph();

private:
    physiology::StatusCode previous_status_code = physiology::StatusCode::PROCESSING_NOT_STARTED;
};

typedef BackgroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Spot, settings::IntegrationMode::Rest> CpuSpotRestBackgroundContainer;
typedef BackgroundContainer<platform_independence::DeviceType::OpenGl, settings::OperationMode::Spot, settings::IntegrationMode::Rest> OpenGlSpotRestBackgroundContainer;
typedef BackgroundContainer<platform_independence::DeviceType::Cpu, settings::OperationMode::Continuous, settings::IntegrationMode::Grpc> CpuContinuousGrpcBackgroundContainer;

template<platform_independence::DeviceType TDeviceType>
using SpotRestBackgroundContainer = BackgroundContainer<TDeviceType, settings::OperationMode::Spot, settings::IntegrationMode::Rest>;

} // namespace presage::smartspectra::container
