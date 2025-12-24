#          Copyright Dominic (DNKpp) Koepke 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET gimo-internal-test-flags)
    add_library(gimo-internal-test-flags INTERFACE)
    add_library(gimo::internal::enable-test-flags ALIAS gimo-internal-test-flags)

    if (GIMO_TEST_ADDITIONAL_COMPILER_FLAGS)
        message(DEBUG "${MESSAGE_PREFIX} enabled additional compiler-flags: ${GIMO_TEST_ADDITIONAL_COMPILER_FLAGS}")
        target_compile_options(gimo-internal-test-flags INTERFACE
            ${GIMO_TEST_ADDITIONAL_COMPILER_FLAGS}
        )
    endif ()

    if (GIMO_TEST_ADDITIONAL_LINKER_FLAGS)
        message(DEBUG "${MESSAGE_PREFIX} enabled additional linker-flags: ${GIMO_TEST_ADDITIONAL_LINKER_FLAGS}")
        target_link_options(gimo-internal-test-flags INTERFACE
            ${GIMO_TEST_ADDITIONAL_LINKER_FLAGS}
        )
    endif ()
endif ()
