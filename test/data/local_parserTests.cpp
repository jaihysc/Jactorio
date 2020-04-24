// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/15/2020

#include <gtest/gtest.h>

#include "core/resource_guard.h"
#include "data/data_exception.h"
#include "data/data_manager.h"
#include "data/local_parser.h"
#include "data/prototype/sprite.h"

namespace data
{
	TEST(LocalParser, Parse) {
		auto guard = jactorio::core::Resource_guard(jactorio::data::clear_data);

		// Setup prototypes
		jactorio::data::set_directory_prefix("test");

		auto* prototype = new jactorio::data::Sprite();
		data_raw_add("test_tile", prototype, true);

		auto* prototype2 = new jactorio::data::Sprite();
		data_raw_add("test_tile1", prototype2, true);


		const std::string str =
			R"(test_tile=Test tile 1


		
				test_tile1=Test tile 2)";

		jactorio::data::local_parse(str, "test");

		// Validate local was set
		EXPECT_EQ(prototype->get_localized_name(), "Test tile 1");
		EXPECT_EQ(prototype2->get_localized_name(), "Test tile 2");
	}

	void expect_err(const std::string& str) {
		try {
			jactorio::data::local_parse(str, "asdf");
		}
		catch (jactorio::data::Data_exception&) {
			return;
		}

		// Failed to throw exception
		printf("Expected parse failure with: %s\n", str.c_str());
		FAIL();
	}

	TEST(LocalParser, ParseErr) {
		// Illegal character (=)
		// Missing r val after =
		// Missing r val
		// Missing l val

		expect_err("graphics/g1=g1 graphics/r1=r1");
		expect_err("audio/s5=");
		expect_err("graphics/g2 g2");
		expect_err("    =r2");
	}
}
