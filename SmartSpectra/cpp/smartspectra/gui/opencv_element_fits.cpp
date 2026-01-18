//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_element_fits.cpp
// Created by greg on 1/6/25.
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
#include <physiology/modules/geometry.hpp>
// === local includes (if any) ===
#include "opencv_element_fits.hpp"
namespace presage::smartspectra::gui {

absl::Status CheckThatElementFitsImage(
    const std::string& element_name,
    const cv::Rect2i element_area,
    const cv::Mat& image
) {
    cv::Rect2i image_bounds = cv::Rect2i(0, 0, image.cols, image.rows);
    if (!presage::geometry::opencv::Rectangle1FullyCovers2(image_bounds, element_area)) {
        return absl::InvalidArgumentError(
            "Element " + element_name + " with bounds of x = " +
            std::to_string(element_area.x) + ", y = " +
            std::to_string(element_area.y) + ", width = " +
            std::to_string(element_area.width) + ", height = " +
            std::to_string(element_area.height) +
            " does not fit into image with size of " + std::to_string(image.cols) + " x " + std::to_string(image.rows) +
            ".");
    }
    return absl::OkStatus();
}

}  // namespace presage::smartspectra::gui
