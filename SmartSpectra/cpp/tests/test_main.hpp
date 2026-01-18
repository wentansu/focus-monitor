////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Created by greg on 9/3/24.
//   Based on https://github.com/Algomorph/NeuralTracking/blob/main/cpp/tests/test_main.hpp,
//   which was created by Gregory Kramida, under Apache 2 License, (c) 2021 Gregory Kramida
//
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

#define CATCH_CONFIG_RUNNER

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>


// Necessary to start running the test(s) as a python program
int main(int argc, char* argv[]) {
#ifdef SMARTSPECTRA_TEST_USE_PYTHON
    wchar_t* program = Py_DecodeLocale(argv[0], nullptr);
	if (program == nullptr) {
		fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
		exit(1);
	}
	Py_SetProgramName(program);  /* optional but recommended */
	Py_Initialize();
#endif
    int result = Catch::Session().run(argc, argv);
#ifdef SMARTSPECTRA_TEST_USE_PYTHON
    if (Py_FinalizeEx() < 0) {
		exit(120);
	}
	PyMem_RawFree(program);
#endif
    return (result < 0xff ? result : 0xff);
}
