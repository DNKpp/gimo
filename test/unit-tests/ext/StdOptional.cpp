//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo_ext/StdOptional.hpp"
#include "gimo.hpp"

#include "../unit-tests/TestCommons.hpp"

TEMPLATE_TEST_CASE(
    "std::optional satisfies the gimo::nullable requirements.",
    "[ext][std::optional]",
    std::optional<int>,
    std::optional<std::optional<int>>)
{
    STATIC_CHECK(gimo::nullable<TestType>);
    STATIC_CHECK(gimo::constructible_from_value<TestType, int>);
    STATIC_CHECK(gimo::rebindable_value_to<TestType, int>);
    STATIC_CHECK_FALSE(gimo::expected_like<TestType>);
}

TEMPLATE_LIST_TEST_CASE(
    "std::optional can be used with transform algorithm.",
    "[ext][std::optional]",
    gimo::testing::with_qualification_list)
{
    using with_qualification = TestType;
    static constexpr gimo::Pipeline pipeline = gimo::transform(
        [](int const v) { return static_cast<float>(v + 1); });
    STATIC_CHECK(gimo::processable_by<std::optional<int>, decltype(pipeline)>);

    SECTION("When a value is contained.")
    {
        constexpr std::optional result = [] {
            std::optional opt{42};

            return gimo::apply(
                with_qualification::cast(opt),
                pipeline);
        }();

        STATIC_CHECK(std::same_as<std::optional<float> const, decltype(result)>);
        STATIC_CHECK(43.f == result);
    }

    SECTION("When nullopt is provided.")
    {
        constexpr std::optional result = [] {
            std::optional<int> opt{};

            return gimo::apply(
                with_qualification::cast(opt),
                pipeline);
        }();

        STATIC_CHECK(std::same_as<std::optional<float> const, decltype(result)>);
        STATIC_CHECK_FALSE(result.has_value());
    }
}

TEMPLATE_LIST_TEST_CASE(
    "std::optional can be used with and_then algorithm.",
    "[ext][std::optional]",
    gimo::testing::with_qualification_list)
{
    using with_qualification = TestType;
    static constexpr gimo::Pipeline pipeline = gimo::and_then(
        [](int const v) { return std::optional{static_cast<float>(v + 1)}; });
    STATIC_CHECK(gimo::processable_by<std::optional<int>, decltype(pipeline)>);

    SECTION("When a value is contained.")
    {
        constexpr std::optional result = [] {
            std::optional opt{42};

            return gimo::apply(
                with_qualification::cast(opt),
                pipeline);
        }();

        STATIC_CHECK(std::same_as<std::optional<float> const, decltype(result)>);
        STATIC_CHECK(43.f == result);
    }

    SECTION("When nullopt is provided.")
    {
        constexpr std::optional result = [] {
            std::optional<int> opt{};

            return gimo::apply(
                with_qualification::cast(opt),
                pipeline);
        }();

        STATIC_CHECK(std::same_as<std::optional<float> const, decltype(result)>);
        STATIC_CHECK_FALSE(result.has_value());
    }
}

TEMPLATE_LIST_TEST_CASE(
    "std::optional can be used with or_else algorithm.",
    "[ext][std::optional]",
    gimo::testing::with_qualification_list)
{
    using with_qualification = TestType;
    static constexpr gimo::Pipeline pipeline = gimo::or_else(
        [] { return std::optional{1337}; });
    STATIC_CHECK(gimo::processable_by<std::optional<int>, decltype(pipeline)>);

    SECTION("When a value is contained.")
    {
        constexpr std::optional result = [] {
            std::optional opt{42};

            return gimo::apply(
                with_qualification::cast(opt),
                pipeline);
        }();

        STATIC_CHECK(std::same_as<std::optional<int> const, decltype(result)>);
        STATIC_CHECK(42 == result);
    }

    SECTION("When nullopt is provided.")
    {
        constexpr std::optional result = [] {
            std::optional<int> opt{};

            return gimo::apply(
                with_qualification::cast(opt),
                pipeline);
        }();

        STATIC_CHECK(std::same_as<std::optional<int> const, decltype(result)>);
        STATIC_CHECK(1337 == result);
    }
}
