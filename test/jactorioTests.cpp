#include <gtest/gtest.h>

// Remember to build project before running tests

int main(int ac, char* av[]) {
	testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}

TEST(exampleTestGroup, exampleText) {
	EXPECT_EQ(1, 0);  // This should fail
}