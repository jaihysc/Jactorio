#include <gtest/gtest.h>

#include "data/startup/data_cfg_parser.h"

TEST(startup_data, data_cfg_parser) {
	std::string str = 
		R"(graphics/grass/grass-1 = grass-1
		graphics/grass/grass-2 = grass-2
		graphics/grass/grass-3 = grass-3
		graphics/grass/grass-4 = grass-4

		audio/misc/sound-5 = sound-1)";

	jactorio::data::Data_cfg_parser parser = jactorio::data::Data_cfg_parser{};
	const jactorio::data::Cfg_data data = parser.parse(str, "asdf");

	EXPECT_EQ(data.count, 5);

	EXPECT_EQ(data.elements[0].id, "grass-1");
	EXPECT_EQ(data.elements[1].id, "grass-2");
	EXPECT_EQ(data.elements[2].id, "grass-3");
	EXPECT_EQ(data.elements[3].id, "grass-4");
	EXPECT_EQ(data.elements[4].id, "sound-1");

	EXPECT_EQ(data.elements[0].type, jactorio::data::data_type::graphics);
	EXPECT_EQ(data.elements[1].type, jactorio::data::data_type::graphics);
	EXPECT_EQ(data.elements[2].type, jactorio::data::data_type::graphics);
	EXPECT_EQ(data.elements[3].type, jactorio::data::data_type::graphics);
	EXPECT_EQ(data.elements[4].type, jactorio::data::data_type::audio);

	EXPECT_EQ(data.elements[0].path, "~/data/asdf/graphics/grass/grass-1");
	EXPECT_EQ(data.elements[1].path, "~/data/asdf/graphics/grass/grass-2");
	EXPECT_EQ(data.elements[2].path, "~/data/asdf/graphics/grass/grass-3");
	EXPECT_EQ(data.elements[3].path, "~/data/asdf/graphics/grass/grass-4");
	EXPECT_EQ(data.elements[4].path, "~/data/asdf/audio/misc/sound-5");
}

TEST(startup_data, data_cfg_parser_err) {
	std::string str = 
		R"(graphics/g1 = g1 graphics/r1 = r1

		graphics/r1 = r1
		audio/s5 =
		graphics/g2 g2
		asdfasfas/g4 = g4)";

	// Errors in above str
	// No newline
	// Valid ---
	// Missing data after =
	// Missing =
	// Invalid leading folder

    jactorio::data::Data_cfg_parser parser = jactorio::data::Data_cfg_parser{};
    const jactorio::data::Cfg_data data = parser.parse(str, "base");

	EXPECT_EQ(data.count, 1);

	EXPECT_EQ(data.elements[0].id, "r1");

	EXPECT_EQ(data.elements[0].type, jactorio::data::data_type::graphics);

	EXPECT_EQ(data.elements[0].path, "~/data/base/graphics/r1");

}
