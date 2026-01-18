//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// confidence_thresholding.hpp
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
// === local includes (if any) ===
#pragma once

namespace presage::smartspectra::gui {

// TODO: establish SST between Core & Edge for thresholding constants
constexpr float pulse_log_snr_threshold = 2.35; // Defined Sept 2024, taken from line 1062 of compute_metrics.py
constexpr float breathing_log_snr_threshold = 1.7; // Defined Oct 2024 (after changing PW =2), taken from line 1115 of compute_metrics.py
constexpr float min_supported_breathing_rate = 8.0;
constexpr float max_supported_breathing_rate = 31.0;

bool is_pulse_high_confidence(float snr);
bool is_breathing_high_confidence(float snr);
bool is_breathing_rate_high_confidence(float snr, float rate);



} // namespace presage::smartspectra::gui
