//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/AndForward.hpp"
#include "gimo_ext/StdOptional.hpp"

#include "TestCommons.hpp"

using namespace gimo;

TEMPLATE_LIST_TEST_CASE(
    "and_forward algorithm invokes its action only when the input contains a value.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        void(int) &,
        void(int) const&,
        void(int) &&,
        void(int) const&&>
        action{};

    using Algorithm = detail::and_forward_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_to<std::optional<int>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input contains a value, the action is invoked.")
    {
        constexpr std::optional opt{1337};

        SCOPED_EXP with_qualification::cast(action).expect_call(1337);

        Algorithm algorithm{std::move(action)};
        STATIC_REQUIRE(std::same_as<void, decltype(with_qualification::cast(algorithm)(opt))>);

        with_qualification::cast(algorithm)(opt);
    }

    SECTION("When input contains no value, the pipeline silently terminates.")
    {
        constexpr std::optional<int> opt{};
        Algorithm algorithm{std::move(action)};
        STATIC_REQUIRE(std::same_as<void, decltype(with_qualification::cast(algorithm)(opt))>);

        with_qualification::cast(algorithm)(opt);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "and_forward algorithm accepts nullables with any cv-ref qualification.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        void(int&) const,
        void(int const&) const,
        void(int&&) const,
        void(int const&&) const> const action{};

    using Algorithm = detail::and_forward_t<decltype(std::cref(action))>;
    STATIC_REQUIRE(gimo::applicable_to<std::optional<int>, typename with_qualification::template type<Algorithm>>);

    Algorithm const algorithm{std::cref(action)};
    std::optional opt{42};

    SCOPED_EXP action.expect_call(matches::type<typename with_qualification::template type<int>>)
        and expect::arg<0>(matches::eq(42));
    algorithm(with_qualification::cast(opt));
}

TEMPLATE_LIST_TEST_CASE(
    "and_forward algorithm supports expected_like types.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        void(int) &,
        void(int) const&,
        void(int) &&,
        void(int) const&&>
        action{};

    using Algorithm = detail::and_forward_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_to<std::optional<int>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input contains a value, the action is invoked.")
    {
        testing::ExpectedFake const expected{1337};

        SCOPED_EXP with_qualification::cast(action).expect_call(1337);

        Algorithm algorithm{std::move(action)};
        STATIC_REQUIRE(std::same_as<void, decltype(with_qualification::cast(algorithm)(expected))>);

        with_qualification::cast(algorithm)(expected);
    }

    SECTION("When input contains no value, the pipeline silently terminates.")
    {
        auto const expected = testing::ExpectedFake<int>::from_error("An error.");

        Algorithm algorithm{std::move(action)};
        STATIC_REQUIRE(std::same_as<void, decltype(with_qualification::cast(algorithm)(expected))>);

        with_qualification::cast(algorithm)(expected);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "gimo::and_forward creates an appropriate pipeline.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<void(int) const> const inner{};
    auto action = [&](int const x) { return inner(x); };
    using DummyAction = decltype(action);

    decltype(auto) pipeline = and_forward(with_qualification::cast(action));
    STATIC_CHECK(std::same_as<Pipeline<detail::and_forward_t<DummyAction>>, decltype(pipeline)>);
    STATIC_CHECK(gimo::applicable_to<std::optional<int>, detail::and_forward_t<DummyAction>>);
    STATIC_CHECK(gimo::processable_by<std::optional<int>, decltype(pipeline)>);

    SCOPED_EXP inner.expect_call(42);
    pipeline.apply(std::optional{42});
}
