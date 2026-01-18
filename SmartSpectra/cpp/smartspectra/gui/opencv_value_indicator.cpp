//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_value_indicator.cpp
// Created by greg on 12/16/24.
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

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <opencv2/imgproc.hpp>
#include <mediapipe/framework/deps/status_macros.h>
#include <physiology/modules/geometry.hpp>
// === local includes (if any) ===
#include "opencv_value_indicator.hpp"
#include "opencv_element_fits.hpp"


namespace presage::smartspectra::gui {

const float OpenCvValueIndicator::min_value = 0.0;
const float OpenCvValueIndicator::max_value = 999.9;

/**
 * @param x - left coordinate of the text box
 * @param y - bottom coordinate of the text box
 * @param width - width of the text box
 * @param height - height of the text box
 * @param precision_digits - number of digits after the decimal point
 */
OpenCvValueIndicator::OpenCvValueIndicator(int x, int y, int width, int height, int precision_digits)
    : indicator_area(x, y, width, height), precision_digits(precision_digits) {
    // Construct template_text with the correct number of zeros after the decimal
    std::string template_text = "000." + std::string(this->precision_digits, '0');
    int baseline = 0;
    auto text_bound_nominal = cv::getTextSize(template_text, font_face, 1, 1, &baseline);
    auto width_scale = static_cast<double>(text_bound_nominal.width) / width;
    auto height_scale = static_cast<double>(text_bound_nominal.height) / height;
    this->font_scale = 1.0 / std::max(width_scale, height_scale);
    int baseline_scaled = 0;
    auto text_bound_scaled = cv::getTextSize(template_text, font_face, font_scale, 1, &baseline_scaled);
    int width_padding_sum = width - text_bound_scaled.width;
    int height_padding_sum = height - text_bound_scaled.height;
    text_origin = cv::Point2i(x + width_padding_sum / 2, y + height_padding_sum / 2);
}

absl::Status OpenCvValueIndicator::Render(cv::Mat& image, float value, cv::Scalar color) {
    if (value > OpenCvValueIndicator::max_value || value < OpenCvValueIndicator::min_value) {
        return absl::InvalidArgumentError("Value " + std::to_string(value) + " is outside the supported range [0.0, 999.0].");
    }
    MP_RETURN_IF_ERROR(CheckThatElementFitsImage("OpenCvValueIndicator", this->indicator_area, image));
    std::stringstream text;
    text << std::setprecision(this->precision_digits) << std::fixed << value;
    cv::putText(image, text.str(), text_origin, font_face, font_scale, std::move(color), 1, cv::LINE_AA);
    return absl::OkStatus();
}

absl::Status OpenCvValueIndicator::RenderNA(cv::Mat& image, cv::Scalar color) {
    MP_RETURN_IF_ERROR(CheckThatElementFitsImage("OpenCvValueIndicator", this->indicator_area, image));
    cv::putText(image, "N/A", text_origin, font_face, font_scale, std::move(color), 1, cv::LINE_AA);
    return absl::OkStatus();
}

} // namespace presage::smartspectra::gui
