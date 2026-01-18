//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_hud.hpp
// Created by greg on 1/3/25.
// Copyright (C) 2025 Presage Security, Inc.
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
#include <opencv2/core.hpp>
#include <physiology/modules/messages/metrics.pb.h>
// === local includes (if any) ===
#include "opencv_trace_plotter.hpp"
#include "opencv_value_indicator.hpp"
#include "opencv_label.hpp"


#pragma once

namespace presage::smartspectra::gui {

class OpenCvHud {
public:
    OpenCvHud(
        int x, int y, int width, int height,
        int max_trace_points = 300,
        cv::Scalar pulse_confident_color = cv::Scalar(0, 255, 0), // green
        cv::Scalar pulse_unconfident_color = cv::Scalar(0, 0, 255), // red
        cv::Scalar breathing_upper_confident_color = cv::Scalar(255, 255, 0), // cyan
        cv::Scalar breathing_upper_unconfident_color = cv::Scalar(0, 0, 255), // red
        cv::Scalar breathing_lower_confident_color = cv::Scalar(255, 0, 0), // blue
        cv::Scalar breathing_lower_unconfident_color = cv::Scalar(0, 0, 255) // red
    );

    void UpdateWithNewMetrics(const physiology::MetricsBuffer& new_metrics);
    absl::Status Render(cv::Mat& image);

    static const int minimal_width; // derived
    static const int minimal_height; // derived

private:
    static const float no_rate_value_to_display;

    static const int top_plot_area_margin;
    static const int bottom_plot_area_margin;
    static const int minimal_plot_area_width; // arbitrary / base on visual experimentation
    static const int minimal_plot_area_height; // arbitrary / base on visual experimentation

    static const int indicator_width;
    static const int label_width;

    const int max_trace_points;
    bool width_sufficient = false;
    bool height_sufficient = false;
    cv::Rect2i hud_area;


    struct MetricsGroup {
        OpenCvTracePlotter trace_plotter;
        OpenCvValueIndicator rate_indicator;
        OpenCvLabel label;
        presage::physiology::MeasurementWithConfidence rate;
        bool display_rate = true;
        bool rate_is_high_confidence = false;
        const cv::Scalar confident_color;
        const cv::Scalar unconfident_color;
        absl::Status Render(cv::Mat& image);
    };

    std::unique_ptr<MetricsGroup> pulse_group;
    std::unique_ptr<MetricsGroup> upper_breathing_group;
    std::unique_ptr<MetricsGroup> lower_breathing_group;

};

} // namespace presage::smartspectra::gui
