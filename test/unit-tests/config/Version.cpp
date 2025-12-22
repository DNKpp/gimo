//          Copyright Dominic (DNKpp) Koepke 2025 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/Version.hpp"

TEST_CASE(
"GIMO_VERSION_MAJOR is an integral number.",
"[config]")
{
    constexpr int version{GIMO_VERSION_MAJOR};
    STATIC_REQUIRE(0 <= version);
}

TEST_CASE(
    "GIMO_VERSION_MINOR is an integral number.",
    "[config]")
{
    constexpr int version{GIMO_VERSION_MINOR};
    STATIC_REQUIRE(0 <= version);
}

TEST_CASE(
    "GIMO_VERSION_PATCH is an integral number.",
    "[config]")
{
    constexpr int version{GIMO_VERSION_PATCH};
    STATIC_REQUIRE(0 <= version);
}

TEST_CASE(
    "Accumulated version is greater 0.",
    "[config]")
{
    constexpr int accumulatedVersion{GIMO_VERSION_MAJOR + GIMO_VERSION_MINOR + GIMO_VERSION_PATCH};
    STATIC_REQUIRE(0 < accumulatedVersion);
}
