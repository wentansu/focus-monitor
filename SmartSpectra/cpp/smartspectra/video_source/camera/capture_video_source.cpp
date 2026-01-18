//
// Created by greg on 2/29/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
#include <exception>
#include <fstream>
// === third-party includes (if any) ===
#include <mediapipe/framework/port/ret_check.h>
#include <mediapipe/framework/port/logging.h>
// === local includes (if any) ===
// @formatter:off
#ifdef __linux__
#include <smartspectra/video_source/camera/camera_v4l2.hpp>
namespace pcam_v4l2 = presage::camera::v4l2;
#endif
// @formatter:on
#include "camera_opencv.hpp"
#include "capture_video_source.hpp"


namespace presage::smartspectra::video_source::capture {

namespace pcam = presage::camera;
namespace pcam_cv = presage::camera::opencv;

bool CaptureVideoFileSource::SupportsExactFrameTimestamp() const {
    return true;
}

int64_t CaptureVideoFileSource::GetFrameTimestamp() const {
    return static_cast<int64_t>(this->capture.get(cv::CAP_PROP_POS_MSEC) * 1000.0); // milliseconds -> microseconds;
}

absl::Status CaptureVideoFileSource::Initialize(const presage::smartspectra::video_source::VideoSourceSettings& settings) {
    MP_RETURN_IF_ERROR(VideoSource::Initialize(settings));
    capture.open(settings.input_video_path);
    RET_CHECK(capture.isOpened());
    return absl::OkStatus();
}

int CaptureVideoFileSource::GetWidth() {
    return static_cast<int>(this->capture.get(cv::CAP_PROP_FRAME_WIDTH));
}

int CaptureVideoFileSource::GetHeight() {
    return static_cast<int>(this->capture.get(cv::CAP_PROP_FRAME_HEIGHT));
}

void CaptureVideoFileSource::ProducePreTransformFrame(cv::Mat& frame) {
    this->capture >> frame;
}

std::vector<int64_t> CaptureVideoAndTimeStampFile::ReadTimestampsFromFile(const std::string& filename) {
    std::vector<int64_t> timestamps;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        try {
            int64_t timestamp_ms = std::stoll(line);  // Convert string to int64_t
            int64_t timestamp_us = timestamp_ms * 1000; // want to be in microseconds
            timestamps.push_back(timestamp_us);
        } catch (const std::exception& e) {
            std::cerr << "Error converting line to timestamp: " << e.what() << std::endl;
        }
    }
    return timestamps;
}

absl::Status CaptureVideoAndTimeStampFile::Initialize(const VideoSourceSettings& settings) {
    timestamps = ReadTimestampsFromFile(settings.input_video_time_path);
    return CaptureVideoFileSource::Initialize(settings);
}

int64_t CaptureVideoAndTimeStampFile::GetFrameTimestamp() const {
    return timestamps[std::max(0, static_cast <int> (this->capture.get(cv::CAP_PROP_POS_FRAMES)) - 1)];
}

bool CaptureVideoAndTimeStampFile::SupportsExactFrameTimestamp() const {
    return true;
}

absl::Status CaptureCameraSource::Initialize(const presage::smartspectra::video_source::VideoSourceSettings& settings) {
    MP_RETURN_IF_ERROR(VideoSource::Initialize(settings));
    if (settings.input_transform_mode == InputTransformMode::MirrorHorizontal) {
        // This is a bit counter-intuitive. OpenCV's Capture doesn't work by default in mirror/face mode,
        // so the expected behavior for "no horizontal mirroring" is actually to flip horizontally and vise versa.
        this->flip_horizontal = false;
    }
    if (settings.input_transform_mode != InputTransformMode::None) {
        LOG(INFO) << "Input transform mode: " << AbslUnparseFlag(settings.input_transform_mode);
    }
#ifdef __linux__
    MP_ASSIGN_OR_RETURN(std::string camera_name, pcam_v4l2::GetCameraName(settings.device_index));
    LOG(INFO) << "Camera name: " << camera_name;
    MP_ASSIGN_OR_RETURN(
        std::vector<pcam_v4l2::AutoExposureSetting> auto_exposure_settings,
        pcam_v4l2::GetAutoExposureSettings(settings.device_index)
    );
    LOG(INFO) << "Auto exposure settings detected by the camera: ";
    for (const auto& setting: auto_exposure_settings) {
        LOG(INFO) << "   " << pcam_v4l2::ToString(setting);
    }
    MP_ASSIGN_OR_RETURN(
        auto_exposure_configuration,
        pcam_v4l2::InferAutoExposureConfigurationFromSettings(auto_exposure_settings)
    );
#else
    // Assume C920 values by default...
    auto_exposure_configuration = {
        pcam::C920E_AUTO_EXPOSURE_ON_SETTING,
        pcam::C920E_AUTO_EXPOSURE_OFF_SETTING
    };
#endif
    int backend_to_use = pcam_cv::DeterminePreferredBackendForCamera(settings.device_index);
    std::string camera_backend_name = pcam_cv::DeterminePreferredBackendNameForCamera(
        settings.device_index
    );
    if (backend_to_use == cv::VideoCaptureAPIs::CAP_V4L2) {
        this->UseUptimeTimestampConversion();
    }
    LOG(INFO) << "Camera backend to use: " << camera_backend_name;
    // region ================================== CHECK PER-FRAME TIMESTAMP SUPPORT =================================
    LOG(INFO) << "Check if frame timestamps are supported by the camera capture interface...";

    pcam::UncertainBool timestamp_supported = pcam_cv::CheckCameraInterfaceSupportsTimestamp(
        settings.device_index
    );
    switch (timestamp_supported) {
        case pcam::UncertainBool::False:
            LOG(INFO) << "Frame timestamp are not supported by the camera capture interface. Using wall time instead.";
            capture_supports_timestamp = false;
            break;
        case pcam::UncertainBool::True:
            capture_supports_timestamp = true;

            LOG(INFO) << "Frame timestamps are supported by the camera capture interface.";

            break;
        case pcam::UncertainBool::Unknown:

            LOG(INFO)
                << "Please check if timestamp is supported by the camera capture interface. Treating as not known and using wall time instead. "
                << camera_backend_name;

            capture_supports_timestamp = false;
            break;
    }
    // endregion ===================================================================================================

    // region =================================== SET CAMERA RESOLUTION ============================================
    LOG(INFO) << "Select and set camera resolution.";
    ResolutionSelectionMode effective_resolution_selection_mode = settings.resolution_selection_mode;
    int effective_capture_width_px = settings.capture_width_px;
    int effective_capture_height_px = settings.capture_height_px;

    if (settings.resolution_selection_mode == video_source::ResolutionSelectionMode::Auto) {
        if (settings.resolution_range == pcam::CameraResolutionRange::Unspecified_EnumEnd) {
            if (settings.capture_width_px == -1 && settings.capture_height_px == -1) {
                LOG(INFO) << "No camera resolution range specified, while exact resolution is not specified. "
                             "Will attempt to use the default exact resolution, 1280x720...";
                effective_resolution_selection_mode = video_source::ResolutionSelectionMode::Exact;
                effective_capture_width_px = 1280;
                effective_capture_height_px = 720;
            }
        }
    }
    cv::Size camera_resolution;
    if (effective_resolution_selection_mode == video_source::ResolutionSelectionMode::Range) {
        if (settings.resolution_range == pcam::CameraResolutionRange::Unspecified_EnumEnd) {
            return absl::FailedPreconditionError(
                "No camera resolution range specified with `range` resolution selection mode. Exiting."
            );
        }
        LOG(INFO) << "Try out different camera resolutions...";
        bool suitable_resolution_found = false;
        // we check first the mid-range, then the low-range
        // we avoid higher resolution ranges because those could result in low FPS due to USB bandwidth
        std::tie(suitable_resolution_found, camera_resolution) =
            pcam_cv::GetMaximumCameraResolutionFromRange(
                settings.device_index,
                settings.resolution_range,
                backend_to_use
            );
        if (!suitable_resolution_found) {
            return absl::FailedPreconditionError("Failed to find a suitable camera resolution.");
        }
    } else if (effective_resolution_selection_mode == video_source::ResolutionSelectionMode::Exact) {
        if (effective_capture_width_px < 0 || effective_capture_height_px < 0) {
            return absl::FailedPreconditionError(
                "Both `capture_width_px` and `capture_height_px` must be set to positive, nonzero values when using "
                "the `exact` resolution selection mode. Got: " + std::to_string(effective_capture_width_px)
                + " x " + std::to_string(effective_capture_height_px) + ". Exiting."
            );
        }
        camera_resolution = {effective_capture_width_px, effective_capture_height_px};
    }

    capture.open(settings.device_index, backend_to_use);

    capture.set(cv::CAP_PROP_FRAME_WIDTH, camera_resolution.width);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, camera_resolution.height);


    LOG(INFO) << "Camera set to resolution: "
              << capture.get(cv::CAP_PROP_FRAME_WIDTH) << " x "
              << capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    // endregion ===================================================================================================

    capture.set(cv::CAP_PROP_FOURCC, pcam_cv::kCvCodecFlagByCaptureCodec.at(settings.codec));

    const int fps = 30;
    capture.set(cv::CAP_PROP_FPS, fps);

    RET_CHECK(capture.isOpened());
    return absl::OkStatus();
}

bool CaptureCameraSource::SupportsExactFrameTimestamp() const {
    return this->capture_supports_timestamp;
}

int64_t CaptureCameraSource::GetFrameTimestamp() const {
    return this->capture_supports_timestamp ?
           static_cast<int64_t>(this->convert_timestamp_ms(this->capture.get(cv::CAP_PROP_POS_MSEC)) * 1000.0) // milliseconds -> microseconds
                                            :
           static_cast<int64_t>(
               std::chrono::duration_cast<std::chrono::microseconds>(
                   // can't use chrono::high_resolution_clock::now()
                   // because time_since_epoch won't yield absolute time on macOS and/or Clang
                std::chrono::system_clock::now().time_since_epoch()
               ).count() - microsecond_epoch_at_start
           );
}

absl::Status CaptureCameraSource::TurnOnAutoExposure() {
    MP_ASSIGN_OR_RETURN(bool auto_exposure_on, this->IsAutoExposureOn());
    if (!auto_exposure_on) {
        if (!capture.set(cv::CAP_PROP_AUTO_EXPOSURE,
                         static_cast<double>(this->auto_exposure_configuration.auto_exposure_on_value))) {
            return absl::UnavailableError(
                "Failed to turn on auto exposure. The capture interface does not support auto exposure mode setting."
            );
        }
    }
    return absl::OkStatus();
}

absl::Status CaptureCameraSource::TurnOffAutoExposure() {
    MP_ASSIGN_OR_RETURN(bool auto_exposure_on, this->IsAutoExposureOn());
    if (auto_exposure_on) {
        if (!capture.set(cv::CAP_PROP_AUTO_EXPOSURE,
                         static_cast<double>(this->auto_exposure_configuration.auto_exposure_off_value))) {
            return absl::UnavailableError(
                "Failed to turn off auto exposure. The capture interface does not support auto exposure mode setting."
            );
        } else {
            MP_ASSIGN_OR_RETURN(double current_exposure, this->GetExposure());
            LOG(INFO) << "Locked exposure at: " << current_exposure;
        }
    }
    return absl::OkStatus();
}

absl::Status CaptureCameraSource::ToggleAutoExposure() {
    MP_ASSIGN_OR_RETURN(bool auto_exposure_on, this->IsAutoExposureOn());
    int desired_value;
    std::string desired_state;
    if (auto_exposure_on) {
        desired_value = this->auto_exposure_configuration.auto_exposure_off_value;
        desired_state = "off";
    } else {
        desired_value = this->auto_exposure_configuration.auto_exposure_on_value;
        desired_state = "on";
    }
    if (!capture.set(cv::CAP_PROP_AUTO_EXPOSURE, static_cast<double>(desired_value))) {
        return absl::UnavailableError(
            "Failed to turn off auto exposure. The capture interface does not support auto exposure mode setting."
        );
    } else {
        LOG(INFO) << "Auto exposure: " << desired_state;
    }
    return absl::OkStatus();
}

absl::StatusOr<bool> CaptureCameraSource::IsAutoExposureOn() {
    int auto_exposure_mode = static_cast<int>(capture.get(cv::CAP_PROP_AUTO_EXPOSURE));
    if (!auto_exposure_mode) {
        return absl::UnavailableError(
            "Failed to retrieve auto exposure mode. The capture interface does not support auto exposure mode retrieval."
        );
    }
    return auto_exposure_mode == this->auto_exposure_configuration.auto_exposure_on_value;
}

absl::Status CaptureCameraSource::IncreaseExposure() {
    return this->ModifyExposure(this->exposure_step);
}

absl::Status CaptureCameraSource::DecreaseExposure() {
    return this->ModifyExposure(-this->exposure_step);
}

absl::StatusOr<double> CaptureCameraSource::GetExposure() {
    double manual_exposure = capture.get(cv::CAP_PROP_EXPOSURE);
    if (manual_exposure == 0.0) {
        return absl::UnavailableError(
            "Failed to get exposure. The capture interface does not support exposure retrieval."
        );
    }
    return manual_exposure;
}

absl::Status CaptureCameraSource::ModifyExposure(int by) {
    if (by == 0) return absl::OkStatus();
    std::string action, which_limit;
    if (by > 0) {
        action = "raise";
        which_limit = "upper";
    } else {
        action = "lower";
        which_limit = "lower";
    }
    MP_ASSIGN_OR_RETURN(bool auto_exposure_on, this->IsAutoExposureOn());
    if (!auto_exposure_on) {
        MP_ASSIGN_OR_RETURN(double manual_exposure, this->GetExposure());
        manual_exposure += by;
        if (capture.set(cv::CAP_PROP_EXPOSURE, manual_exposure)) {
            LOG(INFO) << action << " exposure to: " << manual_exposure;
        } else {
            LOG(WARNING) << "Unable to " << action << " exposure to " << manual_exposure
                         << " , not exposure setting not supported or at " << which_limit << " limit.";
            return absl::OkStatus();
        };
    } else {
        LOG(WARNING) << "Unable to change exposure, not in manual exposure mode.";

    }
    return absl::OkStatus();
}

bool CaptureCameraSource::SupportsExposureControls() {
    int auto_exposure_mode = static_cast<int>(capture.get(cv::CAP_PROP_AUTO_EXPOSURE));
    return static_cast<bool>(auto_exposure_mode);
}

int CaptureCameraSource::GetWidth() {
    return static_cast<int>(this->capture.get(cv::CAP_PROP_FRAME_WIDTH));
}

int CaptureCameraSource::GetHeight() {
    return static_cast<int>(this->capture.get(cv::CAP_PROP_FRAME_HEIGHT));
}

void CaptureCameraSource::UseNoTimestampConversion() {
    this->convert_timestamp_ms = [](int64_t input_timestamp_ms) { return input_timestamp_ms; };
}

static int64_t monotonic_to_epoch_offset_ms = -1;
static int64_t V4l2ConvertCaptureTimeToEpoch(int64_t v4l_ts_ms){

    if(monotonic_to_epoch_offset_ms == -1){
        struct timeval epoch_time;  gettimeofday(&epoch_time, NULL);
        struct timespec  vs_time;  clock_gettime(CLOCK_MONOTONIC, &vs_time);

        long uptime_ms = vs_time.tv_sec * 1000 + (long)  round(vs_time.tv_nsec / 1000000.0);
        long epoch_ms = epoch_time.tv_sec * 1000 + (long) round(epoch_time.tv_usec / 1000.0);

        // add this quantity to the CV_CAP_PROP_POS_MEC to get unix time stamped frames
        monotonic_to_epoch_offset_ms = epoch_ms - uptime_ms;

    }

    return monotonic_to_epoch_offset_ms + v4l_ts_ms;
}

void CaptureCameraSource::UseUptimeTimestampConversion() {
    this->convert_timestamp_ms = V4l2ConvertCaptureTimeToEpoch;
}

void CaptureCameraSource::ProducePreTransformFrame(cv::Mat& frame) {
    this->capture >> frame;
}

InputTransformMode CaptureCameraSource::GetDefaultInputTransformMode() {
    return InputTransformMode::MirrorHorizontal;
}


} // namespace presage::smartspectra::video_source::capture
