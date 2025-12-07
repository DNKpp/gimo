//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/Common.hpp"
#include "gimo_ext/std_optional.hpp"

#include "TestCommons.hpp"

// see: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2445r1.pdf
TEMPLATE_TEST_CASE_SIG(
    "const_ref_like_t merges const and adapts the value-category of T.",
    "[trait]",
    ((typename Expected, typename T, typename U), Expected, T, U),
    (int&&, float, int),
    (int&, float&, int),
    (int&&, float&&, int),

    // 4
    (int const&&, float const, int),
    (int const&, float const&, int),
    (int const&&, float const&&, int),

    // 7
    (int const&&, float, int const),
    (int const&, float&, int const),
    (int const&&, float&&, int const),

    // 10
    (int const&&, float const, int const),
    (int const&, float const&, int const),
    (int const&&, float const&&, int const),

    // 13
    (int&, float&, int&),
    (int&, float&, int&&),

    // 15
    (int const&, float&, int const&),
    (int const&, float&, int const&&),

    // 17
    (int const&, float const&, int const&),
    (int const&, float const&, int const&&),

    // 19
    (int&&, float, int&),
    (int&&, float&&, int&),

    (int const&&, float const, int&),
    (int const&, float const&, int&),
    (int const&&, float const&&, int&),

    // 24
    (int&&, float, int&&),
    (int&&, float&&, int&&),

    (int const&&, float const, int&&),
    (int const&, float const&, int&&),
    (int const&&, float const&&, int&&),

    // 29
    (int const&&, float, int const&),
    (int const&&, float&&, int const&),
    (int const&&, float const, int const&),
    (int const&&, float const&&, int const&),

    // 33
    (int const&&, float, int const&&),
    (int const&&, float&&, int const&&),
    (int const&&, float const, int const&&),
    (int const&&, float const&&, int const&&))
{
    STATIC_CHECK(std::same_as<Expected, gimo::detail::const_ref_like_t<T, U>>);
    STATIC_CHECK(std::same_as<Expected, decltype(gimo::detail::forward_like<T>(std::declval<U>()))>);
}

TEMPLATE_TEST_CASE_SIG(
    "nullable determines whether the specified type satisfies the requirements.",
    "[concept]",
    ((bool expected, typename T), expected, T),
    (false, int),
    (false, std::nullopt_t),
    (true, std::optional<int>),
    (true, gimo::testing::ExpectedFake<int>))
{
    STATIC_CHECK(expected == gimo::nullable<T>);
    STATIC_CHECK(expected == gimo::nullable<T const>);
    STATIC_CHECK(expected == gimo::nullable<T&>);
    STATIC_CHECK(expected == gimo::nullable<T const&>);
    STATIC_CHECK(expected == gimo::nullable<T&&>);
    STATIC_CHECK(expected == gimo::nullable<T const&&>);
}

TEMPLATE_TEST_CASE_SIG(
    "expected_like determines whether the specified type satisfies the requirements.",
    "[concept]",
    ((bool expected, typename T), expected, T),
    (false, int),
    (false, std::nullopt_t),
    (false, std::optional<int>),
    (true, gimo::testing::ExpectedFake<int>))
{
    STATIC_CHECK(expected == gimo::expected_like<T>);
    STATIC_CHECK(expected == gimo::expected_like<T const>);
    STATIC_CHECK(expected == gimo::expected_like<T&>);
    STATIC_CHECK(expected == gimo::expected_like<T const&>);
    STATIC_CHECK(expected == gimo::expected_like<T&&>);
    STATIC_CHECK(expected == gimo::expected_like<T const&&>);
}
