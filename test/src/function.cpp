#include "stdext/traits.h"
#include <stdext/function.h>

#include <catch2/catch_template_test_macros.hpp>
#include <type_traits>


namespace test
{
    namespace
    {
        int f0(int x) noexcept { return x; }
        int f1(int x) noexcept { return -x; }
    }

    TEMPLATE_TEST_CASE("function_proxy (common)", "[function]", std::false_type, std::true_type)
    {
        static_assert(!std::is_default_constructible_v<stdext::function_proxy<void () noexcept(TestType())>>);
        static_assert(std::is_move_constructible_v<stdext::function_proxy<void () noexcept(TestType())>>);
        static_assert(std::is_copy_constructible_v<stdext::function_proxy<void () noexcept(TestType())>>);
        static_assert(std::is_move_assignable_v<stdext::function_proxy<void () noexcept(TestType())>>);
        static_assert(std::is_copy_assignable_v<stdext::function_proxy<void () noexcept(TestType())>>);
        static_assert(std::is_swappable_v<stdext::function_proxy<void () noexcept(TestType())>>);
        static_assert(stdext::is_equality_comparable_v<stdext::function_proxy<void () noexcept(TestType())>>);
        static_assert(std::is_invocable_v<stdext::function_proxy<void () noexcept(TestType())>>);

        SECTION("invocation calls bound object")
        {
            stdext::function_proxy<int (int) noexcept(TestType())> a = f0;
            CHECK(a(0) == 0);
            CHECK(a(1) == 1);
            CHECK(a(-1) == -1);

            stdext::function_proxy<int (int) noexcept(TestType())> b = f1;
            CHECK(b(0) == 0);
            CHECK(b(1) == -1);
            CHECK(b(-1) == 1);
        }

        SECTION("comparisons")
        {
            stdext::function_proxy<int (int) noexcept(TestType())> a = f0;
            stdext::function_proxy<int (int) noexcept(TestType())> b = f1;

            CHECK(a == a);
            CHECK_FALSE(a != a);
            CHECK(b == b);
            CHECK_FALSE(b != b);

            CHECK(a != b);
            CHECK_FALSE(a == b);
            CHECK(b != a);
            CHECK_FALSE(b == a);

            stdext::function_proxy<int (int) noexcept(TestType())> c = f0;

            CHECK(a == c);
            CHECK_FALSE(a != c);
            CHECK(c == a);
            CHECK_FALSE(c != a);
        }

        SECTION("copy construction")
        {
            stdext::function_proxy<int (int) noexcept(TestType())> a = f0;
            stdext::function_proxy<int (int) noexcept(TestType())> b = a;
            CHECK(b == a);
            CHECK(b(0) == 0);
            CHECK(b(1) == 1);
            CHECK(b(-1) == -1);
        }

        SECTION("copy assignment")
        {
            stdext::function_proxy<int (int) noexcept(TestType())> a = f0;
            stdext::function_proxy<int (int) noexcept(TestType())> b = f1;
            CHECK(b != a);

            b = a;
            CHECK(b == a);
        }

        SECTION("construction from non-function callable")
        {
            int x = 3;
            auto f = [&]() noexcept { return x++; };
            stdext::function_proxy<int () noexcept(TestType())> a = f;
            CHECK(a() == 3);
            CHECK(x == 4);
        }

        SECTION("callable return converted to void")
        {
            auto f = []() noexcept { return 5; };
            stdext::function_proxy<void () noexcept(TestType())> a = f;
            a();

            a = +f;
            a();
        }

        SECTION("swap")
        {
            stdext::function_proxy<int (int) noexcept(TestType())> a = f0;
            stdext::function_proxy<int (int) noexcept(TestType())> b = f1;
            CHECK(a(1) == 1);
            CHECK(b(1) == -1);

            stdext::swap(a, b);
            CHECK(a(1) == -1);
            CHECK(b(1) == 1);
        }
    }

    TEST_CASE("function_proxy", "[function]")
    {
        SECTION("binds to throwing callable")
        {
            auto f = []{ return 42; };
            stdext::function_proxy<int ()> a = f;
            CHECK(a() == 42);

            stdext::function_proxy<int ()> b = +f;
            CHECK(b() == 42);
        }
    }
}
