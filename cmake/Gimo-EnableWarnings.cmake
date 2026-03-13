#          Copyright Dominic (DNKpp) Koepke 2025 - 2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

if (NOT TARGET gimo-internal-enable-warnings)

	add_library(gimo-internal-enable-warnings INTERFACE)
	add_library(gimo::internal::enable-warnings ALIAS gimo-internal-enable-warnings)

	# We need to circumvent the huge nonsense warnings from clang-cl
	# see: https://discourse.cmake.org/t/wall-with-visual-studio-clang-toolchain-results-in-way-too-many-warnings/7927
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
		AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")

		set(WARNING_FLAGS /W4 -Wextra -Wpedantic -Werror -Wno-unknown-attributes)
	else()
		set(WARNING_FLAGS
			$<IF:$<CXX_COMPILER_ID:MSVC>,
			/W4 /WX,
			-Wall -Wextra -Wpedantic -Werror>
		)
	endif()

	target_compile_options(gimo-internal-enable-warnings INTERFACE
		${WARNING_FLAGS}
	)

endif()
