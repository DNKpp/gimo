//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#undef GIMO_ASSERT

#include "gimo.hpp"
#include "gimo_ext/StdOptional.hpp"

TEST_CASE("Testing gimo integration.")
{
    int const result = gimo::apply(
        std::optional{1337},
        gimo::and_then([](int const /*x*/) { return std::optional{42.f}; })
            | gimo::or_else([] { throw 42; })
            | gimo::transform([](float const x) { return static_cast<int>(x); })
            | gimo::value_or(-42));
    CHECK(42 == result);
}
