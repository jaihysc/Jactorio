#include <gtest/gtest.h>

#include <iostream>

int main(int ac, char* av[]) {
	std::cout << "Make sure that the working directory is the same directory as the jactorioTest executable"
	<< "\n\n";
	
    testing::InitGoogleTest(&ac, av);
	return RUN_ALL_TESTS();
}