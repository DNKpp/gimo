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
#include <expected>
int main()
{
    return std::expected<float, int>{4.2f}.has_value();
}
")
    check_cxx_source_compiles("${CODE}" HAS_STD_EXPECTED)
    set(GIMO_HAS_STD_EXPECTED ${HAS_STD_EXPECTED} PARENT_SCOPE)
endfunction()

gimo_check_std_optional_monadic()
