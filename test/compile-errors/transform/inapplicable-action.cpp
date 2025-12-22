//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo.hpp"
#include "gimo_ext/StdOptional.hpp"

/*
<begin-expected-compile-error>
The transform action must be invocable with the value of the nullable\.
<end-expected-compile-error>
*/

void check()
{
    std::ignore = gimo::apply(
        std::optional{1337},
        gimo::transform([]{ return 42; }));
}
