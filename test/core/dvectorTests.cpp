// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "core/dvector.h"

#include <algorithm>

namespace jactorio
{
    /// Use to test container for operator calls
    struct TestClass
    {
        TestClass() {
            constructorCalls++;
        };
        ~TestClass() {
            assert(member_ == 0xAB ||
                   member_ == 0xE0E); // Ensures class is valid (Failed? -> Double free, invalid memory)
            destructorCalls++;
            member_ = 0xFE;
        }
        TestClass(const TestClass& other) {
            assert(other.member_ == 0xAB); // Ensures class is valid (Invalid invalid memory)
            copyCalls++;
        }
        TestClass(TestClass&& other) noexcept {
            assert(other.member_ == 0xAB); // Ensures class is valid (Invalid invalid memory)
            member_ = 0xE0E;
            moveCalls++;
        }

        TestClass& operator=(TestClass other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(TestClass& lhs, TestClass& rhs) noexcept {
            using std::swap;
            swap(lhs.member_, rhs.member_);
        }

        /// Resets counters of constructor destructor calls to 0
        static void ResetCounter() {
            constructorCalls = 0;
            destructorCalls  = 0;
            copyCalls        = 0;
            moveCalls        = 0;
        }

        static int constructorCalls;
        static int destructorCalls;
        static int copyCalls;
        static int moveCalls;

    private:
        /// Ensures calling on valid class
        int member_ = 0xAB;
    };
    int TestClass::destructorCalls  = 0;
    int TestClass::constructorCalls = 0;
    int TestClass::copyCalls        = 0;
    int TestClass::moveCalls        = 0;


    // Constructors

    TEST(DVector, DefaultConstruct) {
        DVector<TestClass> v;
        EXPECT_EQ(v.capacity(), 0);

        v.emplace_front();
        v.emplace_back();
    }

    TEST(DVector, ConstructCopiesOfElement) {
        DVector v(100, 42);

        EXPECT_EQ(v.size(), 100);

        int count = 0;
        for (auto& val : v) {
            EXPECT_EQ(val, 42);
            count++;
        }
        EXPECT_EQ(count, 100);
    }

    TEST(DVector, ConstructDefaultInsertElement) {
        TestClass::ResetCounter();

        DVector<TestClass> v(100);
        EXPECT_EQ(v.size(), 100);

        EXPECT_EQ(TestClass::constructorCalls, 100);
        EXPECT_EQ(TestClass::destructorCalls, 0);
    }

    TEST(DVector, ConstructFromIterators) {
        DVector<int> v1;
        v1.reserve(100);

        // Construct from [0, 99]
        for (int i = 0; i < 50; ++i) {
            v1.push_front(49 - i);
        }
        for (int i = 0; i < 50; ++i) {
            v1.push_back(50 + i);
        }

        DVector v2(v1.begin() + 2, v1.end() - 1);

        EXPECT_EQ(v2.size(), 97);
        for (int i = 0; i < 97; ++i) {
            EXPECT_EQ(v2.data()[i], i + 2);
        }
    }

    TEST(DVector, ConstructFromInitializerList) {
        DVector v{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        ASSERT_EQ(v.size(), 10);
        for (int i = 0; i < 10; ++i) {
            v[i] = i;
        }
    }

    TEST(DVector, ConstructDestructElements) {
        TestClass::ResetCounter();

        {
            DVector<TestClass> v;
            v.reserve(100);

            EXPECT_EQ(TestClass::constructorCalls, 0);
            EXPECT_EQ(TestClass::destructorCalls, 0);

            v.emplace_front();
            v.emplace_back();
            v.emplace_back();

            EXPECT_EQ(TestClass::constructorCalls, 3);
            EXPECT_EQ(TestClass::destructorCalls, 0);
        }

        EXPECT_EQ(TestClass::constructorCalls, 3);
        EXPECT_EQ(TestClass::destructorCalls, 3);
    }

    TEST(DVector, Copy) {
        DVector<int> v;
        v.push_front(1);
        v.push_front(2);

        auto v2 = v;

        EXPECT_EQ(v2[-1], 1);
        EXPECT_EQ(v2[-2], 2);
        EXPECT_EQ(v2.size(), 2);
        EXPECT_EQ(v2.capacity(), v.capacity());

        v2.push_front(3); // Ensures valid copy was performed (allows future operations)
        v2.push_back(10);
        EXPECT_EQ(v2[-3], 3);
        EXPECT_EQ(v2[0], 10);
    }

    TEST(DVector, Move) {
        DVector<int> v;
        v.push_back(1);
        v.push_back(2);

        auto v2 = std::move(v);
        EXPECT_EQ(v2[0], 1);
        EXPECT_EQ(v2[1], 2);
        EXPECT_EQ(v2.size(), 2);
        EXPECT_EQ(v2.capacity(), v.capacity());
        EXPECT_TRUE(v.empty());
    }

    TEST(DVector, MoveDestructorCalls) {
        TestClass::ResetCounter();

        {
            DVector<TestClass> v(2);
            {
                auto v2 = std::move(v);
                EXPECT_EQ(TestClass::moveCalls, 0);
            }
            EXPECT_EQ(TestClass::destructorCalls, 2);
        }
        EXPECT_EQ(TestClass::destructorCalls, 2);
    }

    TEST(DVector, AssignReplaceContentsWithValue) {
        DVector v(42, 0);

        v.assign(100, 6);

        ASSERT_EQ(v.size(), 100);
        for (int i = 0; i < 100; ++i) {
            EXPECT_EQ(v[i], 6);
        }
    }

    TEST(DVector, AssignReplaceContentsWithIteratorRange) {
        DVector v(42, TestClass());

        TestClass::ResetCounter();

        DVector<TestClass> v2;
        v2.emplace_front();
        v2.emplace_back();

        v2.assign(v.begin() + 2, v.end());
        EXPECT_EQ(TestClass::destructorCalls, 2); // Ensure previous 2 elements were deleted

        EXPECT_EQ(v2.size(), 40);
    }

    TEST(DVector, AssignReplaceContentsWithInitList) {
        DVector v(42, 0);

        v.assign({1, 2, 3, 4, 5});

        ASSERT_EQ(v.size(), 5);
        EXPECT_EQ(v[0], 1);
        EXPECT_EQ(v[1], 2);
        EXPECT_EQ(v[2], 3);
        EXPECT_EQ(v[3], 4);
        EXPECT_EQ(v[4], 5);
    }

    // Element access

    TEST(DVector, AtGetValid) {
        DVector<int> v;

        v.push_front(-10);
        v.push_back(0);
        v.push_back(20);

        EXPECT_EQ(v.at(-1), -10);
        EXPECT_EQ(v.at(0), 0);
        EXPECT_EQ(v.at(1), 20);
    }

    TEST(DVector, AtGetInvalid) {
        DVector<int> v;

        v.push_front(-1);
        v.push_back(0);
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        try {
            auto t = v.at(-2);
            FAIL();
        }
        catch (std::out_of_range&) {
            SUCCEED();
        }

        try {
            auto t = v.at(4);
            FAIL();
        }
        catch (std::out_of_range&) {
            SUCCEED();
        }
    }

    TEST(DVector, GetFront) {
        DVector<int> v;

        v.push_back(1);
        v.push_back(2);

        EXPECT_EQ(v.front(), 1);
    }

    TEST(DVector, GetBack) {
        DVector<int> v;

        v.push_front(1);
        v.push_back(2);
        v.push_back(3);

        EXPECT_EQ(v.back(), 3);
    }

    TEST(DVector, GetUnderlyingData) {
        DVector<int> v;
        v.reserve(100);

        for (int i = 0; i < 2; ++i) {
            v.push_front(i);
        }

        for (int i = 0; i < 10; ++i) {
            v.push_back(i + 100);
        }

        EXPECT_EQ(v.data()[0], 1);
        EXPECT_EQ(v.data()[1], 0);

        EXPECT_EQ(v.data()[2], 100);
        EXPECT_EQ(v.data()[3], 101);
        EXPECT_EQ(v.data()[4], 102);
        EXPECT_EQ(v.data()[5], 103);
        EXPECT_EQ(v.data()[6], 104);
        EXPECT_EQ(v.data()[7], 105);
        EXPECT_EQ(v.data()[8], 106);
        EXPECT_EQ(v.data()[9], 107);
        EXPECT_EQ(v.data()[10], 108);
        EXPECT_EQ(v.data()[11], 109);
    }

    // Iterators

    TEST(DVector, RangeIterate) {
        DVector<int> v;
        EXPECT_EQ(v.begin(), v.end());
        EXPECT_EQ(v.rbegin(), v.rend());

        v.push_front(1);
        v.push_front(0);
        v.push_back(2);
        v.push_back(3);

        int counter = 0;
        for (auto& element : v) {
            EXPECT_EQ(element, counter);
            ++counter;
        }
        EXPECT_EQ(counter, 4);

        counter = 0;
        for (auto it = v.cbegin(); it != v.cend(); ++it) {
            EXPECT_EQ(*it, counter);
            ++counter;
        }
        EXPECT_EQ(counter, 4);

        // Reverse
        counter = 0;
        for (auto it = v.rbegin(); it != v.rend(); ++it) {
            EXPECT_EQ(*it, 3 - counter);
            ++counter;
        }
        EXPECT_EQ(counter, 4);

        counter = 0;
        for (auto it = v.crbegin(); it != v.crend(); ++it) {
            EXPECT_EQ(*it, 3 - counter);
            ++counter;
        }
        EXPECT_EQ(counter, 4);
    }

    TEST(DVector, IteratorArithmetic) {
        DVector<std::pair<int, int>> v;

        v.push_front({10, -1}); // -1
        v.push_front({20, -2}); // -2
        v.push_front({30, -3}); // -3
        v.push_front({40, -4}); // -4

        v.push_back({0, 0});   // 0
        v.push_back({-10, 1}); // 1
        v.push_back({-20, 2}); // 2
        v.push_back({-30, 3}); // 3
        v.push_back({-40, 4}); // 4
        v.push_back({-50, 5}); // 5
        v.push_back({-60, 6}); // 6

        // Comment is index iterator is at BEFORE operation

        auto it = v.begin();
        EXPECT_EQ(it->first, 40); // -4


        EXPECT_EQ((++it)->first, 30); // -4

        EXPECT_EQ((it++)->first, 30); // -3
        EXPECT_EQ((*it).first, 20);   // -2

        EXPECT_EQ((--it)->first, 30); // -2

        EXPECT_EQ((it--)->first, 30); // -3
        EXPECT_EQ((*it).first, 40);   // -4

        it += 3; // -4
        EXPECT_EQ(it->first, 10);

        EXPECT_EQ((it + 5)->first, -40); // -1

        it -= 2;                  // -1
        EXPECT_EQ(it->first, 30); // -3

        EXPECT_EQ((it - 1)->first, 40); // -3

        EXPECT_EQ(it - (it + 100), -100);

        EXPECT_EQ(it[7].first, -40); // -3

        // Comparision

        EXPECT_TRUE(it == it);
        EXPECT_FALSE(it == (it + 10));

        EXPECT_TRUE(it != (it + 10));
        EXPECT_FALSE(it != it);

        EXPECT_TRUE(it < (it + 10));
        EXPECT_FALSE(it < (it - 10));
        EXPECT_FALSE(it < it);

        EXPECT_TRUE(it > (it - 10));
        EXPECT_FALSE(it > (it + 10));
        EXPECT_FALSE(it > it);

        EXPECT_TRUE(it <= (it + 10));
        EXPECT_FALSE(it <= (it - 10));
        EXPECT_TRUE(it <= it);

        EXPECT_TRUE(it >= (it - 10));
        EXPECT_FALSE(it >= (it + 10));
        EXPECT_TRUE(it >= it);
    }

    TEST(DVector, ReverseIteratorArithmetic) {
        DVector<std::pair<int, int>> v;

        v.push_front({10, -1}); // -1
        v.push_front({20, -2}); // -2
        v.push_front({30, -3}); // -3
        v.push_front({40, -4}); // -4

        v.push_back({0, 0});   // 0
        v.push_back({-10, 1}); // 1
        v.push_back({-20, 2}); // 2
        v.push_back({-30, 3}); // 3
        v.push_back({-40, 4}); // 4
        v.push_back({-50, 5}); // 5
        v.push_back({-60, 6}); // 6

        // Comment is index iterator is at BEFORE operation

        auto it = v.rbegin();
        EXPECT_EQ(it->first, -60); // 6


        EXPECT_EQ((++it)->first, -50); // 6

        EXPECT_EQ((it++)->first, -50); // 5
        EXPECT_EQ((*it).first, -40);   // 4

        EXPECT_EQ((--it)->first, -50); // 4

        EXPECT_EQ((it--)->first, -50); // 5
        EXPECT_EQ((*it).first, -60);   // 6

        it += 5;                   // 6
        EXPECT_EQ(it->first, -10); // 1

        EXPECT_EQ((it + 5)->first, 40); // 1

        it -= 2;                   // 1
        EXPECT_EQ(it->first, -30); // 3

        EXPECT_EQ((it - 1)->first, -40); // 3

        EXPECT_EQ(it - (it + 211), -211);

        EXPECT_EQ(it[-2].first, -50); // 3

        // Comparision

        EXPECT_TRUE(it == it);
        EXPECT_FALSE(it == (it + 10));

        EXPECT_TRUE(it != (it + 10));
        EXPECT_FALSE(it != it);

        EXPECT_TRUE(it < (it + 10));
        EXPECT_FALSE(it < (it - 10));
        EXPECT_FALSE(it < it);

        EXPECT_TRUE(it > (it - 10));
        EXPECT_FALSE(it > (it + 10));
        EXPECT_FALSE(it > it);

        EXPECT_TRUE(it <= (it + 10));
        EXPECT_FALSE(it <= (it - 10));
        EXPECT_TRUE(it <= it);

        EXPECT_TRUE(it >= (it - 10));
        EXPECT_FALSE(it >= (it + 10));
        EXPECT_TRUE(it >= it);
    }

    TEST(DVector, IteratorConstness) {
        DVector<int> v;

        static_assert(!std::is_const_v<std::remove_reference_t<decltype(*v.begin())>>);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(*v.cbegin())>>);

        static_assert(!std::is_const_v<std::remove_reference_t<decltype(*v.end())>>);
        static_assert(std::is_const_v<std::remove_reference_t<decltype(*v.cend())>>);

        // Test for implicit conversion from non const to const
        EXPECT_EQ(v.begin() - v.cbegin(), 0);
        EXPECT_EQ(v.begin(), v.cbegin());
        EXPECT_EQ(v.cbegin(), v.begin());

        EXPECT_EQ(v.rbegin(), v.crbegin());
        EXPECT_EQ(v.crbegin(), v.rbegin());
        EXPECT_EQ(v.begin(), v.crend().base() + 1);
    }

    // Capacity

    TEST(DVector, FrontBackSize) {
        DVector<int> v;
        EXPECT_EQ(v.size_front(), 0);
        EXPECT_EQ(v.size_back(), 0);

        v.push_front(-10);
        v.push_front(-20);

        v.push_back(0);
        v.push_back(10);
        v.push_back(20);
        v.push_back(30);

        EXPECT_EQ(v.size_front(), 2);
        EXPECT_EQ(v.size_back(), 4);
    }
    TEST(DVector, ShrinkCapacityFitSize) {
        DVector<int> v;
        v.reserve(100);

        v.push_back(0);
        v.push_back(0);

        v.shrink_to_fit();

        EXPECT_EQ(v.capacity(), 3);
    }

    // Modifiers

    TEST(DVector, Clear) {
        DVector<int> v;

        v.reserve(3);

        v.push_front(0);
        v.push_back(1);
        v.push_back(2);

        v.clear();

        v.push_front(9);
        v.push_back(10);
        EXPECT_EQ(v.capacity(), 3);
        EXPECT_EQ(v[-1], 9);
        EXPECT_EQ(v[0], 10);
    }

    TEST(DVector, ClearCallDestructor) {
        TestClass::ResetCounter();

        DVector<TestClass> v;
        v.reserve(3);

        v.emplace_front();
        v.emplace_back();
        v.emplace_back();

        v.clear();
        EXPECT_EQ(TestClass::destructorCalls, 3);
    }

    /*
    TEST(DVector, EmplaceMoveBack) {
        DVector v{1, 2, 3, 4, 5, 6};
        auto it = v.emplace(v.begin() + 3, 100);

        EXPECT_EQ(*it, 100);

        ASSERT_EQ(v.size(), 7);
        EXPECT_EQ(v[0], 1);
        EXPECT_EQ(v[1], 2);
        EXPECT_EQ(v[2], 3);
        EXPECT_EQ(v[3], 100);
        EXPECT_EQ(v[4], 4);
        EXPECT_EQ(v[5], 5);
        EXPECT_EQ(v[6], 6);
    }

    TEST(DVector, EmplaceOneBeforeEnd) {
        DVector v{1, 2, 3, 4};
        v.emplace(v.end() - 1, 100);

        ASSERT_EQ(v.size(), 5);
        EXPECT_EQ(v[0], 1);
        EXPECT_EQ(v[1], 2);
        EXPECT_EQ(v[2], 3);
        EXPECT_EQ(v[3], 100);
        EXPECT_EQ(v[4], 4);
    }
    TEST(DVector, EmplaceEnd) {
        DVector v{1, 2, 3, 4};
        v.emplace(v.end(), 100);

        ASSERT_EQ(v.size(), 5);
        EXPECT_EQ(v[0], 1);
        EXPECT_EQ(v[1], 2);
        EXPECT_EQ(v[2], 3);
        EXPECT_EQ(v[3], 4);
        EXPECT_EQ(v[4], 100);
    }

    TEST(DVector, EmplaceCallOperators) {
        TestClass::ResetCounter();

        DVector<TestClass> v(10);
        v.emplace(v.begin());

        EXPECT_EQ(TestClass::constructorCalls, 11);
        EXPECT_EQ(TestClass::destructorCalls, 11); // + 1 since must destruct temporary for strong exception guarantee
        EXPECT_EQ(TestClass::moveCalls, 11);
    }

    TEST(DVector, EraseElement) {
        DVector v{1, 2, 3, 4, 5};
        auto it = v.erase(v.begin() + 1);

        EXPECT_EQ(*(it + 1), 4);

        ASSERT_EQ(v.size(), 4);
        EXPECT_EQ(v[0], 1);
        EXPECT_EQ(v[1], 3);
        EXPECT_EQ(v[2], 4);
        EXPECT_EQ(v[3], 5);
    }

    TEST(DVector, EraseElementCallOperators) {
        TestClass::ResetCounter();

        DVector<TestClass> v(10);
        v.erase(v.begin(), v.begin() + 2);

        EXPECT_EQ(TestClass::constructorCalls, 10);
        EXPECT_EQ(TestClass::moveCalls, 7);
        EXPECT_EQ(TestClass::destructorCalls, 10);
    }

    TEST(DVector, EraseElementRange) {
        DVector v{1, 2, 3, 4, 5};
        auto it = v.erase(v.begin(), v.end() - 2);

        EXPECT_EQ(it, v.end() - 1);

        ASSERT_EQ(v.size(), 1);
        EXPECT_EQ(v[0], 5);
    }

    TEST(DVector, EraseElementEmptyRange) {
        DVector v{1, 2, 3, 4, 5};
        auto it = v.erase(v.begin(), v.begin() - 10);

        EXPECT_EQ(it, v.begin() - 10);

        ASSERT_EQ(v.size(), 5);
    }

    /// Checks did not move from unowned memory when erasing
    TEST(DVector, EraseElementValidMemoryAccess) {
        DVector<TestClass> v(5);
        v.erase(v.end() - 2, v.end() - 1);

        ASSERT_EQ(v.size(), 3);
    }
    */

    TEST(DVector, PushFront) {
        DVector<int> v;

        v.push_front(30);
        v.push_front(20);
        v.push_front(10);
        v.push_front(0);

        EXPECT_EQ(v[-4], 0);
        EXPECT_EQ(v[-3], 10);
        EXPECT_EQ(v[-2], 20);
        EXPECT_EQ(v[-1], 30);

        EXPECT_EQ(v.size(), 4);
    }

    TEST(DVector, PushBack) {
        DVector<int> v;

        v.push_back(30);
        v.push_back(20);
        v.push_back(10);
        v.push_back(0);

        EXPECT_EQ(v[0], 30);
        EXPECT_EQ(v[1], 20);
        EXPECT_EQ(v[2], 10);
        EXPECT_EQ(v[3], 0);

        EXPECT_EQ(v.size(), 4);
    }

    TEST(DVector, PushFrontAndBack) {
        DVector<int> v;
        EXPECT_TRUE(v.empty());

        v.push_front(30);
        v.push_front(20);
        v.push_back(40);
        v.push_back(50);
        v.push_back(60);
        v.push_front(10);
        v.push_front(0);

        EXPECT_EQ(v[-4], 0);
        EXPECT_EQ(v[-3], 10);
        EXPECT_EQ(v[-2], 20);
        EXPECT_EQ(v[-1], 30);
        EXPECT_EQ(v[0], 40);
        EXPECT_EQ(v[1], 50);
        EXPECT_EQ(v[2], 60);

        EXPECT_FALSE(v.empty());
        EXPECT_EQ(v.size(), 7);
    }

    TEST(DVector, PopFront) {
        DVector<int> v;
        v.push_front(3);
        v.push_front(2);
        v.push_front(1);

        v.pop_front();
        v.pop_front();
        v.push_front(40);

        EXPECT_EQ(v[-2], 40);
        EXPECT_EQ(v[-1], 3);
        EXPECT_EQ(v.size(), 2);
    }

    TEST(DVector, PopBack) {
        DVector<int> v;
        v.push_back(3);
        v.push_back(2);
        v.push_back(1);

        v.pop_back();
        v.pop_back();
        v.push_back(40);

        EXPECT_EQ(v[0], 3);
        EXPECT_EQ(v[1], 40);
        EXPECT_EQ(v.size(), 2);
    }

    TEST(DVector, FrontCallConstructorDestructor) {
        TestClass::ResetCounter();

        DVector<TestClass> v;
        v.reserve(20);

        v.emplace_front();
        EXPECT_EQ(TestClass::constructorCalls, 1);
        EXPECT_EQ(TestClass::destructorCalls, 0);

        v.pop_front();
        EXPECT_EQ(TestClass::constructorCalls, 1);
        EXPECT_EQ(TestClass::destructorCalls, 1);
    }

    TEST(DVector, BackCallConstructorDestructor) {
        TestClass::ResetCounter();

        DVector<TestClass> v;
        v.reserve(20);

        v.emplace_back();
        EXPECT_EQ(TestClass::constructorCalls, 1);
        EXPECT_EQ(TestClass::destructorCalls, 0);

        v.pop_back();
        EXPECT_EQ(TestClass::constructorCalls, 1);
        EXPECT_EQ(TestClass::destructorCalls, 1);
    }

    TEST(DVector, ResizeDown) {
        DVector<int> v;
        v.reserve(100);

        for (int i = 1; i < 20; ++i) {
            v.push_front(-i);
        }

        for (int i = 0; i < 20; ++i) {
            v.push_back(i);
        }

        v.resize(10);

        // [-1, -5]
        for (int i = 1; i < 6; ++i) {
            EXPECT_EQ(v[-i], -i);
        }

        // [0, 5]
        for (int i = 0; i < 6; ++i) {
            EXPECT_EQ(v[i], i);
        }
    }

    //

    // Tests DVector on a bunch of standard algorithms
    TEST(DVector, Algorithms) {
        DVector<int> v;
        v.assign(100, 0);

        std::fill(v.begin(), v.end(), 1);
        EXPECT_EQ(std::count(v.begin(), v.end(), 1), 100);
    }
} // namespace jactorio