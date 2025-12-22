//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo.hpp"
#include "gimo_ext/RawPointer.hpp"

/*
<begin-expected-compile-error>
The transform algorithm requires a nullable whose value-type can be rebound\.
<end-expected-compile-error>
*/

void check()
{
    constexpr int x{};
    std::ignore = gimo::apply(
        &x,
        gimo::transform([](int const v){ return static_cast<float>(42); }));
}
