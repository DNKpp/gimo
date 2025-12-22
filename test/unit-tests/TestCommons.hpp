//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <tuple>
#include <type_traits>

#include <mimic++/mimic++.hpp>

#include "gimo/Common.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

namespace gimo::testing
{
    struct as_lvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T>&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    struct as_const_lvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T> const&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    struct as_rvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T>&&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T&& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    struct as_const_rvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T> const&&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T&& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    using with_qualification_list = std::tuple<
        as_lvalue_ref,
        as_const_lvalue_ref,
        as_rvalue_ref,
        as_const_rvalue_ref>;

    template <template <typename> typename ResultTemplate>
    struct BasicAlgorithmMockTraits
    {
        template <nullable Nullable>
        using output = ResultTemplate<std::remove_cvref_t<detail::value_result_t<Nullable>>>;

        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = true;

        template <typename Action, nullable Nullable, typename... Steps>
        inline static mimicpp::Mock<std::remove_cvref_t<output<Nullable>>(Action, Nullable, Steps...) const> on_value_{
            {"AlgorithmMock::on_value", 1u}
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return on_value_<Action&&, Nullable&&, Steps&&...>(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Action, nullable Nullable, typename... Steps>
        inline static mimicpp::Mock<std::remove_cvref_t<output<Nullable>>(Action, Nullable, Steps...) const> on_null_{
            {"AlgorithmMock::on_null", 1u}
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return on_null_<Action&&, Nullable&&, Steps&&...>(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }
    };

    using AlgorithmMockTraits = BasicAlgorithmMockTraits<std::optional>;

    template <typename Action>
    using AlgorithmMock = BasicAlgorithm<AlgorithmMockTraits, Action>;

    template <typename Value, typename Error = std::string>
    class ExpectedFake
    {
    public:
        using value_type = Value;
        using error_type = Error;

        struct null_t
        {
            [[nodiscard]]
            friend constexpr bool operator==(ExpectedFake const& expected, [[maybe_unused]] null_t const tag) noexcept
            {
                return !expected;
            }

            [[nodiscard]]
            explicit(false) constexpr operator ExpectedFake() const noexcept
            {
                return ExpectedFake::from_error({});
            }
        };

        [[nodiscard]]
        explicit ExpectedFake(Value value) noexcept
            : m_Value{std::move(value)}
        {
        }

        [[nodiscard]]
        static ExpectedFake from_error(error_type error) noexcept
        {
            ExpectedFake expected{};
            expected.m_Error = std::move(error);

            return expected;
        }

        [[nodiscard]]
        constexpr value_type const& operator*() const&
        {
            return m_Value.value();
        }

        [[nodiscard]]
        constexpr value_type&& operator*() &&
        {
            return std::move(m_Value).value();
        }

        [[nodiscard]]
        constexpr error_type& error() &
        {
            return m_Error.value();
        }

        [[nodiscard]]
        constexpr error_type const& error() const&
        {
            return m_Error.value();
        }

        [[nodiscard]]
        constexpr error_type&& error() &&
        {
            return std::move(m_Error).value();
        }

        [[nodiscard]]
        constexpr error_type const&& error() const&&
        {
            return std::move(m_Error).value();
        }

        [[nodiscard]]
        explicit constexpr operator bool() const noexcept
        {
            return m_Value.has_value();
        }

        [[nodiscard]]
        friend bool operator==(ExpectedFake const&, ExpectedFake const&) = default;

    private:
        std::optional<value_type> m_Value{};
        std::optional<error_type> m_Error{};

        [[nodiscard]]
        ExpectedFake() = default;
    };
}

template <typename Value, typename Error>
struct gimo::traits<gimo::testing::ExpectedFake<Value, Error>>
{
    using expected = testing::ExpectedFake<Value, Error>;

    static constexpr expected::null_t null{};

    template <typename V>
    using rebind_value = testing::ExpectedFake<V, Error>;

    template <typename E>
    using rebind_error = testing::ExpectedFake<Value, E>;

    static constexpr testing::ExpectedFake<Value, Error> from_error(Error error)
    {
        return testing::ExpectedFake<Value, Error>::from_error(std::move(error));
    }
};
