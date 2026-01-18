//
// Created by greg on 6/2/25.
// Copyright (c) 2025 Presage Technologies
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/strings/string_view.h>


namespace presage::smartspectra::video_source {

/**
 * @brief Transformation applied to frames prior to processing.
 * \ingroup video_source
 */
enum class InputTransformMode : int {
    None,
    Clockwise90,
    Counterclockwise90,
    Rotate180,
    MirrorHorizontal,
    MirrorVertical,
    Unspecified_EnumEnd
};

/** Convert an input transform mode to a string for flags.
 *  \ingroup video_source
 */
std::string AbslUnparseFlag(InputTransformMode mode);
/** Parse an input transform mode from a flag string.
 *  \ingroup video_source
 */
bool AbslParseFlag(absl::string_view text, InputTransformMode* mode, std::string* error);

/** List of input transform mode names.
 *  \ingroup video_source
 */
extern const std::vector<std::string> kInputTransformModeNames;
/** Comma separated list of mode names.
 *  \ingroup video_source
 */
extern const std::string kInputTransformModeNameList;

} // namespace presage::smartspectra::video_source
