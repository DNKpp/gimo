//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_VALUE_OR_HPP
#define GIMO_ALGORITHM_VALUE_OR_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <concepts>
#include <functional>
#include <utility>

namespace gimo::detail::value_or
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
                return *value_or::print_diagnostics<Nullable, Action>();
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
                return *value_or::print_diagnostics<Nullable, Action>();
            }
        }
    };
}

namespace gimo
{
    namespace detail
    {
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
        using value_or_t = BasicAlgorithm<
            value_or::traits,
            ValueStorageFun<std::decay_t<Alternative>>>;
    }

    template <typename Alternative>
    [[nodiscard]]
    constexpr auto value_or(Alternative&& alternative)
    {
        using Algorithm = detail::value_or_t<Alternative>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Alternative>(alternative)}};
    }
}

#endif
