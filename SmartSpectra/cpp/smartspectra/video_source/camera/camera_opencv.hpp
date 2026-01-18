#pragma once
// stdlib includes
#include <tuple>
#include <vector>
#include <map>
// third-party includes
#include <mediapipe/framework/port/opencv_video_inc.h>
// local includes
#include "camera.hpp"


namespace presage::camera::opencv {


// region ========================================= CODECS =============================================================
extern const std::map<CaptureCodec, int> kCvCodecFlagByCaptureCodec;
// endregion ===========================================================================================================
// region ========================================= MISCELLANEOUS CHECKS ===============================================
bool CheckCameraOpens(int camera_device_index, int cv_api_index = cv::CAP_ANY);

int DeterminePreferredBackendForCamera(int camera_device_index);

std::string DeterminePreferredBackendNameForCamera(int camera_device_index);

UncertainBool CheckCameraInterfaceSupportsTimestamp(int camera_device_index);

// endregion ===========================================================================================================
// region ========================================= RESOLUTION =========================================================

bool CheckCameraWorksWithResolution(cv::VideoCapture& capture, const cv::Size& resolution, int cv_capture_api = cv::CAP_ANY);

std::tuple<bool, cv::Size> GetMaximumCameraResolutionFromRange(
    int camera_device_index,
    CameraResolutionRange range_to_check = CameraResolutionRange::Mid,
    int cv_capture_api = cv::CAP_ANY
);

//@formatter:off
extern const std::vector<cv::Size> kCommonCameraResolutions;
extern const std::map<CameraResolutionRange, std::pair<int, int>> kCommonCameraResolutionRanges;
extern const std::vector<CameraResolutionRange> kCommonCameraResolutionRangeValues;
//@formatter:on

// endregion ===========================================================================================================

} // presage::camera::opencv
