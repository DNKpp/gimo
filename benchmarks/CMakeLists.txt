#          Copyright Dominic (DNKpp) Koepke 2025 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(CTest)

set(TARGET_NAME gimo-benchmarks)

add_executable(${TARGET_NAME}
    "main.cpp"
)

target_compile_features(${TARGET_NAME} PRIVATE
    cxx_std_23
)

include(EnableSanitizers)
enable_sanitizers(${TARGET_NAME})

include(EnableWarnings)
include(Gimo-get_cpm)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
CPMAddPackage("gh:google/benchmark@1.9.4")
CPMAddPackage("gh:martinus/nanobench@4.3.11")
target_link_libraries(${TARGET_NAME} PRIVATE
    nanobench::nanobench
    gimo::internal::enable-warnings

    gimo::gimo
)
