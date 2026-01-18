//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_plotter.hpp
// Created by Greg on 12/13/24.
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
#include <thread>
// === third-party includes (if any) ===
#include <absl/status/status.h>
#include <opencv2/core.hpp>
#include <physiology/modules/messages/metrics.pb.h>
// === local includes (if any) ===

#pragma once

namespace presage::smartspectra::gui {

class OpenCvTracePlotter {
public:
    OpenCvTracePlotter(int x, int y, int width, int height, int max_points = 300);

    ~OpenCvTracePlotter() = default;

    void UpdateTraceWithSampleRange(const google::protobuf::RepeatedPtrField<physiology::Measurement>& new_values);
    void UpdateTraceWithSample(const physiology::Measurement& new_value);

    absl::Status Render(
        cv::Mat& image,
        const cv::Scalar& color = cv::Scalar(0, 255, 0)
    );

private:
    cv::Rect2i plot_area;
    google::protobuf::RepeatedPtrField<physiology::Measurement> buffer;

    const int max_points;

    int last_overlap_area_start = 0;
};

} // presage::smartspectra::gui
