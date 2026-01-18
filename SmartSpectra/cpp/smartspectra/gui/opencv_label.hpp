//
// Created by greg on 6/17/25.
// Copyright (c) 2025 Presage Technologies
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
// === standard library includes (if any) ===
#include <string>
// === third-party includes (if any) ===
#include <opencv2/imgproc.hpp>
#include <absl/status/status.h>
// === local includes (if any) ===

namespace presage::smartspectra::gui {

class OpenCvLabel {
public:
    OpenCvLabel(int x, int y, int width, int height, std::string default_text = "", int character_count = 10);
    ~OpenCvLabel() = default;
    absl::Status Render(cv::Mat& image, const std::string& text, cv::Scalar color) const;
    absl::Status Render(cv::Mat& image, cv::Scalar color) const;
private:
    cv::Rect2i label_area;
    double font_scale;
    cv::Point2i text_origin;
    const int font_face = cv::FONT_HERSHEY_DUPLEX;
    std::string default_text;
};

} // namespace presage::smartspectra::gui
