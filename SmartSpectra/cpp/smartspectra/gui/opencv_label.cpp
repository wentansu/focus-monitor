//
// Created by greg on 6/17/25.
// Copyright (c) 2025 Presage Technologies
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/deps/status_macros.h>

#include <utility>
// === local includes (if any) ===
#include "opencv_label.hpp"
#include "opencv_element_fits.hpp"

namespace presage::smartspectra::gui {

OpenCvLabel::OpenCvLabel(int x, int y, int width, int height, std::string default_text, int character_count)
    : label_area(x, y, width, height), default_text(std::move(default_text))
{
    std::string template_text;
    if (!this->default_text.empty()) {
        template_text = this->default_text;
    } else {
        template_text = std::string(character_count, '0');
    }
    int baseline = 0;
    auto text_bound_nominal = cv::getTextSize(template_text, this->font_face, 1, 1, &baseline);
    auto width_scale = static_cast<double>(text_bound_nominal.width) / width;
    auto height_scale = static_cast<double>(text_bound_nominal.height) / height;
    this->font_scale = 1.0 / std::max(width_scale, height_scale);
    int baseline_scaled = 0;
    auto text_bound = cv::getTextSize(template_text, this->font_face, this->font_scale, 1, &baseline_scaled);
    int width_padding_sum = width - text_bound.width;
    int height_padding_sum = height - text_bound.height;
    this->text_origin = cv::Point2i(x + width_padding_sum / 2, y + height_padding_sum / 2 + text_bound.height);
}

absl::Status OpenCvLabel::Render(cv::Mat& image, const std::string& text, cv::Scalar color) const {
    MP_RETURN_IF_ERROR(CheckThatElementFitsImage("OpenCvLabel", this->label_area, image));
    cv::putText(image, text, text_origin, font_face, font_scale, std::move(color), 1, cv::LINE_AA);
    return absl::OkStatus();
}

absl::Status OpenCvLabel::Render(cv::Mat& image, cv::Scalar color) const {
    return this->Render(image, default_text, std::move(color));
}

} // namespace presage::smartspectra::gui
