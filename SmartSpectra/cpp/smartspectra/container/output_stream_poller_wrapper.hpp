//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// output_stream_poller_wrapper.h
// Created by Greg on 2/22/2024.
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
#include <optional>
// === third-party includes (if any) ===
#include <mediapipe/framework/output_stream_poller.h>
#include <mediapipe/framework/calculator_graph.h>
#include <absl/status/status.h>
// === local includes (if any) ===

namespace presage::smartspectra::container::output_stream_poller_wrapper {

/** Lightweight RAII wrapper around MediaPipe's OutputStreamPoller. */
class OutputStreamPollerWrapper {
public:
    OutputStreamPollerWrapper() = default;
    ~OutputStreamPollerWrapper() = default;
    
    // Non-copyable - MediaPipe objects typically aren't copyable
    OutputStreamPollerWrapper(const OutputStreamPollerWrapper&) = delete;
    OutputStreamPollerWrapper& operator=(const OutputStreamPollerWrapper&) = delete;
    
    // Movable
    OutputStreamPollerWrapper(OutputStreamPollerWrapper&&) = default;
    OutputStreamPollerWrapper& operator=(OutputStreamPollerWrapper&&) = default;
    /** Attach to a stream from the given graph. */
    absl::Status Initialize(mediapipe::CalculatorGraph& graph, const std::string& stream_name);
    /** Access the underlying poller. */
    mediapipe::OutputStreamPoller& Get();
private:
    std::optional<mediapipe::OutputStreamPoller> stream_poller_;
};

} // namespace presage::smartspectra::container::output_stream_poller_wrapper
