#include <gtest/gtest.h>

#include <iostream>

#include "core/filesystem.h"

int main(int ac, char* av[]) {
	jactorio::core::filesystem::set_executing_directory(av[0]);

	
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}
