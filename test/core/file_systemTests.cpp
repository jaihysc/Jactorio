// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <gtest/gtest.h>

#include "core/filesystem.h"

namespace core
{
	TEST(file_system, working_directory_get_set) {
		// Certain tests require a path to files, store the original executing directory
		const auto original_path = jactorio::core::filesystem::get_executing_directory() + "/";

#ifdef WIN32
		jactorio::core::filesystem::set_executing_directory("C:\\x\\y\\z\\b.exe");
		EXPECT_EQ(jactorio::core::filesystem::get_executing_directory(), "C:/x/y/z");
#endif

		jactorio::core::filesystem::set_executing_directory("abcdefghijkl;'");
		EXPECT_EQ(jactorio::core::filesystem::get_executing_directory(), "");

		jactorio::core::filesystem::set_executing_directory("x/y/z/b.exe");
		EXPECT_EQ(jactorio::core::filesystem::get_executing_directory(), "x/y/z");

		jactorio::core::filesystem::set_executing_directory("x/123456.exe");
		EXPECT_EQ(jactorio::core::filesystem::get_executing_directory(), "x");

		// Re- set the original executing directory
		jactorio::core::filesystem::set_executing_directory(original_path);
	}

	TEST(file_system, resolve_path) {
		// Certain tests require a path to files, store the original executing directory
		const auto original_path = jactorio::core::filesystem::get_executing_directory() + "/";


		jactorio::core::filesystem::set_executing_directory("antarctica/coolApplication.exe");

		EXPECT_EQ(
			jactorio::core::filesystem::resolve_path("~/glaciers/glacier1.png"),
			"antarctica/glaciers/glacier1.png"
		);

		EXPECT_EQ(
			jactorio::core::filesystem::resolve_path("banana/banana1.png"),
			"banana/banana1.png"
		);

		// Re- set the original executing directory
		jactorio::core::filesystem::set_executing_directory(original_path);
	}

	TEST(file_system, read_file_strInvalidPath) {
		if (!jactorio::core::filesystem::read_file_as_str("").empty() ||
			!jactorio::core::filesystem::read_file_as_str("someMysteriousFilePathThatDOESnotEXIST").empty())
			FAIL();

		// Test does not check if the read contents are valid, it is assumed to be valid
	}
}
