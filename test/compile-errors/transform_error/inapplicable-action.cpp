//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo.hpp"
#include "../../unit-tests/TestCommons.hpp"

/*
<begin-expected-compile-error>
The transform_error algorithm requires an action invocable with the expected's error\.
<end-expected-compile-error>
*/

void check()
{
    std::ignore = gimo::apply(
        gimo::testing::ExpectedFake{42},
        gimo::transform_error([]{ return 42; }));
}
