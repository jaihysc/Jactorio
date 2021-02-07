// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/utility.h"

namespace jactorio
{
    TEST(Utility, StrToLower) {
        EXPECT_EQ(StrToLower("ASDADxcxvcASDJAKDJ:AL"), "asdadxcxvcasdjakdj:al");
    }

    TEST(Utility, StrToUpper) {
        EXPECT_EQ(StrToUpper("ASDADxcxvcASDJAKDJ:AL"), "ASDADXCXVCASDJAKDJ:AL");
    }

    TEST(Utility, StrTrim) {
        EXPECT_EQ(StrTrim("aaaabcdefgbbbb", {'a', 'b'}), "cdefg");
        EXPECT_EQ(StrTrim("11112222fff22221113", {'1', '2'}), "fff22221113");
    }

    TEST(Utility, StrTrimWhitespace) {
        std::string str = "     abcdefg    ";
        str.resize(100);

        EXPECT_EQ(StrTrimWhitespace(str), "abcdefg");
        EXPECT_EQ(StrTrimWhitespace(str).find("abcdefg"), 0);

        EXPECT_EQ(StrTrimWhitespace("asdf                      "), "asdf");
        EXPECT_TRUE(StrTrimWhitespace("            ").empty());
        EXPECT_TRUE(StrTrimWhitespace("").empty());
    }

    TEST(Utility, StrMatchLen) {
        EXPECT_EQ(StrMatchLen("abc", 10), "abc       ");
        EXPECT_EQ(StrMatchLen("abc", 0), "abc");
        EXPECT_EQ(StrMatchLen("", 5, '-'), "-----");
    }
} // namespace jactorio
