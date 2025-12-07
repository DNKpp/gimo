//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/Transform.hpp"
#include "gimo_ext/std_optional.hpp"

#include "TestCommons.hpp"

using namespace gimo;

TEMPLATE_LIST_TEST_CASE(
    "transform algorithm invokes its action with the contained value, when there is any.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        int(float)&,
        int(float) const&,
        int(float)&&,
        int(float) const&&>
        action{};

    using Algorithm = detail::transform_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_on<std::optional<float>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input has a value, the action is invoked.")
    {
        SCOPED_EXP with_qualification::cast(action).expect_call(1337.f)
            and finally::returns(42);

        constexpr std::optional opt{1337.f};
        Algorithm transform{std::move(action)};

        decltype(auto) result = with_qualification::cast(transform)(opt);
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(42 == result);
    }

    SECTION("When input is empty, action is not invoked.")
    {
        Algorithm transform{std::move(action)};
        constexpr std::optional<float> opt{};

        decltype(auto) result = with_qualification::cast(transform)(opt);
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(!result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "transform algorithm accepts nullables with any cv-ref qualification.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        int(float&) const,
        int(float const&) const,
        int(float&&) const,
        int(float const&&) const> const action{};
    using Algorithm = detail::transform_t<decltype(std::cref(action))>;
    STATIC_REQUIRE(gimo::applicable_on<std::optional<float>, typename with_qualification::template type<Algorithm>>);

    Algorithm const andThen{std::cref(action)};
    std::optional opt{1337.f};

    using ExpectedRef = with_qualification::template type<float>;
    SCOPED_EXP action.expect_call(matches::type<ExpectedRef>)
        and expect::arg<0>(matches::eq(1337.f))
        and finally::returns(42);
    decltype(auto) result = andThen(with_qualification::cast(opt));
    STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
    CHECK(42 == result);
}

TEMPLATE_LIST_TEST_CASE(
    "transform algorithm forwards additional steps as-is.",
    "[algorith]",
    testing::with_qualification_list)
{
    using matches::_;
    using with_qualification = TestType;

    auto const step0 = detail::transform_t<decltype([](int const v) { return v + 1; })>{};
    using StepMock = testing::AlgorithmMock<std::identity>;
    using StepRef = with_qualification::template type<StepMock>;
    using ActionRef = with_qualification::template type<std::identity>;
    StepMock step1{};
    StepMock step2{};

    SECTION("When input has a value.")
    {
        auto& on_value = testing::AlgorithmMockTraits::on_value_<ActionRef, std::optional<int>&&, StepRef>;
        SCOPED_EXP on_value.expect_call(_, 43, matches::instance(step2))
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
        auto& on_null = testing::AlgorithmMockTraits::on_null_<ActionRef, std::optional<int>&&, StepRef>;
        SCOPED_EXP on_null.expect_call(_, std::nullopt, matches::instance(step2))
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
    "gimo::transform creates an appropriate pipeline.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<float(int) const> const inner{};
    auto action = [&](int const v) { return inner(v); };
    using DummyAction = decltype(action);

    decltype(auto) pipeline = transform(with_qualification::cast(action));
    STATIC_CHECK(std::same_as<Pipeline<detail::transform_t<DummyAction>>, decltype(pipeline)>);
    STATIC_CHECK(gimo::applicable_on<std::optional<int>, detail::transform_t<DummyAction>>);

    SCOPED_EXP inner.expect_call(1337)
        and finally::returns(4.2f);
    CHECK(std::optional{4.2f} == pipeline.apply(std::optional<int>{1337}));
}
