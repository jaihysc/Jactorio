// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/15/2020

#include <gtest/gtest.h>

#include "data/data_exception.h"
#include "data/data_manager.h"
#include "data/local_parser.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	class LocalParserTest : public testing::Test
	{
	protected:
		PrototypeManager dataManager_;
	};

	TEST_F(LocalParserTest, Parse) {
		// Setup prototypes
		dataManager_.SetDirectoryPrefix("test");

		auto* prototype = new Sprite();
		dataManager_.DataRawAdd("test_tile", prototype, true);

		auto* prototype2 = new Sprite();
		dataManager_.DataRawAdd("test_tile1", prototype2, true);


		const std::string str =
			R"(test_tile=Test tile 1


		
				test_tile1=Test tile 2)";

		LocalParse(dataManager_, str, "test");

		// Validate local was set
		EXPECT_EQ(prototype->GetLocalizedName(), "Test tile 1");
		EXPECT_EQ(prototype2->GetLocalizedName(), "Test tile 2");
	}

	void ExpectErr(PrototypeManager& data_manager, const std::string& str) {
		try {
			LocalParse(data_manager, str, "asdf");
		}
		catch (DataException&) {
			return;
		}

		// Failed to throw exception
		printf("Expected parse failure with: %s\n", str.c_str());
		FAIL();
	}

	TEST_F(LocalParserTest, ParseErr) {
		// Illegal character (=)
		// Missing r val after =
		// Missing r val
		// Missing l val

		ExpectErr(dataManager_, "graphics/g1=g1 graphics/r1=r1");
		ExpectErr(dataManager_, "audio/s5=");
		ExpectErr(dataManager_, "graphics/g2 g2");
		ExpectErr(dataManager_, "    =r2");
	}
}
