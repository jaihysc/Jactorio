// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <gtest/gtest.h>

#include "core/filesystem.h"

namespace core
{
	class FileSystemTest : public testing::Test
	{
		std::string original_executing_dir_;

	protected:
		void SetUp() override {
			// Certain tests (DataManager) require a path to files, store the original executing directory
			original_executing_dir_ = jactorio::core::get_executing_directory() + "/";
		}

		void TearDown() override {
			// Re- set the original executing directory
			jactorio::core::set_executing_directory(original_executing_dir_);
		}
	};

	TEST_F(FileSystemTest, WorkingDirectoryGetSet) {
#ifdef WIN32
		jactorio::core::set_executing_directory("C:\\x\\y\\z\\b.exe");
		EXPECT_EQ(jactorio::core::get_executing_directory(), "C:/x/y/z");
#endif

		jactorio::core::set_executing_directory("abcdefghijkl;'");
		EXPECT_EQ(jactorio::core::get_executing_directory(), "");

		jactorio::core::set_executing_directory("x/y/z/b.exe");
		EXPECT_EQ(jactorio::core::get_executing_directory(), "x/y/z");

		jactorio::core::set_executing_directory("x/123456.exe");
		EXPECT_EQ(jactorio::core::get_executing_directory(), "x");
	}

	TEST_F(FileSystemTest, ResolvePath) {
		// Certain tests require a path to files, store the original executing directory
		const auto original_path = jactorio::core::get_executing_directory() + "/";


		jactorio::core::set_executing_directory("antarctica/coolApplication.exe");

		EXPECT_EQ(jactorio::core::resolve_path("~/glaciers/glacier1.png"), "antarctica/glaciers/glacier1.png");

		EXPECT_EQ(jactorio::core::resolve_path("banana/banana1.png"), "banana/banana1.png");

		// Re- set the original executing directory
		jactorio::core::set_executing_directory(original_path);
	}

	TEST_F(FileSystemTest, ReadFileStrInvalidPath) {
		if (!jactorio::core::read_file_as_str("").empty() ||
			!jactorio::core::read_file_as_str("someMysteriousFilePathThatDOESnotEXIST").empty()) {
			FAIL();
		}

		// Test does not check if the read contents are valid, it is assumed to be valid
	}
}
