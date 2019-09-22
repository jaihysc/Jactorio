#include <gtest/gtest.h>

#include "core/file_system.h"

TEST(file_system, read_file_strInvalidPath) {
	if (!read_file_as_str("").empty() || !read_file_as_str("someMysteriousFilePathThatDOESnotEXIST").empty())
		FAIL();

	// Test does not check if the read contents are valid, it is assumed to be valid
}