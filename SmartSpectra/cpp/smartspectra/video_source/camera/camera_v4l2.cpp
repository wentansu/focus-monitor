//
// Created by greg on 1/11/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
#include <cstdio>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
// === third-party includes (if any) ===
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mediapipe/framework/deps/status_macros.h>
#include <physiology/modules/filesystem_absl.h>
// === local includes (if any) ===
#include "camera_v4l2.hpp"

namespace presage::camera::v4l2 {

absl::StatusOr<std::string> GetCameraName(int device_index) {
    std::string device_path = "/dev/video" + std::to_string(device_index);

    MP_ASSIGN_OR_RETURN(int file_descriptor, presage::filesystem::abseil::SafeOpen(device_path.c_str(), O_RDWR));
    if (file_descriptor == -1) {
        return absl::NotFoundError("Failed to open video device at " + device_path);
    }

    struct v4l2_capability video_capture;
    if (ioctl(file_descriptor, VIDIOC_QUERYCAP, &video_capture) == -1) {
        close(file_descriptor);
        return absl::UnavailableError("Failed to query video device capabilities.");
    }

    close(file_descriptor);
    return std::string(reinterpret_cast<const char*>(video_capture.card));
}

absl::StatusOr<std::vector<AutoExposureSetting>> GetAutoExposureSettings(int device_index) {
    std::vector<AutoExposureSetting> settings;

    // Open the device
    std::string device_path = "/dev/video" + std::to_string(device_index);
    MP_ASSIGN_OR_RETURN(int file_descriptor, presage::filesystem::abseil::SafeOpen(device_path.c_str(), O_RDWR));
    if (file_descriptor == -1) {
        return absl::UnavailableError("Failed to open video device at " + device_path);
    }

    // Query the control
    struct v4l2_queryctrl control = {};
    control.id = V4L2_CID_EXPOSURE_AUTO;

    if (ioctl(file_descriptor, VIDIOC_QUERYCTRL, &control) == 0) {
        std::string control_name = reinterpret_cast<const char*>(control.name);
        if (control.type == V4L2_CTRL_TYPE_MENU) {
            struct v4l2_querymenu menu = {};
            menu.id = control.id;
            for (menu.index = control.minimum;
                 menu.index <= control.maximum;
                 menu.index++) {
                if (ioctl(file_descriptor, VIDIOC_QUERYMENU, &menu) == 0) {
                    AutoExposureSetting setting = {
                        static_cast<int>(menu.index),
                        reinterpret_cast<char*>(menu.name)
                    };
                    settings.push_back(setting);
                }
            }
        }
    } else {
        return absl::NotFoundError("Query for automatic exposure setting control failed.");
    }

    // Close the device
    close(file_descriptor);
    return settings;
}


absl::StatusOr<AutoExposureConfiguration> InferAutoExposureConfigurationFromSettings(
    const std::vector<AutoExposureSetting>& settings
) {
    AutoExposureConfiguration config = {-1, -1};
    std::regex auto_regex("(\\b[Aa]uto|[Aa]perture)", std::regex_constants::icase);
    std::regex manual_regex("(\\b[Mm]anual|[Ss]hutter\\b)", std::regex_constants::icase);
    bool on_setting_found = false;
    bool off_setting_found = false;

    for (const auto& setting : settings) {
        if (std::regex_search(setting.description, auto_regex)) {
            config.auto_exposure_on_value = setting.value;
            on_setting_found = true;
        } else if (std::regex_search(setting.description, manual_regex)) {
            config.auto_exposure_off_value = setting.value;
            off_setting_found = true;
        }
    }

    if(on_setting_found && off_setting_found) {
        return config;
    } else {
        return absl::NotFoundError("Failed to infer auto exposure configuration.");
    }

}

absl::StatusOr<std::vector<Resolution>> GetSupportedResolutions(int camera_device_index, const std::string& codec) {
    std::vector<Resolution> resolutions;
    int file_descriptor;
    struct v4l2_format format{};
    struct v4l2_frmsizeenum frame_size{};

    // Open the device
    std::string device_path = "/dev/video" + std::to_string(camera_device_index);
    file_descriptor = open(device_path.c_str(), O_RDWR);
    if (file_descriptor < 0) {
        return absl::NotFoundError("Failed to open camera device.");
    }

    // Set the codec
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = v4l2_fourcc(codec[0], codec[1], codec[2], codec[3]);
    if (ioctl(file_descriptor, VIDIOC_S_FMT, &format) < 0) {
        close(file_descriptor);
        return absl::InternalError("Failed to set codec for camera device.");
    }

    // Query the supported resolutions
    frame_size.pixel_format = format.fmt.pix.pixelformat;
    frame_size.index = 0;
    while (ioctl(file_descriptor, VIDIOC_ENUM_FRAMESIZES, &frame_size) >= 0) {
        if (frame_size.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            resolutions.push_back(
                Resolution{static_cast<int>(frame_size.discrete.width),
                           static_cast<int>(frame_size.discrete.height)}
            );
        }
        frame_size.index++;
    }

    // Check if resolutions were found
    if (resolutions.empty()) {
        close(file_descriptor);
        return absl::NotFoundError("No resolutions found for the specified codec.");
    }

    // Close the device
    close(file_descriptor);

    return resolutions;
}

std::string ToString(const AutoExposureSetting& setting) {
    return std::to_string(setting.value) + ": " + setting.description;
}

} // namespace presage::camera::v4l2
