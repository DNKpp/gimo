//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/ValueOr.hpp"
#include "gimo_ext/StdOptional.hpp"

#include "TestCommons.hpp"

using namespace gimo;

TEMPLATE_LIST_TEST_CASE(
    "value_or algorithm extracts the contained value, when the input is not empty.",
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
    "value_or algorithm accepts nullables with any cv-ref qualification.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    using Algorithm = detail::value_or_t<float>;
    STATIC_REQUIRE(gimo::applicable_to<std::optional<float>, typename with_qualification::template type<Algorithm>>);

    constexpr Algorithm algorithm{42.f};
    std::optional opt{1337.f};

    decltype(auto) result = algorithm(with_qualification::cast(opt));
    STATIC_REQUIRE(std::same_as<float, decltype(result)>);
    CHECK(1337.f == result);
}

TEMPLATE_LIST_TEST_CASE(
    "value_or algorithm supports expected_like types.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using with_qualification = TestType;

    using Algorithm = detail::value_or_t<float>;
    STATIC_REQUIRE(gimo::applicable_to<testing::ExpectedFake<float>, typename with_qualification::template type<Algorithm>>);

    SECTION("When input has a value, the contained value is forwarded.")
    {
        testing::ExpectedFake const expected{1337.f};

        Algorithm algorithm{42.f};
        decltype(auto) result = std::invoke(with_qualification::cast(algorithm), expected);
        STATIC_REQUIRE(std::same_as<float, decltype(result)>);
        CHECK(1337.f == result);
    }

    SECTION("When input is empty, the alternative is forwarded.")
    {
        auto const expected = testing::ExpectedFake<float>::from_error("An error.");
        Algorithm algorithm{42.f};

        decltype(auto) result = std::invoke(with_qualification::cast(algorithm), expected);
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
