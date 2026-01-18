//
// Created by greg on 2/21/24.
// Copyright (c) 2024 Presage Technologies
//

// === standard library includes (if any) ===
#include <fstream>
// === third-party includes (if any) ===
#include <mediapipe/framework/port/logging.h>
// === local includes (if any) ===
#include "json_file_io.hpp"

namespace presage::smartspectra::container::json_file_io {

void WriteJsonDataToFile(
    const nlohmann::json& json_data,
    const std::string& output_file_name,
    const std::string& short_data_description
) {
    WriteJsonDataToFile(json_data, output_file_name);
    LOG(INFO) << "JSON for " << short_data_description << " written to file: " << output_file_name;
}

void WriteJsonDataToFile(
    const nlohmann::json& json_data,
    const std::string& output_file_name
) {
    std::string json_as_string = json_data.dump();
    std::ofstream output_file(output_file_name.c_str());
    output_file << json_as_string;
    output_file.close();
}

} // namespace presage::smartspectra::container::json_file_io

