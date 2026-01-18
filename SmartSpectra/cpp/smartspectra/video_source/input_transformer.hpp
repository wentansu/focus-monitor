//
// Created by greg on 6/2/25.
// Copyright (c) 2025 Presage Technologies
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_core_inc.h>
// === local includes (if any) ===
#include "input_transform.hpp"

namespace presage::smartspectra::video_source {

struct InputTransformer{
    InputTransformMode mode = InputTransformMode::None;
    cv::Mat apply(cv::Mat& frame) const;
};

} // namespace presage::smartspectra::video_source
