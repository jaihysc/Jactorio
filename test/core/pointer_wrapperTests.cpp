// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <gtest/gtest.h>

#include "core/pointer_wrapper.h"

#include <vector>

namespace jactorio::core
{
    static_assert(std::is_same_v<PointerWrapper<int>::element_type, int>, "Incorrect element_type");

    TEST(PointerWrapper, ConstructPointer) {
        int integer              = 1;
        PointerWrapper p_wrapper = &integer;
        EXPECT_EQ(p_wrapper.Get(), &integer);
    }

    TEST(PointerWrapper, ConstructReference) {
        int integer = 1;
        PointerWrapper p_wrapper(integer);
        EXPECT_EQ(p_wrapper.Get(), &integer);
    }

    TEST(PointerWrapper, GetSet) {
        PointerWrapper<std::vector<int>> p_wrapper;
        EXPECT_EQ(p_wrapper.Get(), nullptr);

        std::vector<int> vec_integer = {1, 2};

        p_wrapper = &vec_integer;
        p_wrapper->push_back(3);

        EXPECT_EQ(vec_integer.size(), 3);

        (*p_wrapper).push_back(4);
        EXPECT_EQ(vec_integer.size(), 4);
    }

    TEST(PointerWrapper, GetConstTy) {
        const std::vector<int> vec_integer = {1, 2};

        PointerWrapper<const std::vector<int>> p_wrapper(vec_integer);

        EXPECT_EQ(p_wrapper->size(), 2);
        EXPECT_EQ((*p_wrapper).size(), 2);
    }

    TEST(PointerWrapper, GetTyConstWrapper) {
        std::vector<int> vec_integer = {1, 2};

        const PointerWrapper<std::vector<int>> p_wrapper(vec_integer);

        p_wrapper->push_back(3);

        EXPECT_EQ(p_wrapper->size(), 3);
    }
} // namespace jactorio::core
