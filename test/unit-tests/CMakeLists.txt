#          Copyright Dominic (DNKpp) Koepke 2025 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

set(TARGET_NAME gimo-tests)

add_executable(${TARGET_NAME}
    "Common.cpp"
    "Pipeline.cpp"
)
add_subdirectory(algorithm)
add_subdirectory(config)
add_subdirectory(ext)

target_include_directories(${TARGET_NAME} PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

include(EnableWarnings)
find_package(Catch2 REQUIRED)
find_package(gimo-mimic++ REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE
    gimo::gimo
    gimo::internal::enable-warnings

    Catch2::Catch2WithMain
    mimicpp::mimicpp
)

# @formatter:off
string(CONCAT DISABLED_WARNINGS
    "$<"
        "$<AND:"
            "$<CXX_COMPILER_ID:GNU>,"
            "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,10>,"
            "$<NOT:$<CONFIG:Debug>>>:"
        "-Wno-maybe-uninitialized>"
)
#  @formatter:on

target_compile_options(${TARGET_NAME} PRIVATE
    # some test files exceed some limits
    # see: https://learn.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/fatal-error-c1128?view=msvc-170
    "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
    ${DISABLED_WARNINGS}
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    CATCH_CONFIG_FALLBACK_STRINGIFIER=mimicpp::print
)
target_precompile_headers(${TARGET_NAME} PRIVATE
    "pch.hpp"
)

catch_discover_tests(${TARGET_NAME})
