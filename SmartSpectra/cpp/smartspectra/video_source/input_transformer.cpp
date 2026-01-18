//
// Created by greg on 6/2/25.
// Copyright (c) 2025 Presage Technologies
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/logging.h>
// === local includes (if any) ===
#include "input_transformer.hpp"

namespace presage::smartspectra::video_source {


cv::Mat InputTransformer::apply(cv::Mat& frame) const {
    if (frame.empty()) {
        return frame;
    }
    switch (mode) {
        case InputTransformMode::None:
            return frame;
        case InputTransformMode::Clockwise90:{
            cv::Mat rotated90cw;
            cv::rotate(frame, rotated90cw, cv::ROTATE_90_CLOCKWISE);
            return rotated90cw;
        }
        case InputTransformMode::Counterclockwise90:{
            cv::Mat rotated90ccw;
            cv::rotate(frame, rotated90ccw, cv::ROTATE_90_COUNTERCLOCKWISE);
            return rotated90ccw;
        }
        case InputTransformMode::Rotate180:{
            cv::Mat rotated180;
            cv::rotate(frame, rotated180, cv::ROTATE_180);
            return rotated180;
        }
        case InputTransformMode::MirrorHorizontal:{
            cv::Mat flipped_h;
            cv::flip(frame, flipped_h, /*flip_code=HORIZONTAL*/1);
            return flipped_h;
        }
        case InputTransformMode::MirrorVertical:{
            cv::Mat flipped_v;
            cv::flip(frame, flipped_v, /*flip_code=VERTICAL*/0);
            return flipped_v;
        }
        default:
            LOG(ERROR) << "Unsupported input transform mode: " << AbslUnparseFlag(mode);
            return frame;
    }
}

} // namespace presage::smartspectra::video_source
