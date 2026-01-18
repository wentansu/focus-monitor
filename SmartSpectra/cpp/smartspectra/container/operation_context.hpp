//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// operation_context.h
// Created by Greg on 2/20/2024.
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
#include <mediapipe/framework/calculator_graph.h>
// === local includes (if any) ===
#include "settings.hpp"
#include "output_stream_poller_wrapper.hpp"

namespace presage::smartspectra::container {
namespace poller = output_stream_poller_wrapper;

template<settings::OperationMode TOperationMode>
/**
 * @brief Helper for managing polling of operation-specific graph streams.
 */
class OperationContext {
public:
    explicit OperationContext(const settings::OperationSettings<settings::OperationMode::Continuous>& settings) {};
    void Reset(){};
    /** Initialize pollers required for the given operation mode. */
    absl::Status InitializePollers(mediapipe::CalculatorGraph& graph);
    /** Poll graph output streams and update internal state. */
    absl::Status QueryPollers(bool& operation_state_changed, bool verbose);
};

template<>
class OperationContext<settings::OperationMode::Spot> {
public:
    /** Create a context for spot mode operation. */
    explicit OperationContext(const settings::OperationSettings<settings::OperationMode::Spot>& operation_settings);
    /** Reset internal state to the beginning of a spot run. */
    void Reset();
    /** Initialize pollers required for spot mode. */
    absl::Status InitializePollers(mediapipe::CalculatorGraph& graph);
    /** Poll spot specific streams and update state. */
    absl::Status QueryPollers(bool& operation_state_changed, bool verbose);
private:
    double time_left_s;
    const double spot_duration_s;
    poller::OutputStreamPollerWrapper time_left_poller;
};

} // namespace presage::smartspectra::container
