//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_value_indicator.hpp
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
#include <absl/status/status.h>
// === local includes (if any) ===


namespace presage::smartspectra::gui {

class OpenCvValueIndicator {
public:
    OpenCvValueIndicator(int x, int y, int width, int height, int precision_digits = 1);
    ~OpenCvValueIndicator() = default;
    absl::Status Render(cv::Mat& image, float value, cv::Scalar color);
    absl::Status RenderNA(cv::Mat& image, cv::Scalar color);
    static const float min_value;
    static const float max_value;
private:
    cv::Rect2i indicator_area;
    double font_scale;
    cv::Point2i text_origin;
    const int font_face = cv::FONT_HERSHEY_DUPLEX;
    int precision_digits;

};

} // namespace presage::smartspectra::gui
