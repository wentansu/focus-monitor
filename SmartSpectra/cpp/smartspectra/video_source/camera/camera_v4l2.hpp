//
// Created by greg on 1/11/24.
// Copyright (c) 2024 Presage Technologies
//

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/status/statusor.h>
// === local includes (if any) ===
#include "camera.hpp"


namespace presage::camera::v4l2 {

struct AutoExposureSetting {
    int value;
    std::string description;
};

std::string ToString(const AutoExposureSetting& setting);

absl::StatusOr<std::string> GetCameraName(int device_index);

absl::StatusOr<std::vector<AutoExposureSetting>> GetAutoExposureSettings(int device_index);

absl::StatusOr<std::vector<Resolution>> GetSupportedResolutions(int camera_device_index, const std::string& codec);

absl::StatusOr<AutoExposureConfiguration> InferAutoExposureConfigurationFromSettings(
    const std::vector<AutoExposureSetting>& settings
);

} // namespace presage::camera::v4l2
