//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo_ext/StdExpected.hpp"
#include "gimo.hpp"

TEST_CASE(
    "std::expected satisfies the gimo::expected_like requirements.",
    "[ext]")
{
    using expected = std::expected<int, std::string>;

    STATIC_CHECK(gimo::expected_like<expected>);
    STATIC_CHECK(gimo::expected_like<expected const>);
    STATIC_CHECK(gimo::expected_like<expected&>);
    STATIC_CHECK(gimo::expected_like<expected const&>);
    STATIC_CHECK(gimo::expected_like<expected&&>);
    STATIC_CHECK(gimo::expected_like<expected const&&>);

    STATIC_CHECK(gimo::nullable<expected>);
    STATIC_CHECK(gimo::nullable<expected const>);
    STATIC_CHECK(gimo::nullable<expected&>);
    STATIC_CHECK(gimo::nullable<expected const&>);
    STATIC_CHECK(gimo::nullable<expected&&>);
    STATIC_CHECK(gimo::nullable<expected const&&>);
}
