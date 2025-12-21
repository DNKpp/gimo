//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/OrElse.hpp"
#include "gimo_ext/StdOptional.hpp"

#include "TestCommons.hpp"

using namespace gimo;

TEMPLATE_LIST_TEST_CASE(
    "or_else algorithm invokes its action only when the input has no value.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        std::optional<int>()&,
        std::optional<int>() const&,
        std::optional<int>()&&,
        std::optional<int>() const&&>
        action{};

    using Algorithm = detail::or_else_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_on<std::optional<int>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input has no value, the action is invoked.")
    {
        constexpr std::optional<int> opt{};

        SCOPED_EXP with_qualification::cast(action).expect_call()
            and finally::returns(std::optional{42});

        Algorithm orElse{std::move(action)};
        decltype(auto) result = with_qualification::cast(orElse)(opt);
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(42 == result);
    }

    SECTION("When input has a value, the action is not invoked.")
    {
        Algorithm orElse{std::move(action)};
        constexpr std::optional opt{1337};

        decltype(auto) result = with_qualification::cast(orElse)(opt);
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "or_else algorithm accepts nullables with any cv-ref qualification.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<std::optional<int>() const> const action{};

    using Algorithm = detail::or_else_t<decltype(std::cref(action))>;
    STATIC_REQUIRE(gimo::applicable_on<std::optional<int>, typename with_qualification::template type<Algorithm>>);

    Algorithm const orElse{std::cref(action)};
    std::optional<int> opt{};

    SCOPED_EXP action.expect_call()
        and finally::returns(std::optional{42});
    decltype(auto) result = orElse(with_qualification::cast(opt));
    STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
    CHECK(42 == result);
}

TEMPLATE_LIST_TEST_CASE(
    "or_else algorithm forwards additional steps as-is.",
    "[algorith]",
    testing::with_qualification_list)
{
    using matches::_;
    using with_qualification = TestType;

    auto const step0 = detail::or_else_t<decltype([] { return std::optional{43}; })>{};
    using StepMock = testing::AlgorithmMock<std::identity>;
    using StepRef = with_qualification::template type<StepMock>;
    using ActionRef = with_qualification::template type<std::identity>;
    StepMock step1{};
    StepMock step2{};

    SECTION("When input has a value.")
    {
        auto& on_value = testing::AlgorithmMockTraits::on_value_<ActionRef, std::optional<int>&&, StepRef>;
        SCOPED_EXP on_value.expect_call(_, 42, matches::instance(step2))
            and finally::returns(1337);

        decltype(auto) result = std::invoke(
            step0,
            std::optional{42},
            with_qualification::cast(step1),
            with_qualification::cast(step2));
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(1337 == result);
    }

    SECTION("When input is empty.")
    {
        auto& on_value = testing::AlgorithmMockTraits::on_value_<ActionRef, std::optional<int>&&, StepRef>;
        SCOPED_EXP on_value.expect_call(_, 43, matches::instance(step2))
            and finally::returns(1337);

        decltype(auto) result = std::invoke(
            step0,
            std::optional<int>{},
            with_qualification::cast(step1),
            with_qualification::cast(step2));
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(1337 == result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "or_else algorithm supports expected_like types.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        testing::ExpectedFake<int>()&,
        testing::ExpectedFake<int>() const&,
        testing::ExpectedFake<int>()&&,
        testing::ExpectedFake<int>() const&&>
        action{};

    using Algorithm = detail::or_else_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_on<testing::ExpectedFake<int>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input holds an error, the action is invoked.")
    {
        auto const expected = testing::ExpectedFake<int>::from_error("An error.");

        SCOPED_EXP with_qualification::cast(action).expect_call()
            and finally::returns(42);

        Algorithm orElse{std::move(action)};
        decltype(auto) result = std::invoke(with_qualification::cast(orElse), expected);
        STATIC_REQUIRE(std::same_as<testing::ExpectedFake<int>, decltype(result)>);
        CHECK(42 == *result);
    }

    SECTION("When input holds a value, it is forwarded as-is.")
    {
        testing::ExpectedFake const expected{1337};
        Algorithm orElse{std::move(action)};

        decltype(auto) result = std::invoke(with_qualification::cast(orElse), expected);
        STATIC_REQUIRE(std::same_as<testing::ExpectedFake<int>, decltype(result)>);
        CHECK(1337 == *result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "gimo::or_else creates an appropriate pipeline.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<std::optional<int>() const> const inner{};
    auto action = [&] { return inner(); };
    using DummyAction = decltype(action);

    decltype(auto) pipeline = or_else(with_qualification::cast(action));
    STATIC_CHECK(std::same_as<Pipeline<detail::or_else_t<DummyAction>>, decltype(pipeline)>);
    STATIC_CHECK(gimo::applicable_on<std::optional<int>, detail::or_else_t<DummyAction>>);

    SCOPED_EXP inner.expect_call()
        and finally::returns(1337);
    CHECK(1337 == pipeline.apply(std::optional<int>{}));
}
