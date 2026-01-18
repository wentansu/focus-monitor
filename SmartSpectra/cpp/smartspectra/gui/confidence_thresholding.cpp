//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// confidence_thresholding.cpp
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
#include <cmath>
// === third-party includes (if any) ===
// === local includes (if any) ===
#include "confidence_thresholding.hpp"

namespace presage::smartspectra::gui {

#define EPSILON 1e-15


bool is_pulse_high_confidence(float snr) {
    if (snr < EPSILON) {
        snr = EPSILON;
    }
    return std::log(snr) >= pulse_log_snr_threshold;
}

bool is_breathing_high_confidence(float snr) {
    if (snr < EPSILON) {
        snr = EPSILON;
    }
    return std::log(snr) >= breathing_log_snr_threshold;
}

bool is_breathing_rate_high_confidence(float snr, float rate) {
    if (snr < EPSILON) {
        snr = EPSILON;
    }
    return std::log(snr) >= breathing_log_snr_threshold &&
           rate >= min_supported_breathing_rate &&
           rate <= max_supported_breathing_rate;
}



} // namespace presage::smartspectra::gui
