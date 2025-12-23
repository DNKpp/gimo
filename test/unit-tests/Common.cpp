//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/Common.hpp"
#include "gimo_ext/StdOptional.hpp"

#include "TestCommons.hpp"

// see: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2445r1.pdf
TEMPLATE_TEST_CASE_SIG(
    "const_ref_like_t merges const and adapts the value-category of T.",
    "[trait]",
    ((typename Expected, typename T, typename U), Expected, T, U),
    (int&&, float, int),
    (int&, float&, int),
    (int&&, float&&, int),

    // 4
    (int const&&, float const, int),
    (int const&, float const&, int),
    (int const&&, float const&&, int),

    // 7
    (int const&&, float, int const),
    (int const&, float&, int const),
    (int const&&, float&&, int const),

    // 10
    (int const&&, float const, int const),
    (int const&, float const&, int const),
    (int const&&, float const&&, int const),

    // 13
    (int&, float&, int&),
    (int&, float&, int&&),

    // 15
    (int const&, float&, int const&),
    (int const&, float&, int const&&),

    // 17
    (int const&, float const&, int const&),
    (int const&, float const&, int const&&),

    // 19
    (int&&, float, int&),
    (int&&, float&&, int&),

    (int const&&, float const, int&),
    (int const&, float const&, int&),
    (int const&&, float const&&, int&),

    // 24
    (int&&, float, int&&),
    (int&&, float&&, int&&),

    (int const&&, float const, int&&),
    (int const&, float const&, int&&),
    (int const&&, float const&&, int&&),

    // 29
    (int const&&, float, int const&),
    (int const&&, float&&, int const&),
    (int const&&, float const, int const&),
    (int const&&, float const&&, int const&),

    // 33
    (int const&&, float, int const&&),
    (int const&&, float&&, int const&&),
    (int const&&, float const, int const&&),
    (int const&&, float const&&, int const&&))
{
    STATIC_CHECK(std::same_as<Expected, gimo::detail::const_ref_like_t<T, U>>);
    STATIC_CHECK(std::same_as<Expected, decltype(gimo::detail::forward_like<T>(std::declval<U>()))>);
}

TEMPLATE_TEST_CASE_SIG(
    "nullable determines whether the specified type satisfies the requirements.",
    "[concept]",
    ((bool expected, typename T), expected, T),
    (false, int),
    (false, std::nullopt_t),
    (true, std::optional<int>),
    (true, gimo::testing::ExpectedFake<int>))
{
    STATIC_CHECK(expected == gimo::nullable<T>);
    STATIC_CHECK(expected == gimo::nullable<T const>);
    STATIC_CHECK(expected == gimo::nullable<T&>);
    STATIC_CHECK(expected == gimo::nullable<T const&>);
    STATIC_CHECK(expected == gimo::nullable<T&&>);
    STATIC_CHECK(expected == gimo::nullable<T const&&>);
}

TEMPLATE_TEST_CASE_SIG(
    "expected_like determines whether the specified type satisfies the requirements.",
    "[concept]",
    ((bool expected, typename T), expected, T),
    (false, int),
    (false, std::nullopt_t),
    (false, std::optional<int>),
    (true, gimo::testing::ExpectedFake<int>))
{
    STATIC_CHECK(expected == gimo::expected_like<T>);
    STATIC_CHECK(expected == gimo::expected_like<T const>);
    STATIC_CHECK(expected == gimo::expected_like<T&>);
    STATIC_CHECK(expected == gimo::expected_like<T const&>);
    STATIC_CHECK(expected == gimo::expected_like<T&&>);
    STATIC_CHECK(expected == gimo::expected_like<T const&&>);
}

namespace dummy
{
    namespace
    {
        struct ADLTester
        {
        };

        [[nodiscard]]
        constexpr int value([[maybe_unused]] ADLTester const& opt)
        {
            return 1337;
        }

        [[nodiscard]]
        constexpr std::string_view error([[maybe_unused]] ADLTester const& opt)
        {
            return "Error";
        }
    }
}

TEST_CASE(
    "value customization point supports ADL.",
    "[customization-point]")
{
    constexpr dummy::ADLTester test{};

    STATIC_CHECK(1337 == gimo::detail::value(test));
}

TEST_CASE(
    "forward_error customization point supports ADL.",
    "[customization-point]")
{
    constexpr dummy::ADLTester test{};

    STATIC_CHECK("Error" == gimo::detail::error(test));
}

namespace dummy
{
    namespace
    {
        struct TraitCPTester
        {
            [[nodiscard]]
            constexpr int operator*() const
            {
                return 42;
            }

            [[nodiscard]]
            constexpr std::string_view error() const
            {
                return "Member error.";
            }
        };
    }
}

template <>
struct gimo::traits<dummy::TraitCPTester>
{
    static constexpr int value([[maybe_unused]] dummy::TraitCPTester const& obj)
    {
        return 1337;
    }

    static constexpr std::string_view error([[maybe_unused]] dummy::TraitCPTester const& obj)
    {
        return "Error";
    }
};

TEST_CASE(
    "value customization point supports trait definition.",
    "[customization-point]")
{
    constexpr dummy::TraitCPTester test{};

    STATIC_CHECK(1337 == gimo::detail::value(test));
}

TEST_CASE(
    "error customization point supports trait definition.",
    "[customization-point]")
{
    constexpr dummy::TraitCPTester test{};

    STATIC_CHECK("Error" == gimo::detail::error(test));
}

namespace
{
    struct DirectlyConstructibleNullable
    {
        struct null_t
        {
            [[nodiscard, maybe_unused]]
            friend constexpr bool operator==([[maybe_unused]] DirectlyConstructibleNullable const& nullable, [[maybe_unused]] null_t const tag) noexcept
            {
                return true;
            }

            [[nodiscard, maybe_unused]]
            explicit(false) constexpr operator DirectlyConstructibleNullable() const noexcept
            {
                return DirectlyConstructibleNullable{-1};
            }
        };

        explicit constexpr DirectlyConstructibleNullable(int const value)
            : value{value}
        {
        }

        int value;

        [[nodiscard]]
        constexpr int const& operator*() const noexcept
        {
            return value;
        }
    };

}

template <>
struct gimo::traits<DirectlyConstructibleNullable>
{
    static constexpr DirectlyConstructibleNullable::null_t null{};
};

static_assert(gimo::nullable<DirectlyConstructibleNullable>);

TEST_CASE(
    "construct_from_value customization point supports direct initialization.",
    "[customization-point]")
{
    STATIC_CHECK(gimo::constructible_from_value<DirectlyConstructibleNullable, int>);
    constexpr auto obj = gimo::construct_from_value<DirectlyConstructibleNullable>(42);

    STATIC_CHECK(42 == *obj);
}

namespace
{
    struct IndirectlyConstructibleNullable
    {
        struct null_t
        {
            [[nodiscard, maybe_unused]]
            friend constexpr bool operator==([[maybe_unused]] IndirectlyConstructibleNullable const& nullable, [[maybe_unused]] null_t const tag) noexcept
            {
                return true;
            }

            [[nodiscard, maybe_unused]]
            explicit(false) constexpr operator IndirectlyConstructibleNullable() const noexcept
            {
                return Make(-1);
            }
        };

        static constexpr IndirectlyConstructibleNullable Make([[maybe_unused]] int const value)
        {
            IndirectlyConstructibleNullable obj{};
            obj.m_Value = value;

            return obj;
        }

        [[nodiscard]]
        constexpr int const& operator*() const noexcept
        {
            return m_Value;
        }

    private:
        int m_Value;

        IndirectlyConstructibleNullable() = default;
    };

}

template <>
struct gimo::traits<IndirectlyConstructibleNullable>
{
    static constexpr IndirectlyConstructibleNullable::null_t null{};

    [[nodiscard]]
    static constexpr IndirectlyConstructibleNullable from_value(int const value)
    {
        return IndirectlyConstructibleNullable::Make(value);
    }
};

static_assert(gimo::nullable<IndirectlyConstructibleNullable>);

TEST_CASE(
    "construct_from_value customization point supports trait definition.",
    "[customization-point]")
{
    STATIC_CHECK(gimo::constructible_from_value<IndirectlyConstructibleNullable, int>);
    constexpr auto obj = gimo::construct_from_value<IndirectlyConstructibleNullable>(42);

    STATIC_CHECK(42 == *obj);
}
