//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// keyboard_input.h
// Created by Greg on 2/15/24.
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

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <mediapipe/framework/port/opencv_video_inc.h>
#include <absl/status/status.h>
#include <physiology/modules/messages/status.pb.h>
// === local includes (if any) ===
#include "settings.hpp"
#include <smartspectra/video_source/video_source.hpp>

namespace presage::smartspectra::container::keyboard_input {

/**
 * @brief Handle interactive keyboard commands for example applications.
 */
absl::Status HandleKeyboardInput(
    bool& grab_frames,
    bool& recording,
    video_source::VideoSource& v_source,
    const settings::GeneralSettings& settings,
    physiology::StatusValue status
);


} // namespace presage::smartspectra::container::keyboard_input
