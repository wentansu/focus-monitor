//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// json_file_io.h
// Created by Greg on 2/21/2024.
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
// === third-party includes ===
#include <nlohmann/json.hpp>

namespace presage::smartspectra::container::json_file_io {
/**
 * @brief Serialize JSON data to disk with a short description logged.
 */
void WriteJsonDataToFile(
    const nlohmann::json& json_data,
    const std::string& output_file_name,
    const std::string& short_data_description
);

/**
 * @brief Serialize JSON data to disk without a description.
 */
void WriteJsonDataToFile(
    const nlohmann::json& json_data,
    const std::string& output_file_name
);
} // namespace presage::smartspectra::container::json_file_io
