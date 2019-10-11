#include <gtest/gtest.h>

#include "core/file_system.h"

TEST(file_system, working_directory_get_set) {
	jactorio::core::File_system::set_executing_directory("C:\\x\\y\\z\\b.exe");
	EXPECT_EQ(jactorio::core::File_system::get_executing_directory(), "C:/x/y/z");

	
	jactorio::core::File_system::set_executing_directory("abcdefghijkl;'");
	EXPECT_EQ(jactorio::core::File_system::get_executing_directory(), "");

	jactorio::core::File_system::set_executing_directory("x/y/z/b.exe");
	EXPECT_EQ(jactorio::core::File_system::get_executing_directory(), "x/y/z");

	jactorio::core::File_system::set_executing_directory("x/123456.exe");
	EXPECT_EQ(jactorio::core::File_system::get_executing_directory(), "x");
}

TEST(file_system, resolve_path) {
	jactorio::core::File_system::set_executing_directory("antarctica/coolApplication.exe");
	
	EXPECT_EQ(
		jactorio::core::File_system::resolve_path("~/glaciers/glacier1.png"),
		"antarctica/glaciers/glacier1.png"
	);

	EXPECT_EQ(
		jactorio::core::File_system::resolve_path("banana/banana1.png"),
		"banana/banana1.png"
	);
}

TEST(file_system, read_file_strInvalidPath) {
	if (!jactorio::core::File_system::read_file_as_str("").empty() || 
		!jactorio::core::File_system::read_file_as_str("someMysteriousFilePathThatDOESnotEXIST").empty())
		FAIL();

	// Test does not check if the read contents are valid, it is assumed to be valid
}