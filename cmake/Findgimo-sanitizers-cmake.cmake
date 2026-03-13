#          Copyright Dominic (DNKpp) Koepke 2025 - 2026.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

include(Gimo-get_cpm)

CPMAddPackage(
	NAME sanitizers-cmake
	GITHUB_REPOSITORY "arsenm/sanitizers-cmake"
	GIT_TAG "bcb1fc68616e9645ca5acea2992412606373ab04"
	SYSTEM YES
	EXCLUDE_FROM_ALL YES
	DOWNLOAD_ONLY YES
)

list(APPEND CMAKE_MODULE_PATH "${sanitizers-cmake_SOURCE_DIR}/cmake")

find_package(Sanitizers REQUIRED)
