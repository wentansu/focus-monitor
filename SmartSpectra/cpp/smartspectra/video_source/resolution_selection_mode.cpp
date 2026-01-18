//
// Created by greg on 2/29/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/strings/str_join.h>
#include <absl/flags/parse.h>
// === local includes (if any) ===
#include "resolution_selection_mode.hpp"



namespace presage::smartspectra::video_source {

std::string AbslUnparseFlag(ResolutionSelectionMode mode) {
    switch (mode) {
        case ResolutionSelectionMode::Auto:
            return "auto";
        case ResolutionSelectionMode::Exact:
            return "exact";
        case ResolutionSelectionMode::Range:
            return "range";
        default:
            return absl::StrCat(mode);
    }
}

bool AbslParseFlag(absl::string_view text, ResolutionSelectionMode* mode, std::string* error) {
    if (text == "auto" || text == "a" || text == "AUTO" || text == "A") {
        *mode = ResolutionSelectionMode::Auto;
        return true;
    }
    if (text == "exact" || text == "e" || text == "EXACT" || text == "E") {
        *mode = ResolutionSelectionMode::Exact;
        return true;
    }
    if (text == "range" || text == "r" || text == "RANGE" || text == "R") {
        *mode = ResolutionSelectionMode::Range;
        return true;
    }
    *error =
        "Unknown value for enumeration. Possible values: " + absl::StrJoin(GetResolutionSelectionModeNames(), ", ");
    return false;
}

std::vector<std::string> GetResolutionSelectionModeNames() {
    std::vector<std::string> names;
    for (int mode = static_cast<int>(ResolutionSelectionMode::Exact);
         mode < static_cast<int>(ResolutionSelectionMode::Unknown_EnumEnd);
         ++mode) {
        names.push_back(AbslUnparseFlag(static_cast<ResolutionSelectionMode>(mode)));
    }
    return names;
}

} // namespace presage::smartspectra::video_source
