// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#include <gtest/gtest.h>

#include "core/logger.h"

namespace core
{
	std::string RemoveTimestamp(std::string str) {
		int i = 0;
		while (str[i] < 'A' || str[i] > 'Z')
			i++;

		return str.substr(i);
	}

	TEST(Logger, CreateLogMessage) {
		std::string logged_message = GenLogMessage(
			jactorio::core::LogSeverity::error,
			"Some component, such as Networking",
			0,
			"Oh no, something went wrong");

		// Timestamp not tested
		// Substring away timestamp to first open square bracket

		EXPECT_EQ(RemoveTimestamp(logged_message),
		          "ERROR    [Some component, such as Networking:0] Oh no, something went wrong\033[0m\n");

		logged_message = GenLogMessage(
			jactorio::core::LogSeverity::info,
			"Renderer",
			10,
			"Just letting you know something");

		EXPECT_EQ(RemoveTimestamp(logged_message),
		          "Info     [Renderer:10] Just letting you know something\033[0m\n");

		LOG_MESSAGE(debug, "hello");
	}


	// TEST(logging, log_severityToStr) {
	// 	// No need to test every one of them
	// 	EXPECT_EQ(
	// 		jactorio::core::logger::log_severity_str(jactorio::core::logger::logSeverity::
	// 			critical), "CRITICAL");
	// 	EXPECT_EQ(
	// 		jactorio::core::logger::log_severity_str(jactorio::core::logger::logSeverity::debug),
	// 		"Debug   ");
	//
	// 	EXPECT_EQ(
	// 		jactorio::core::logger::log_severity_str(jactorio::core::logger::logSeverity::none),
	// 		"        ");
	// }
}
