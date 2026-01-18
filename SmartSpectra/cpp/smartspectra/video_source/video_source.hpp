//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// interface.h
// Created by Greg on 2/29/2024.
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
#include <cstdint>
#include <chrono>
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_core_inc.h>
#include <absl/status/statusor.h>
// === local includes (if any) ===
#include "settings.hpp"
#include "input_transformer.hpp"

/**
 * @defgroup video_source Video Sources
 * @brief Interfaces and helpers for obtaining frames for processing.
 * @{
 */

namespace presage::smartspectra::video_source {

/** Epoch timestamp captured at startup for relative frame timing. */
const int64_t microsecond_epoch_at_start =
    std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

/**
 * @brief Abstract interface for camera/video input sources.
 * \ingroup video_source
 */
class VideoSource {
public:
    /** Grab the next frame from the source. */
    VideoSource& operator>>(cv::Mat& frame);

    /** Configure the source with provided settings. */
    virtual absl::Status Initialize(const VideoSourceSettings& settings);

    virtual ~VideoSource() = default;

    // == timestamp controls
    virtual bool SupportsExactFrameTimestamp() const = 0;

    /**
     * return the current frame's timestamp, in microseconds
     */
    virtual int64_t GetFrameTimestamp() const = 0;

    // These have definitions here, technically making this not a true interface.
    // Ignore this for now, maybe redesign later, (e.g. using C++20 concepts?).
    // == exposure controls
    virtual absl::Status TurnOnAutoExposure();

    virtual absl::Status TurnOffAutoExposure();

    virtual absl::Status ToggleAutoExposure();

    virtual absl::StatusOr<bool> IsAutoExposureOn();

    virtual absl::Status IncreaseExposure();

    virtual absl::Status DecreaseExposure();

    virtual bool SupportsExposureControls();

    virtual int GetWidth();

    virtual int GetHeight();

    virtual InputTransformMode GetDefaultInputTransformMode();

    /** Check if the source has valid frame dimension information. */
    bool HasFrameDimensions();
protected:
    InputTransformer input_transformer;
    virtual void ProducePreTransformFrame(cv::Mat& frame) = 0;
};


} // namespace presage::smartspectra::video_source
/** @}*/
