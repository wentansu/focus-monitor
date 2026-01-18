//
// Created by greg on 1/16/24.
// Copyright (c) 2024 Presage Technologies
//

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/flags/flag.h>
// === local includes (if any) ===

//TODO: rethink namespace here, for presage::camera::camera_cv, and for presage::camera::camera_v4l2
namespace presage::camera {

// region ======================================== RESOLUTION ==========================================================

struct Resolution {
    int width;
    int height;
};

// For now, when updating this, be sure to update kCommonCameraResolutionRanges as well (and kCommonCameraResolutions, if need be).
enum CameraResolutionRange {
    Low,
    Mid,
    High,
    Ultra,
    FourK,
    Giant,
    Complete,
    Unspecified_EnumEnd  // has to be kept last in enum
};

//@formatter:off
bool AbslParseFlag(absl::string_view text, CameraResolutionRange* range, std::string* error);
std::string AbslUnparseFlag(CameraResolutionRange range);
extern const std::vector<std::string> kCommonCameraResolutionRangeNames;
extern const std::string kCommonCameraResolutionRangeNameList;
//@formatter:on

// endregion ===========================================================================================================

// region ========================================= CODECS =============================================================
//TODO: all enum-based collections should be using reflective enums.
// One candidate: https://github.com/BlackMATov/enum.hpp
// Another: https://github.com/aantron/better-enums

//TODO: move all functions using ABSL (and constants using thsoe functions for initialization) to a separate library,
// with the same namespace. We need to maintain loose coupling, i.e. in case we want to use the presage::camera library
// where Abseil is not available.
enum class UncertainBool : int {
    True = 1,
    False = 0,
    Unknown = -1
};

// note: needs to be updated only together with kCaptureCodecValues until we use reflective enums
enum CaptureCodec {
    MJPG,
    UYVY
};


//@formatter:off
std::string AbslUnparseFlag(CaptureCodec codec);
bool AbslParseFlag(absl::string_view text, CaptureCodec* codec, std::string* error);
extern const std::vector<CaptureCodec> kCaptureCodecValues;
extern const std::vector<std::string> kCaptureCodecNames;
extern const std::string kCaptureCodecNameList;
//@formatter:on
// endregion ===========================================================================================================

// region ========================================= EXPOSURE ===========================================================

struct AutoExposureConfiguration {
    int auto_exposure_on_value;
    int auto_exposure_off_value;
};

constexpr int C920E_AUTO_EXPOSURE_ON_SETTING = 3;
constexpr int C920E_AUTO_EXPOSURE_OFF_SETTING = 1;
constexpr int CU30_AUTO_EXPOSURE_ON_SETTING = 0;
constexpr int CU30_AUTO_EXPOSURE_OFF_SETTING = 1;
constexpr int CU27_AUTO_EXPOSURE_ON_SETTING = 0;
constexpr int CU27_AUTO_EXPOSURE_OFF_SETTING = 2;

// endregion ===========================================================================================================

} // namespace presage::camera
