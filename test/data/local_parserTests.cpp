// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/local_parser.h"
#include "data/prototype_manager.h"
#include "proto/sprite.h"

namespace jactorio::data
{
    class LocalParserTest : public testing::Test
    {
    protected:
        PrototypeManager proto_;
    };

    TEST_F(LocalParserTest, Parse) {
        // Setup prototypes
        proto_.SetDirectoryPrefix("test");

        auto& prototype  = proto_.Make<proto::Sprite>("test_tile");
        auto& prototype2 = proto_.Make<proto::Sprite>("test_tile1");

        const std::string str =
            R"(test_tile=Test tile 1


		
				test_tile1=Test tile 2)";

        LocalParse(proto_, str, "test");

        // Validate local was set
        EXPECT_EQ(prototype.GetLocalizedName(), "Test tile 1");
        EXPECT_EQ(prototype2.GetLocalizedName(), "Test tile 2");
    }

    void ExpectErr(PrototypeManager& proto, const std::string& str) {
        try {
            LocalParse(proto, str, "asdf");
        }
        catch (proto::ProtoError&) {
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

        ExpectErr(proto_, "graphics/g1=g1 graphics/r1=r1");
        ExpectErr(proto_, "audio/s5=");
        ExpectErr(proto_, "graphics/g2 g2");
        ExpectErr(proto_, "    =r2");
    }
} // namespace jactorio::data
