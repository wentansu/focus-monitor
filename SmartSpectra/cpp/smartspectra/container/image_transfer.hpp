//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// image_transfer.h
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
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/status/status.h>
#include <mediapipe/framework/formats/image_frame.h>
#include <mediapipe/framework/calculator_framework.h>
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <physiology/modules/device_type.h>
#include <physiology/modules/device_context.h>
// === local includes (if any) ===

namespace presage::smartspectra::container::image_transfer {

/**
 * @brief Send an image frame into the MediaPipe graph.
 */
absl::Status FeedFrameToGraph(
    std::unique_ptr<mediapipe::ImageFrame> input_frame,
    mediapipe::CalculatorGraph& graph,
    const int64_t& frame_timestamp,
    const char* video_stream
);

/**
 * @brief Device-aware version of FeedFrameToGraph.
 */
template<presage::platform_independence::DeviceType TDeviceType>
absl::Status FeedFrameToGraph(
    std::unique_ptr<mediapipe::ImageFrame> input_frame,
    mediapipe::CalculatorGraph& graph,
    presage::platform_independence::DeviceContext<TDeviceType>& device_context,
    const int64_t& frame_timestamp,
    const char* video_stream
);

/** Retrieve a frame from an output packet and convert to RGB. */
absl::Status GetFrameFromPacket(
    cv::Mat& output_frame_rgb,
    const mediapipe::Packet& output_video_packet
);

/**
 * @brief Device-aware version of GetFrameFromPacket.
 */
template<presage::platform_independence::DeviceType TDeviceType>
absl::Status GetFrameFromPacket(
    cv::Mat& output_frame_rgb,
    presage::platform_independence::DeviceContext<TDeviceType>& device_context,
    const mediapipe::Packet& output_video_packet
);


} // namespace presage::smartspectra::container::image_transfer
