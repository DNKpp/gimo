//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo_ext/StdSharedPtr.hpp"
#include "gimo.hpp"

#include "TestCommons.hpp"

TEMPLATE_TEST_CASE(
    "std::shared_ptr satisfies the gimo::nullable requirements.",
    "[ext][std::shared_ptr]",
    std::shared_ptr<int>,
    std::shared_ptr<int const>,
    std::shared_ptr<std::unique_ptr<int>>)
{
    STATIC_CHECK(gimo::nullable<TestType>);
    STATIC_CHECK(gimo::nullable<TestType const>);
    STATIC_CHECK(gimo::nullable<TestType&>);
    STATIC_CHECK(gimo::nullable<TestType const&>);
    STATIC_CHECK(gimo::nullable<TestType&&>);
    STATIC_CHECK(gimo::nullable<TestType const&&>);
    STATIC_CHECK_FALSE(gimo::constructible_from_value<TestType, int>);
    STATIC_CHECK_FALSE(gimo::rebindable_value_to<TestType, int>);
    STATIC_CHECK_FALSE(gimo::expected_like<TestType>);
}

TEST_CASE(
    "std::shared_ptr<T[]> is unsupported.",
    "[ext][std::shared_ptr]")
{
    STATIC_CHECK_FALSE(gimo::nullable<std::shared_ptr<int[]>>);
}

TEST_CASE(
    "std::shared_ptr can not be used with transform algorithm",
    "[ext][std::shared_ptr]")
{
    STATIC_CHECK_FALSE(gimo::applicable_on<std::shared_ptr<int>, gimo::detail::transform_t<std::identity>>);
}

TEMPLATE_LIST_TEST_CASE(
    "std::shared_ptr can be used with and_then algorithm.",
    "[ext][std::shared_ptr]",
    gimo::testing::with_qualification_list)
{
    using with_qualification = TestType;
    static constexpr gimo::Pipeline pipeline = gimo::and_then(
        [](int const v) { return std::make_shared<float>(static_cast<float>(v + 1)); });

    SECTION("When a value is contained.")
    {
        auto ptr = std::make_shared<int>(42);

        std::shared_ptr const result = gimo::apply(
            with_qualification::cast(ptr),
            pipeline);

        STATIC_CHECK(std::same_as<std::shared_ptr<float> const, decltype(result)>);
        CHECK(43.f == *result);
    }

    SECTION("When nullptr is provided.")
    {
        std::shared_ptr<int> ptr{};

        std::shared_ptr const result = gimo::apply(
            with_qualification::cast(ptr),
            pipeline);

        STATIC_CHECK(std::same_as<std::shared_ptr<float> const, decltype(result)>);
        CHECK_FALSE(result);
    }
}

TEST_CASE(
    "std::shared_ptr can be used with or_else algorithm.",
    "[ext][std::shared_ptr]")
{
    static constexpr gimo::Pipeline pipeline = gimo::or_else(
        [] { return std::make_shared<int>(1337); });

    SECTION("When a value is contained.")
    {
        auto ptr = std::make_shared<int>(42);

        std::shared_ptr const result = gimo::apply(
            std::move(ptr),
            pipeline);

        STATIC_CHECK(std::same_as<std::shared_ptr<int> const, decltype(result)>);
        CHECK(43.f == *result);
    }

    SECTION("When nullptr is provided.")
    {
        std::shared_ptr<int> ptr{};

        std::shared_ptr const result = gimo::apply(
            std::move(ptr),
            pipeline);

        STATIC_CHECK(std::same_as<std::shared_ptr<int> const, decltype(result)>);
        CHECK_FALSE(result);
    }
}
