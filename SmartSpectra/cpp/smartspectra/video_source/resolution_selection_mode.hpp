//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// resolution_selection_mode.h
// Created by Greg on 2/29/24.
// Copyright (C) 2024 Presage Security, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
// === standard library includes (if any) ===
// === third-party includes (if any) ===
#include <absl/strings/string_view.h>
// === local includes (if any) ===
namespace presage::smartspectra::video_source {

/**
 * How capture resolution should be selected.
 * \ingroup video_source
 */
enum class ResolutionSelectionMode : int {
    Auto,
    Exact,
    Range,
    Unknown_EnumEnd
};

/** Convert a resolution selection mode to a string for flags.
 *  \ingroup video_source
 */
std::string AbslUnparseFlag(ResolutionSelectionMode mode);
/** Parse a resolution selection mode from a flag.
 *  \ingroup video_source
 */
bool AbslParseFlag(absl::string_view text, ResolutionSelectionMode* mode, std::string* error);

/** Names of the available resolution selection modes.
 *  \ingroup video_source
 */
std::vector<std::string> GetResolutionSelectionModeNames();

} // namespace presage::smartspectra::video_source
