#          Copyright Dominic (DNKpp) Koepke 2025 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(Gimo-HasStdExpected)

target_sources(${TARGET_NAME} PRIVATE
    "RawPointer.cpp"
    "StdOptional.cpp"
    "StdUniquePtr.cpp"
)

if (GIMO_CONFIG_CXX_STANDARD GREATER_EQUAL 23
    AND GIMO_HAS_STD_EXPECTED)
    target_sources(${TARGET_NAME} PRIVATE
        "StdExpected.cpp"
    )
endif ()
