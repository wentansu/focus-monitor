//
// Created by greg on 2/22/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/status/status.h>
#include <physiology/graph/stream_and_packet_names.h>
// === local includes (if any) ===
#include "operation_context.hpp"
#include "packet_helpers.hpp"

namespace presage::smartspectra::container {
namespace ph = packet_helpers;
namespace pe = physiology::edge;

template<settings::OperationMode TOperationMode>
absl::Status OperationContext<TOperationMode>::InitializePollers(mediapipe::CalculatorGraph& graph) {
    return absl::OkStatus();
}

template<settings::OperationMode TOperationMode>
absl::Status OperationContext<TOperationMode>::QueryPollers(bool& operation_state_changed, bool verbose) {
    return absl::OkStatus();
}

template
class OperationContext<settings::OperationMode::Continuous>;

// region ==================================== SPOT ====================================================================
OperationContext<settings::OperationMode::Spot>::OperationContext(
    const settings::OperationSettings<settings::OperationMode::Spot>& operation_settings
) : spot_duration_s(operation_settings.spot_duration_s), time_left_s(operation_settings.spot_duration_s) {}

void OperationContext<settings::OperationMode::Spot>::Reset() {
    time_left_s = spot_duration_s;
}

absl::Status OperationContext<settings::OperationMode::Spot>::InitializePollers(mediapipe::CalculatorGraph& graph) {
    return this->time_left_poller.Initialize(graph, pe::graph::output_streams::spot::kTimeLeft);
}


absl::Status OperationContext<settings::OperationMode::Spot>::QueryPollers(
    bool& operation_state_changed,
    bool verbose
) {
    double previous_time_left = this->time_left_s;
    MP_RETURN_IF_ERROR(ph::GetPacketContentsIfAny(
        this->time_left_s,
        operation_state_changed,
        time_left_poller.Get(),
        pe::graph::output_streams::spot::kTimeLeft,
        [this, &verbose, &previous_time_left]() {
            return (this->time_left_s < this->spot_duration_s && this->time_left_s != previous_time_left) || verbose;
        }
    ));
    return absl::OkStatus();
}
// endregion ===========================================================================================================

} // namespace presage::smartspectra::container
