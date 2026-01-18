////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Created by greg on 9/3/24.
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

// === standard library includes (if any) ===
#include <algorithm>
// === third-party includes (if any) ===
// === local includes (if any) ===
#include "test_utilities_impl.hpp"
namespace presage::smartspectra::test {


// index conversions
std::vector<long> UnravelIndex(long linear_index, const std::vector<long>& dimensions) {
    std::vector<long> position;
    long dividend = linear_index;

    for (auto iterator = dimensions.rbegin(); iterator != dimensions.rend(); ++iterator) {
        long dimension = *iterator;
        long remainder = dividend % dimension;
        position.push_back(remainder);
        dividend = dividend / dimension;
    }

    std::reverse(position.begin(), position.end());
    return position;
}

}  // namespace presage::smartspectra::test
