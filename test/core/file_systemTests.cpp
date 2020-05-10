// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <gtest/gtest.h>

#include "core/filesystem.h"

namespace core
{
	class FileSystemTest : public testing::Test
	{
		std::string originalExecutingDir_;

	protected:
		void SetUp() override {
			// Certain tests (DataManager) require a path to files, store the original executing directory
			originalExecutingDir_ = jactorio::core::GetExecutingDirectory() + "/";
		}

		void TearDown() override {
			// Re- set the original executing directory
			jactorio::core::SetExecutingDirectory(originalExecutingDir_);
		}
	};

	TEST_F(FileSystemTest, WorkingDirectoryGetSet) {
#ifdef WIN32
		jactorio::core::SetExecutingDirectory("C:\\x\\y\\z\\b.exe");
		EXPECT_EQ(jactorio::core::GetExecutingDirectory(), "C:/x/y/z");
#endif

		jactorio::core::SetExecutingDirectory("abcdefghijkl;'");
		EXPECT_EQ(jactorio::core::GetExecutingDirectory(), "");

		jactorio::core::SetExecutingDirectory("x/y/z/b.exe");
		EXPECT_EQ(jactorio::core::GetExecutingDirectory(), "x/y/z");

		jactorio::core::SetExecutingDirectory("x/123456.exe");
		EXPECT_EQ(jactorio::core::GetExecutingDirectory(), "x");
	}

	TEST_F(FileSystemTest, ResolvePath) {
		// Certain tests require a path to files, store the original executing directory
		const auto original_path = jactorio::core::GetExecutingDirectory() + "/";


		jactorio::core::SetExecutingDirectory("antarctica/coolApplication.exe");

		EXPECT_EQ(jactorio::core::ResolvePath("~/glaciers/glacier1.png"), "antarctica/glaciers/glacier1.png");

		EXPECT_EQ(jactorio::core::ResolvePath("banana/banana1.png"), "banana/banana1.png");

		// Re- set the original executing directory
		jactorio::core::SetExecutingDirectory(original_path);
	}

	TEST_F(FileSystemTest, ReadFileStrInvalidPath) {
		if (!jactorio::core::ReadFile("").empty() ||
			!jactorio::core::ReadFile("someMysteriousFilePathThatDOESnotEXIST").empty()) {
			FAIL();
		}

		// Test does not check if the read contents are valid, it is assumed to be valid
	}
}
