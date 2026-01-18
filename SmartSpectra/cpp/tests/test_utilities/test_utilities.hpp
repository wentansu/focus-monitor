////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Created by greg on 9/3/24.
//    Based on https://github.com/Algomorph/NeuralTracking/blob/main/cpp/tests/test_utils/test_utils.hpp,
//    Which was created by Gregory Kramida (https://github.com/Algomorph) on 2/28/21, Copyright (c) 2021 Gregory Kramida
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
// === standard library includes (if any) ===
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
// === third-party includes (if any) ===
// === local includes (if any) ===
#pragma once

#include "compile_time_string_concatenation.hpp"
#include <test_data_paths.hpp>



namespace presage::smartspectra::test {
static constexpr auto generated_test_data_directory = StringFactory(GENERATED_TEST_DATA_DIRECTORY);
static constexpr auto generated_video_test_data_directory = StringFactory(GENERATED_TEST_DATA_DIRECTORY "videos");
static constexpr auto generated_image_test_data_directory = StringFactory(GENERATED_TEST_DATA_DIRECTORY "images");
static constexpr auto generated_json_test_data_directory = StringFactory(GENERATED_TEST_DATA_DIRECTORY "json");
static constexpr auto static_test_data_directory = StringFactory(STATIC_TEST_DATA_DIRECTORY);
static constexpr auto static_video_test_data_directory = StringFactory(STATIC_TEST_DATA_DIRECTORY "videos");
static constexpr auto static_image_test_data_directory = StringFactory(STATIC_TEST_DATA_DIRECTORY "images");
static constexpr auto static_array_test_data_directory = StringFactory(STATIC_TEST_DATA_DIRECTORY "arrays");
static constexpr auto static_json_test_data_directory = StringFactory(STATIC_TEST_DATA_DIRECTORY "json");

// index conversion for multidimensional arrays
std::vector<long> UnravelIndex(long linear_index, const std::vector<long>& dimensions);

template<typename TElement>
struct ArrayElementMismatchInformation{
    std::vector<long> position;
    long linear_index;
    TElement element1;
    TElement element2;
    float absolute_tolerance;
    float relative_tolerance;
};

struct ArrayDimensionMismatchInformation{
    std::vector<long> dimensions1;
    std::vector<long> dimensions2;
};

template<typename TElement>
struct ArrayComparisonResult{
    bool arrays_match;
    std::shared_ptr<ArrayDimensionMismatchInformation> dimension_mismatch_information;
    std::shared_ptr<ArrayElementMismatchInformation<TElement>> element_mismatch_information;
};

} // namespace presage::smartspectra::test
