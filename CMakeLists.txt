#          Copyright Dominic (DNKpp) Koepke 2025 - 2025.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.15)
set(MESSAGE_PREFIX "gimo:")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GimoReadVersion)
gimo_read_version("include/gimo/Version.hpp" GIMO_VERSION)

project(gimo
	LANGUAGES CXX
	VERSION ${GIMO_VERSION}
	DESCRIPTION "TBD"
	HOMEPAGE_URL "https://github.com/DNKpp/gimo"
)

message(STATUS "${MESSAGE_PREFIX} version: v${PROJECT_VERSION} from: ${gimo_SOURCE_DIR}")

include(GNUInstallDirs)

add_library(gimo INTERFACE)
add_library(gimo::gimo ALIAS gimo)
target_include_directories(gimo INTERFACE
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

set(GIMO_CONFIG_CXX_STANDARD 20 CACHE STRING "Which c++ standard to use.")
target_compile_features(gimo INTERFACE
	cxx_std_${GIMO_CONFIG_CXX_STANDARD}
)

string(COMPARE EQUAL "${gimo_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" IS_TOP_LEVEL_PROJECT)

include(Gimo-HasStdOptionalMonadic)

option(GIMO_BUILD_TESTS "Determines, whether the tests shall be built." ${IS_TOP_LEVEL_PROJECT})
if (GIMO_BUILD_TESTS)
	include(CTest)
	add_subdirectory("test")
endif()

option(GIMO_BUILD_BENCHMARKS "Determines, whether the benchmarks shall be built." ${IS_TOP_LEVEL_PROJECT})
if (GIMO_BUILD_BENCHMARKS AND GIMO_HAS_STD_OPTIONAL_MONADIC)
	include(CTest)
	add_subdirectory("benchmarks")
endif()

option(GIMO_CONFIGURE_DOXYGEN "Determines, whether the doxyfile shall be configured." OFF)
if (GIMO_CONFIGURE_DOXYGEN)
	option(GIMO_ENABLE_GENERATE_DOCS "Enable the doxygen documentation target." OFF)
	add_subdirectory("docs")
endif()

option(GIMO_ENABLE_AMALGAMATE_HEADERS "Enables the amalgamation target." OFF)
if (GIMO_ENABLE_AMALGAMATE_HEADERS)
	add_subdirectory("tools/amalgamate-headers")
endif ()

include(CMakeDependentOption)
cmake_dependent_option(GIMO_ENABLE_INSTALL_RULES "Enables install rules." ON "NOT CMAKE_SKIP_INSTALL_RULES" OFF)
if (GIMO_ENABLE_INSTALL_RULES)
  include(GimoInstallRules)
endif()
