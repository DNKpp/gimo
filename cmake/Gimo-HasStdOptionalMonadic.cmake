#          Copyright Dominic (DNKpp) Koepke 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(CheckCXXSourceCompiles)
include(Gimo-HasC++23)

function(gimo_check_std_optional_monadic)
    if (NOT GIMO_HAS_CXX_23)
        return()
    endif ()

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(CODE "
#include <optional>
int main()
{
    return std::optional<int>{1}
        .and_then([](int const v){ return std::optional{v}; })
        .transform([](int const v){ return v; })
        .or_else([]{ return std::optional{1337}; })
        .value();
}
")
    check_cxx_source_compiles("${CODE}" HAS_STD_OPTIONAL_MONADIC)
    set(GIMO_HAS_STD_OPTIONAL_MONADIC ${HAS_STD_OPTIONAL_MONADIC} PARENT_SCOPE)
endfunction()

gimo_check_std_optional_monadic()
