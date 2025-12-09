//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/TransformError.hpp"

#include "TestCommons.hpp"

using namespace gimo;

TEMPLATE_LIST_TEST_CASE(
    "transform_error algorithm invokes its action with the error, when there is any.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;
    using testing::ExpectedFake;

    mimicpp::Mock<
        int(std::string) &,
        int(std::string) const&,
        int(std::string) &&,
        int(std::string) const&&>
        action{};

    using Algorithm = detail::transform_error_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_on<ExpectedFake<float>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input has a value, the action is not invoked.")
    {
        ExpectedFake const expected{1337.f};
        Algorithm algorithm{std::move(action)};

        decltype(auto) result = with_qualification::cast(algorithm)(expected);
        STATIC_REQUIRE(std::same_as<ExpectedFake<float, int>, decltype(result)>);
        CHECK(1337.f == *result);
    }

    SECTION("When input holds an error, the action is invoked.")
    {
        SCOPED_EXP with_qualification::cast(action).expect_call("An error")
            and finally::returns(42);

        Algorithm algorithm{std::move(action)};
        auto expected = ExpectedFake<float>::from_error("An error");

        decltype(auto) result = with_qualification::cast(algorithm)(expected);
        STATIC_REQUIRE(std::same_as<ExpectedFake<float, int>, decltype(result)>);
        CHECK(42 == result.error());
    }
}

TEMPLATE_LIST_TEST_CASE(
    "transform_error algorithm accepts expected-like types with any cv-ref qualification.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;
    using testing::ExpectedFake;

    mimicpp::Mock<
        int(std::string&) const,
        int(std::string const&) const,
        int(std::string&&) const,
        int(std::string const&&) const> const action{};
    using Algorithm = detail::transform_error_t<decltype(std::cref(action))>;
    STATIC_REQUIRE(gimo::applicable_on<ExpectedFake<float>, typename with_qualification::template type<Algorithm>>);

    Algorithm const algorithm{std::cref(action)};
    auto expected = ExpectedFake<float>::from_error("An error");

    using ExpectedErrorRef = with_qualification::template type<std::string>;
    SCOPED_EXP action.expect_call(matches::type<ExpectedErrorRef>)
        and expect::arg<0>(matches::str::eq("An error"))
        and finally::returns(42);
    decltype(auto) result = algorithm(with_qualification::cast(expected));
    STATIC_REQUIRE(std::same_as<ExpectedFake<float, int>, decltype(result)>);
    CHECK(42 == result.error());
}

namespace
{
    // This is required to please clang with a version <= 18
    template <typename Value>
    using ExpectedFakeStr = testing::ExpectedFake<Value, std::string>;
}

TEMPLATE_LIST_TEST_CASE(
    "transform_error algorithm forwards additional steps as-is.",
    "[algorith]",
    testing::with_qualification_list)
{
    using matches::_;
    using with_qualification = TestType;
    using testing::ExpectedFake;
    using AlgorithmTraits = testing::BasicAlgorithmMockTraits<ExpectedFakeStr>;

    auto const step0 = detail::transform_error_t<decltype([](std::string const& e) { return e + " error"; })>{};
    using StepMock = BasicAlgorithm<AlgorithmTraits, std::identity>;
    using StepRef = with_qualification::template type<StepMock>;
    using ActionRef = with_qualification::template type<std::identity>;
    StepMock step1{};
    StepMock step2{};

    SECTION("When input contains a value.")
    {
        auto& on_value = AlgorithmTraits::on_value_<ActionRef, ExpectedFake<float>&&, StepRef>;
        SCOPED_EXP on_value.expect_call(_, ExpectedFake{4.2f}, matches::instance(step2))
            and finally::returns(1337.f);

        decltype(auto) result = std::invoke(
            step0,
            ExpectedFake{4.2f},
            with_qualification::cast(step1),
            with_qualification::cast(step2));
        STATIC_CHECK(std::same_as<ExpectedFake<float>, decltype(result)>);
        CHECK(1337.f == *result);
    }

    SECTION("When input holds an error.")
    {
        auto& on_null = AlgorithmTraits::on_null_<ActionRef, ExpectedFake<float>&&, StepRef>;
        SCOPED_EXP on_null.expect_call(_, ExpectedFake<float>::from_error("An error"), matches::instance(step2))
            and finally::returns_arg<1u>();

        decltype(auto) result = std::invoke(
            step0,
            ExpectedFake<float>::from_error("An"),
            with_qualification::cast(step1),
            with_qualification::cast(step2));
        STATIC_REQUIRE(std::same_as<ExpectedFake<float>, decltype(result)>);
        CHECK_THAT(
            result.error(),
            Catch::Matchers::Equals("An error"));
    }
}

TEMPLATE_LIST_TEST_CASE(
    "gimo::transform_error creates an appropriate pipeline.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;
    using testing::ExpectedFake;

    mimicpp::Mock<int(std::string const&) const> const inner{};
    auto action = [&](std::string const& e) { return inner(e); };
    using DummyAction = decltype(action);

    decltype(auto) pipeline = transform_error(with_qualification::cast(action));
    STATIC_CHECK(std::same_as<Pipeline<detail::transform_error_t<DummyAction>>, decltype(pipeline)>);
    STATIC_CHECK(gimo::applicable_on<ExpectedFake<float>, detail::transform_error_t<DummyAction>>);

    SCOPED_EXP inner.expect_call("An error")
        and finally::returns(42);
    CHECK(42 == pipeline.apply(ExpectedFake<float>::from_error("An error")).error());
}
