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
		auto guard = jactorio::core::ResourceGuard(jactorio::data::ClearData);

		// Setup prototypes
		jactorio::data::SetDirectoryPrefix("test");

		auto* prototype = new jactorio::data::Sprite();
		DataRawAdd("test_tile", prototype, true);

		auto* prototype2 = new jactorio::data::Sprite();
		DataRawAdd("test_tile1", prototype2, true);


		const std::string str =
			R"(test_tile=Test tile 1


		
				test_tile1=Test tile 2)";

		jactorio::data::LocalParse(str, "test");

		// Validate local was set
		EXPECT_EQ(prototype->GetLocalizedName(), "Test tile 1");
		EXPECT_EQ(prototype2->GetLocalizedName(), "Test tile 2");
	}

	void expect_err(const std::string& str) {
		try {
			jactorio::data::LocalParse(str, "asdf");
		}
		catch (jactorio::data::DataException&) {
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
