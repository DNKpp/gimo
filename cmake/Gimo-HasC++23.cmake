#          Copyright Dominic (DNKpp) Koepke 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(CheckCXXCompilerFlag)

function(gimo_check_cxx_23)
    if (MSVC)
        set(CXX23_FLAG "/std:c++latest")
    else ()
        set(CXX23_FLAG "-std=c++23")
    endif ()

    check_cxx_compiler_flag(${CXX23_FLAG} HAS_CXX_23)
    set(GIMO_HAS_CXX_23 ${HAS_CXX_23} PARENT_SCOPE)
endfunction()

gimo_check_cxx_23()
