//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opencv_element_fits.hpp
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
#include <opencv2/core.hpp>
#include <absl/status/status.h>
// === local includes (if any) ===

#pragma once
namespace presage::smartspectra::gui {

absl::Status CheckThatElementFitsImage(
    const std::string& element_name,
    const cv::Rect2i element_area,
    const cv::Mat& image
);

} // namespace presage::smartspectra::gui
