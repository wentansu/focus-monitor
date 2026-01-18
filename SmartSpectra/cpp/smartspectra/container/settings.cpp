//
// Created by greg on 1/12/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/strings/str_join.h>
#include <absl/flags/parse.h>
// === local includes (if any) ===
#include "settings.hpp"


namespace presage::smartspectra::container::settings {

bool AbslParseFlag(absl::string_view text, OperationMode* mode, std::string* error) {
    if (text == "spot" || text == "SPOT" || text == "Spot") {
        *mode = OperationMode::Spot;
        return true;
    }
    if (text == "continuous" || text == "CONTINUOUS" || text == "Continuous" || text == "cont") {
        *mode = OperationMode::Continuous;
        return true;
    }
    *error = "unknown value for enumeration";
    return false;
}

std::string AbslUnparseFlag(OperationMode mode) {
    switch(mode) {
        case OperationMode::Spot:
            return "spot";
        case OperationMode::Continuous:
            return "continuous";
        default:
            return absl::StrCat(mode);
    }
}

std::vector<std::string> GetOperationModeNames() {
    std::vector<std::string> names;
    for (int mode = static_cast<int>(OperationMode::Spot);
         mode < static_cast<int>(OperationMode::Unknown_EnumEnd);
         ++mode) {
        names.push_back(AbslUnparseFlag(static_cast<OperationMode>(mode)));
    }
    return names;
}


bool AbslParseFlag(absl::string_view text, IntegrationMode* mode, std::string* error) {
    if (text == "rest" || text == "REST" || text == "Rest") {
        *mode = IntegrationMode::Rest;
        return true;
    }
    if (text == "grpc" || text == "GRPC" || text == "Grpc") {
        *mode = IntegrationMode::Grpc;
        return true;
    }
    *error = "unknown value for enumeration";
    return false;
}

std::string AbslUnparseFlag(IntegrationMode mode) {
    switch(mode){
        case IntegrationMode::Rest:
            return "rest";
        case IntegrationMode::Grpc:
            return "grpc";
        default:
            return absl::StrCat(mode);
    }
}

std::vector<std::string> GetIntegrationModeNames() {
    std::vector<std::string> names;
    for (int mode = static_cast<int>(IntegrationMode::Rest);
         mode < static_cast<int>(IntegrationMode::Unknown_EnumEnd);
         ++mode) {
        names.push_back(AbslUnparseFlag(static_cast<IntegrationMode>(mode)));
    }
    return names;
}

bool AbslParseFlag(absl::string_view text, VideoSinkMode* mode, std::string* error) {
    if (text == "mjpg" || text == "MJPG" || text == "mjpeg" || text == "MJPEG") {
        *mode = VideoSinkMode::MJPG;
        return true;
    }
    if (text == "gstreamer" || text == "GSTREAMER" || text == "gstreamer-templated" || text == "GSTREAMER_TEMPLATED") {
        *mode = VideoSinkMode::GSTREAMER_TEMPLATED;
        return true;
    }
    if (text == "unknown" || text == "UNKNOWN" || text == "unspecified" || text == ""){
        *mode = VideoSinkMode::Unknown_EnumEnd;
        return true;
    }
    *error = "unknown value for enumeration";
    return false;
}

std::string AbslUnparseFlag(VideoSinkMode mode) {
    switch(mode) {
        case VideoSinkMode::MJPG:
            return "mjpg";
        case VideoSinkMode::GSTREAMER_TEMPLATED:
            return "gstreamer";
        case VideoSinkMode::Unknown_EnumEnd:
            return "unknown";
        default:
            return absl::StrCat(mode);
    }
}

std::vector<std::string> GetVideoSinkModeNames() {
    std::vector<std::string> names;
    for (int mode = static_cast<int>(VideoSinkMode::MJPG);
         mode < static_cast<int>(VideoSinkMode::Unknown_EnumEnd);
         ++mode) {
        names.push_back(AbslUnparseFlag(static_cast<VideoSinkMode>(mode)));
    }
    return names;
}



} // namespace presage::smartspectra::container::settings
