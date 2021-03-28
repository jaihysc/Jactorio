// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <gtest/gtest.h>

#include "core/convert.h"

namespace jactorio
{
    TEST(Convert, SafeCast) {
        auto a = SafeCast<int>(42);
        auto c = SafeCast<uint64_t>(1); // Widening cast

        // const auto b = SafeCast<float>(static_cast<double>(321321312));  // Error: Must be integral
        // auto d = SafeCast<uint8_t>(256);  // Error: Loss of data
    }

    TEST(Convert, SafeDowncast) {
        class Base
        {
        public:
            virtual ~Base() = default;
        };

        class A : public Base
        {
        };

        class B : public Base
        {
        };

        Base base;
        A a;
        B b;

        Base* base_ptr          = &base;
        Base* a_ptr             = &a;
        const Base* const_a_ptr = &a;
        Base* b_ptr             = &b;

        // Pointers
        EXPECT_EQ(SafeCast<const A*>(const_a_ptr), const_a_ptr);
        EXPECT_EQ(SafeCast<B*>(b_ptr), b_ptr);

        // EXPECT_EQ(SafeCast<A>(a_ptr), a_ptr); // Provided type not a pointer
        // EXPECT_EQ(SafeCast<A>(base_ptr), base_ptr); // Error: Cannot downcast to A

        // References
        EXPECT_EQ(&SafeCast<const A&>(*const_a_ptr), const_a_ptr);
        EXPECT_EQ(&SafeCast<B&>(*b_ptr), b_ptr);

        // EXPECT_EQ(&SafeCast<A>(*a_ptr), a_ptr); // Provided type not a reference
        // EXPECT_EQ(&SafeCast<A>(*base_ptr), base_ptr); // Error: Cannot downcast to A
    }

    TEST(Convert, SafeCastAssign) {
        auto a = 42;
        auto c = 1ull;

        SafeCastAssign(a, c);

        EXPECT_EQ(a, c);
    }
} // namespace jactorio
