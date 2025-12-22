//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo_ext/RawPointer.hpp"
#include "gimo.hpp"
#include "gimo_ext/StdOptional.hpp"

TEMPLATE_TEST_CASE(
    "raw-pointers satisfies the gimo::nullable requirements.",
    "[ext][raw-ptr]",
    int*,
    int const*,
    int* const,
    int const* const,
    int**,
    int* const*)
{
    STATIC_CHECK(gimo::nullable<TestType>);
    STATIC_CHECK_FALSE(gimo::constructible_from_value<TestType, int>);
    STATIC_CHECK_FALSE(gimo::rebindable_value_to<TestType, int&&>);
    STATIC_CHECK_FALSE(gimo::expected_like<TestType>);
}

TEST_CASE(
    "raw-pointers can not be used with transform algorithm",
    "[ext][raw-ptr]")
{
    STATIC_REQUIRE(!gimo::applicable_on<int const*, gimo::detail::transform_t<std::identity>>);
}

TEST_CASE(
    "raw-pointers can be used with and_then algorithm",
    "[ext][raw-ptr]")
{
    constexpr int x{42};

    constexpr std::optional result = gimo::apply(
        &x,
        gimo::and_then([](int const& v) { return std::optional{v + 1}; }));
    STATIC_CHECK(43 == result);
}

TEST_CASE(
    "raw-pointers can be used with or_else algorithm",
    "[ext][raw-ptr]")
{
    constexpr int const* x = nullptr;
    static constexpr int y{1337};

    constexpr int const* result = gimo::apply(
        x,
        gimo::or_else([] { return &y; }));
    STATIC_CHECK(1337 == *result);
}
