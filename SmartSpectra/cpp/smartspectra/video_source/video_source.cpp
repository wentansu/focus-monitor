//
// Created by greg on 2/29/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/status/statusor.h>
// === local includes (if any) ===
#include "video_source.hpp"

namespace presage::smartspectra::video_source {


absl::Status VideoSource::TurnOnAutoExposure() {
    return absl::UnavailableError("TurnOnAutoExposure is not supported for this VideoSource.");
}

absl::Status VideoSource::TurnOffAutoExposure() {
    return absl::UnavailableError("TurnOffAutoExposure is not supported for this VideoSource.");
}

absl::Status VideoSource::ToggleAutoExposure() {
    return absl::UnavailableError("ToggleAutoExposure is not supported for this VideoSource.");
}

absl::StatusOr<bool> VideoSource::IsAutoExposureOn() {
    return absl::UnavailableError("IsAutoExposureOn is not supported for this VideoSource.");
}

absl::Status VideoSource::IncreaseExposure() {
    return absl::UnavailableError("IncreaseExposure is not supported for this VideoSource.");
}

absl::Status VideoSource::DecreaseExposure() {
    return absl::UnavailableError("DecreaseExposure is not supported for this VideoSource.");
}

bool VideoSource::SupportsExposureControls() {
    return false;
}

int VideoSource::GetWidth() {
    return -1;
}

int VideoSource::GetHeight() {
    return -1;
}

bool VideoSource::HasFrameDimensions() {
    return this->GetHeight() > -1 && this->GetWidth() > -1;
}

VideoSource& VideoSource::operator>>(cv::Mat& frame) {
    this->ProducePreTransformFrame(frame);
    frame = this->input_transformer.apply(frame);
    return *this;
}

absl::Status VideoSource::Initialize(const VideoSourceSettings& settings) {
    if (settings.input_transform_mode == InputTransformMode::Unspecified_EnumEnd) {
        this->input_transformer.mode = this->GetDefaultInputTransformMode();
    } else {
        this->input_transformer.mode = settings.input_transform_mode;
    }
    return absl::OkStatus();
}

InputTransformMode VideoSource::GetDefaultInputTransformMode() {
    return InputTransformMode::None;
}

} // namespace presage::smartspectra::video_source
