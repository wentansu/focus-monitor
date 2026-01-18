//
// Created by greg on 6/2/25.
// Copyright (c) 2025 Presage Technologies
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/strings/str_join.h>
#include <absl/flags/parse.h>
#include <mediapipe/framework/port/logging.h>
// === local includes (if any) ===
#include "input_transform.hpp"
namespace presage::smartspectra::video_source {

std::string AbslUnparseFlag(InputTransformMode mode) {
    switch (mode) {
        case InputTransformMode::None:
            return "none";
        case InputTransformMode::Clockwise90:
            return "cw90";
        case InputTransformMode::Counterclockwise90:
            return "ccw90";
        case InputTransformMode::Rotate180:
            return "rotate180";
        case InputTransformMode::MirrorHorizontal:
            return "mirror_horizontal";
        case InputTransformMode::MirrorVertical:
            return "mirror_vertical";
        case InputTransformMode::Unspecified_EnumEnd:
            return "Unspecified_EnumEnd";
        default:
            return absl::StrCat(mode);
    }
}

bool AbslParseFlag(absl::string_view text, InputTransformMode* mode, std::string* error) {
    if (text == "none" || text == "None" || text == "NONE" || text == "OFF" || text == "off") {
        *mode = InputTransformMode::None;
        return true;
    }
    if (text == "clockwise90" || text == "cw90" || text == "CLOCKWISE90" || text == "CW90" || text == "ClockWise90" ||
        text == "90") {
        *mode = InputTransformMode::Clockwise90;
        return true;
    }
    if (text == "counterclockwise90" || text == "ccw90" || text == "COUNTERCLOCKWISE90" || text == "CCW90" ||
        text == "CounterClockWise90") {
        *mode = InputTransformMode::Counterclockwise90;
        return true;
    }
    if (text == "rotate180" || text == "180" || text == "ROTATE180" || text == "rot180" || text == "cw180") {
        *mode = InputTransformMode::Rotate180;
        return true;
    }
    if (text == "mirror_horizontal" || text == "mirror_h" || text == "MIRROR_HORIZONTAL" || text == "MH" || text == "mh") {
        *mode = InputTransformMode::MirrorHorizontal;
        return true;
    }
    if (text == "mirror_vertical" || text == "mirror_v" || text == "MIRROR_VERTICAL" || text == "MV" || text == "mv") {
        *mode = InputTransformMode::MirrorVertical;
        return true;
    }
    if (text == "Unspecified_EnumEnd") {
        *mode = InputTransformMode::Unspecified_EnumEnd;
        return true;
    }
    *error = "unknown value for enumeration";
    return false;
}

const std::vector<std::string> kInputTransformModeNames = []() {
    std::vector<std::string> names;
    for (
        InputTransformMode mode = static_cast<InputTransformMode>(0);
        mode < InputTransformMode::Unspecified_EnumEnd;
        ++(int & )mode
    ) {
//@formatter:off <-- work-around for Clion bug
        names.push_back(AbslUnparseFlag(mode));
    }
//@formatter:on <-- work-around for Clion bug
    return names;
}();
const std::string kInputTransformModeNameList = absl::StrJoin(kInputTransformModeNames, ", ");

} // namespace presage::smartspectra::video_source
