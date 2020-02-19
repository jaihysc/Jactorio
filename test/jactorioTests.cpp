#include <gtest/gtest.h>

#include "core/filesystem.h"

int main(int ac, char* av[]) {
	jactorio::core::filesystem::set_executing_directory(av[0]);

	
	testing::InitGoogleTest(&ac, av);
	RUN_ALL_TESTS();
	return 0;  // Always return 0 even if tests fail, as a non-zero return code is seen as abnormal test suite exit
}
