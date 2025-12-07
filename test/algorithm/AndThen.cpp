//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/AndThen.hpp"
#include "gimo_ext/std_optional.hpp"

#include "TestCommons.hpp"

using namespace gimo;

namespace
{

    /*
    template <typename Alternative>
struct value_or_algorithm
{
    template <typename Nullable>
    [[nodiscard]]
    constexpr auto operator()(Nullable&& opt)
    {
        if (detail::has_value(opt))
        {
            return std::invoke(*this, has_value_tag{}, std::forward<Nullable>());
        }

        return std::invoke(*this, is_empty_tag{}, std::forward<Nullable>());
    }

    template <typename Nullable>
    [[nodiscard]]
    constexpr auto operator()(detail::has_value_tag const, Nullable&& opt)
    {
        return value(std::forward<Nullable>(opt));
    }

    template <typename Nullable>
    [[nodiscard]]
    constexpr auto operator()(is_empty_tag const, [[maybe_unused]] Nullable&& opt)
    {
        return alternative;
    }

    Alternative alternative;
};*/
}

TEMPLATE_LIST_TEST_CASE(
    "and_then algorithm invokes its action only when the input has a value.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        std::optional<int>(float)&,
        std::optional<int>(float) const&,
        std::optional<int>(float)&&,
        std::optional<int>(float) const&&>
        action{};

    using Algorithm = detail::and_then_t<decltype(action)>;
    STATIC_REQUIRE(gimo::applicable_on<std::optional<float>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input has a value, the action is invoked.")
    {
        constexpr std::optional opt{1337.f};

        SCOPED_EXP with_qualification::cast(action).expect_call(1337.f)
            and finally::returns(std::optional{42});

        Algorithm andThen{std::move(action)};
        decltype(auto) result = with_qualification::cast(andThen)(opt);
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(42 == result);
    }

    SECTION("When input is empty, action is not invoked.")
    {
        constexpr std::optional<float> opt{};
        Algorithm andThen{std::move(action)};

        decltype(auto) result = with_qualification::cast(andThen)(opt);
        STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
        CHECK(!result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "and_then algorithm accepts nullables with any cv-ref qualification.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<
        std::optional<int>(float&) const,
        std::optional<int>(float const&) const,
        std::optional<int>(float&&) const,
        std::optional<int>(float const&&) const>
        action{};
    using Algorithm = detail::and_then_t<decltype(std::cref(action))>;
    STATIC_REQUIRE(gimo::applicable_on<std::optional<float>, typename with_qualification::template type<Algorithm>>);

    Algorithm const andThen{std::cref(action)};
    std::optional opt{1337.f};

    using ExpectedRef = with_qualification::template type<float>;
    SCOPED_EXP action.expect_call(matches::type<ExpectedRef>)
        and expect::arg<0>(matches::eq(1337.f))
        and finally::returns(std::optional{42});
    decltype(auto) result = andThen(with_qualification::cast(opt));
    STATIC_REQUIRE(std::same_as<std::optional<int>, decltype(result)>);
    CHECK(42 == result);
}

TEMPLATE_LIST_TEST_CASE(
    "and_then algorithm forwards additional steps as-is.",
    "[algorith]",
    testing::with_qualification_list)
{
    using matches::_;
    using with_qualification = TestType;

    auto const step0 = detail::and_then_t<decltype([](int const v) { return std::optional{v + 1}; })>{};
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
    "gimo::and_then creates an appropriate pipeline.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    mimicpp::Mock<std::optional<float>(int) const> const inner{};
    auto action = [&](int const v) { return inner(v); };
    using DummyAction = decltype(action);

    decltype(auto) pipeline = and_then(with_qualification::cast(action));
    STATIC_CHECK(std::same_as<Pipeline<detail::and_then_t<DummyAction>>, decltype(pipeline)>);
    STATIC_CHECK(gimo::applicable_on<std::optional<int>, detail::and_then_t<DummyAction>>);

    SCOPED_EXP inner.expect_call(1337)
        and finally::returns(4.2f);
    CHECK(4.2f == pipeline.apply(std::optional<int>{1337}));
}
