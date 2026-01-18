//
// Created by greg on 2/22/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
// === local includes (if any) ===
#include "output_stream_poller_wrapper.hpp"

namespace presage::smartspectra::container::output_stream_poller_wrapper {

absl::Status OutputStreamPollerWrapper::Initialize(mediapipe::CalculatorGraph& graph, const std::string& stream_name) {
    MP_ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller stream_poller, graph.AddOutputStreamPoller(stream_name));
    // Construct in-place via move; ensures proper lifetime and destructor handling
    this->stream_poller_.emplace(std::move(stream_poller));
    return absl::OkStatus();
}

mediapipe::OutputStreamPoller& OutputStreamPollerWrapper::Get() {
    return this->stream_poller_.value();
}

} // namespace presage::smartspectra::container::output_stream_poller_wrapper
