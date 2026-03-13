//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_VALUE_OR_ELSE_HPP
#define GIMO_ALGORITHM_VALUE_OR_ELSE_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <concepts>
#include <functional>
#include <tuple>
#include <utility>

namespace gimo::detail::value_or_else
{
    template <typename Nullable>
    using result_t = std::remove_cvref_t<value_result_t<Nullable>>;

    template <typename Nullable, typename Action>
    consteval Nullable* print_diagnostics()
    {
        if constexpr (!std::convertible_to<std::invoke_result_t<Action>, result_t<Nullable>>)
        {
            static_assert(always_false_v<Nullable>, "The value_or algorithm requires an alternative that is convertible to the nullable's value-type.");
        }

        return nullptr;
    }

    struct traits
    {
        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires std::convertible_to<
                std::invoke_result_t<Action>,
                result_t<Nullable>>;
        };

        template <typename Action, nullable Nullable>
        [[nodiscard]]
        static constexpr auto on_value([[maybe_unused]] Action&& action, Nullable&& opt)
        {
            if constexpr (is_applicable_on<Nullable, Action>)
            {
                return detail::forward_value<Nullable>(opt);
            }
            else
            {
                return *value_or_else::print_diagnostics<Nullable, Action>();
            }
        }

        template <typename Action, nullable Nullable>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, [[maybe_unused]] Nullable&& opt)
        {
            if constexpr (is_applicable_on<Nullable, Action>)
            {
                return static_cast<result_t<Nullable>>(
                    std::invoke(std::forward<Action>(action)));
            }
            else
            {
                return *value_or_else::print_diagnostics<Nullable, Action>();
            }
        }
    };
}

namespace gimo
{
    namespace detail
    {
        template <typename Action>
        using value_or_else_t = BasicAlgorithm<
            value_or_else::traits,
            std::remove_cvref_t<Action>>;

        template <unqualified T>
        class ValueStorageFun
        {
        public:
            [[nodiscard]]
            explicit constexpr ValueStorageFun(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
                : m_value{std::move(value)}
            {
            }

            [[nodiscard]]
            constexpr T& operator()() & noexcept
            {
                return m_value;
            }

            [[nodiscard]]
            constexpr T const& operator()() const& noexcept
            {
                return m_value;
            }

            [[nodiscard]]
            constexpr T&& operator()() && noexcept
            {
                return std::move(m_value);
            }

            [[nodiscard]]
            constexpr T const&& operator()() const&& noexcept
            {
                return std::move(m_value);
            }

        private:
            T m_value;
        };

        template <typename Alternative>
        using value_or_t = value_or_else_t<
            ValueStorageFun<std::decay_t<Alternative>>>;
    }

    /**
     * \brief Creates a terminating pipeline step that returns the contained value or invokes a fallback action if the nullable is null.
     * \ingroup ALGORITHM
     * \tparam Action The fallback action type.
     * \param action A nullary operation that produces an alternative value.
     * \return A Pipeline step containing the `value_or_else` algorithm.
     * \details
     * - **On Value**: Forwards the underlying value of the input.
     * - **On Null**: Invokes `action` and returns its result converted to the nullable's value type.
     *
     * The action is invoked only if the input nullable does not contain a value.
     * The return type of `action` must be convertible to the nullable's value type.
     */
    template <std::invocable Action>
    [[nodiscard]]
    constexpr auto value_or_else(Action&& action)
    {
        using Algorithm = detail::value_or_else_t<Action>;

        return Pipeline{
            std::tuple<Algorithm>{std::forward<Action>(action)}};
    }

    /**
     * \brief Creates a terminating pipeline step that returns the contained value or a specified alternative if the nullable is null.
     * \ingroup ALGORITHM
     * \tparam Alternative The alternative value type.
     * \param alternative The alternative value to use if the nullable is null.
     * \return A Pipeline step containing the `value_or` algorithm.
     * \details
     * - **On Value**: Forwards the underlying value of the input.
     * - **On Null**: Returns the specified alternative converted to the nullable's value type.
     *
     * `Alternative` must be convertible to the nullable's value type.
     */
    template <typename Alternative>
    [[nodiscard]]
    constexpr auto value_or(Alternative&& alternative)
    {
        using Algorithm = detail::value_or_t<Alternative>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Alternative>(alternative)}};
    }
}

#endif
