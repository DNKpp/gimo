//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/ValueOrElse.hpp"
#include "gimo_ext/StdOptional.hpp"

#include "TestCommons.hpp"

using namespace gimo;

TEMPLATE_LIST_TEST_CASE(
    "value_or_else algorithm invokes its action only when the input has no value.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        int() &,
        int() const&,
        int() &&,
        int() const&&>
        action{};

    using Algorithm = detail::value_or_else_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_to<std::optional<int>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input has no value, the action is invoked.")
    {
        constexpr std::optional<int> opt{};

        SCOPED_EXP with_qualification::cast(action).expect_call()
            and finally::returns(42);

        Algorithm algorithm{std::move(action)};
        decltype(auto) result = with_qualification::cast(algorithm)(opt);
        STATIC_REQUIRE(std::same_as<int, decltype(result)>);
        CHECK(42 == result);
    }

    SECTION("When input has a value, that value is forwarded.")
    {
        Algorithm algorithm{std::move(action)};
        constexpr std::optional opt{1337};

        decltype(auto) result = with_qualification::cast(algorithm)(opt);
        STATIC_REQUIRE(std::same_as<int, decltype(result)>);
        CHECK(1337 == result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "value_or_else algorithm accepts nullables with any cv-ref qualification.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<int() const> const action{};

    using Algorithm = detail::value_or_else_t<decltype(std::cref(action))>;
    STATIC_REQUIRE(gimo::applicable_to<std::optional<int>, typename with_qualification::template type<Algorithm>>);

    Algorithm const algorithm{std::cref(action)};
    std::optional<int> opt{};

    SCOPED_EXP action.expect_call()
        and finally::returns(42);
    CHECK(42 == algorithm(with_qualification::cast(opt)));
}

TEMPLATE_LIST_TEST_CASE(
    "value_or_else algorithm supports expected_like types.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        int() &,
        int() const&,
        int() &&,
        int() const&&>
        action{};

    using Algorithm = detail::value_or_else_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_to<testing::ExpectedFake<int>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input holds an error, the action is invoked.")
    {
        auto const expected = testing::ExpectedFake<int>::from_error("An error.");

        SCOPED_EXP with_qualification::cast(action).expect_call()
            and finally::returns(42);

        Algorithm algorithm{std::move(action)};
        int const result = std::invoke(with_qualification::cast(algorithm), expected);
        CHECK(42 == result);
    }

    SECTION("When input holds a value, it is forwarded as-is.")
    {
        testing::ExpectedFake const expected{1337};
        Algorithm algorithm{std::move(action)};

        int const result = std::invoke(with_qualification::cast(algorithm), expected);
        CHECK(1337 == result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "gimo::value_or_else creates an appropriate pipeline.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<int() const> const inner{};
    auto action = [&] { return inner(); };
    using DummyAction = decltype(action);

    decltype(auto) pipeline = value_or_else(with_qualification::cast(action));
    STATIC_CHECK(std::same_as<Pipeline<detail::value_or_else_t<DummyAction>>, decltype(pipeline)>);
    STATIC_CHECK(gimo::applicable_to<std::optional<int>, detail::value_or_else_t<DummyAction>>);
    STATIC_CHECK(gimo::processable_by<std::optional<int>, decltype(pipeline)>);

    SCOPED_EXP inner.expect_call()
        and finally::returns(1337);
    CHECK(1337 == pipeline.apply(std::optional<int>{}));
}

TEMPLATE_LIST_TEST_CASE(
    "value_or algorithm extracts the contained value or returns the alternative.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    using Algorithm = detail::value_or_t<float>;
    STATIC_REQUIRE(gimo::applicable_to<std::optional<float>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input has a value, the contained value is forwarded.")
    {
        constexpr std::optional opt{1337.f};

        Algorithm algorithm{42.f};
        decltype(auto) result = with_qualification::cast(algorithm)(opt);
        STATIC_REQUIRE(std::same_as<float, decltype(result)>);
        CHECK(1337.f == result);
    }

    SECTION("When input is empty, the alternative is forwarded.")
    {
        constexpr std::optional<float> opt{};
        Algorithm algorithm{42.f};

        decltype(auto) result = with_qualification::cast(algorithm)(opt);
        STATIC_REQUIRE(std::same_as<float, decltype(result)>);
        CHECK(42.f == result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "gimo::value_or creates an appropriate pipeline.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    float alternative{42.f};

    decltype(auto) pipeline = value_or(with_qualification::cast(alternative));
    STATIC_CHECK(std::same_as<Pipeline<detail::value_or_t<float>>, decltype(pipeline)>);
    STATIC_CHECK(gimo::applicable_to<std::optional<float>, detail::value_or_t<float>>);
    STATIC_CHECK(gimo::processable_by<std::optional<float>, decltype(pipeline)>);

    CHECK(1337.f == pipeline.apply(std::optional<float>{1337.f}));
}
