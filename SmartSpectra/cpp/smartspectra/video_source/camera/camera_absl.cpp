/*
 * Created by Gregory Kramida on 9/13/23.
 * Copyright (c) 2023 Presage Technologies. All rights reserved.
 */
// third-party includes
#include <absl/strings/str_join.h>
#include <absl/strings/str_cat.h>
#include <absl/flags/flag.h>

// local includes
#include "camera.hpp"


namespace presage::camera {
bool AbslParseFlag(absl::string_view text, CaptureCodec* codec, std::string* error) {
    if (text == "mjpg" || text == "MJPG" || text == "mjpeg" || text == "MJPEG") {
        *codec = CaptureCodec::MJPG;
        return true;
    }
    if (text == "uyvy" || text == "UYVY" || text == "iyuv" || text == "IYUV") {
        *codec = CaptureCodec::UYVY;
        return true;
    }
    *error = "unknown value for enumeration";
    return false;
}

std::string AbslUnparseFlag(CaptureCodec codec) {
    switch (codec) {
        case CaptureCodec::MJPG:
            return "MJPG";
        case CaptureCodec::UYVY:
            return "UYVY";
        default:
            return absl::StrCat(codec);
    }
}

const std::vector<CaptureCodec> kCaptureCodecValues = {
    CaptureCodec::MJPG,
    CaptureCodec::UYVY
};

const std::vector<std::string> kCaptureCodecNames = []() {
    std::vector<std::string> names;
    for (const auto& codec: kCaptureCodecValues) {
        names.push_back(AbslUnparseFlag(codec));
    }
    return names;
}();
extern const std::string kCaptureCodecNameList = absl::StrJoin(kCaptureCodecNames, ", ");

// region ========================================== RESOLUTION ========================================================
bool AbslParseFlag(absl::string_view text, CameraResolutionRange* range, std::string* error) {
    if (text == "low" || text == "l") {
        *range = CameraResolutionRange::Low;
        return true;
    }
    if (text == "mid" || text == "m") {
        *range = CameraResolutionRange::Mid;
        return true;
    }
    if (text == "high" || text == "h") {
        *range = CameraResolutionRange::High;
        return true;
    }
    if (text == "ultra" || text == "u") {
        *range = CameraResolutionRange::Ultra;
        return true;
    }
    if (text == "4k") {
        *range = CameraResolutionRange::FourK;
        return true;
    }
    if (text == "giant" || text == "g") {
        *range = CameraResolutionRange::Giant;
        return true;
    }
    if (text == "complete" || text == "c") {
        *range = CameraResolutionRange::Complete;
        return true;
    }
    if (text == "unspecified") {
        *range = CameraResolutionRange::Unspecified_EnumEnd;
        return true;
    }
    *error = "unknown value for enumeration: " + std::string(text);
    return false;
}

std::string AbslUnparseFlag(CameraResolutionRange range) {
    switch (range) {
        case CameraResolutionRange::Low:
            return "low";
        case CameraResolutionRange::Mid:
            return "mid";
        case CameraResolutionRange::High:
            return "high";
        case CameraResolutionRange::Ultra:
            return "ultra";
        case CameraResolutionRange::FourK:
            return "4k";
        case CameraResolutionRange::Giant:
            return "giant";
        case CameraResolutionRange::Complete:
            return "complete";
        case CameraResolutionRange::Unspecified_EnumEnd:
            return "unspecified";
        default:
            return absl::StrCat(range);
    }
}

const std::vector<std::string> kCommonCameraResolutionRangeNames = []() {
    std::vector<std::string> range_names;
    for (
        CameraResolutionRange range = CameraResolutionRange::Low;
        range < CameraResolutionRange::Unspecified_EnumEnd;
        ++(int & )range
    ) {
//@formatter:off <-- work-around for Clion bug
        range_names.push_back(AbslUnparseFlag(range));
    }
//@formatter:on <-- work-around for Clion bug
    return range_names;
}();

const std::string kCommonCameraResolutionRangeNameList = absl::StrJoin(kCommonCameraResolutionRangeNames, ", ");
// endregion ===========================================================================================================
} // namespace
