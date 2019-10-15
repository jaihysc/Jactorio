#include <gtest/gtest.h>

#include "core/logger.h"

std::string remove_timestamp(std::string str) {
	int i = 0;
	while (str[i] != '[')
		i++;

	return str.substr(i);
}

TEST(logging, createLogMessage) {
	std::string logged_message = gen_log_message(
		jactorio::core::logger::error,
		"Some component, such as Networking",
		"Oh no, something went wrong");

	// Timestamp not tested
	// Substring away timestamp to first open square bracket

	EXPECT_EQ(remove_timestamp(logged_message),
	          "[ERROR   ] - [Some component, such as Networking] Oh no, something went wrong\n");

	logged_message = gen_log_message(
		jactorio::core::logger::info,
		"Renderer",
		"Just letting you know something");

	EXPECT_EQ(remove_timestamp(logged_message),
	          "[Info    ] - [Renderer] Just letting you know something\n");
}


TEST(logging, log_severityToStr) {
	// No need to test every one of them
	EXPECT_EQ(
		jactorio::core::logger::log_severity_str(jactorio::core::logger::
			critical), "CRITICAL");
	EXPECT_EQ(
		jactorio::core::logger::log_severity_str(jactorio::core::logger::debug),
		"Debug   ");
}
