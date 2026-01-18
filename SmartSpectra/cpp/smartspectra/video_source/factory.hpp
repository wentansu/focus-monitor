//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// factory.h
// Created by Greg on 3/1/24.
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
#include <memory>
// === third-party includes (if any) ===
#include <absl/status/statusor.h>
// === local includes (if any) ===
#include "video_source.hpp"
#include "settings.hpp"

namespace presage::smartspectra::video_source {

/**
 * @brief Factory helper for constructing the appropriate VideoSource
 *        implementation based on the provided settings.
 * \ingroup video_source
 */
absl::StatusOr<std::unique_ptr<VideoSource>> BuildVideoSource(const VideoSourceSettings& settings);

} // namespace presage::smartspectra::video_source
