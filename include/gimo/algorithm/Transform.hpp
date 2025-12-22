//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_TRANSFORM_HPP
#define GIMO_ALGORITHM_TRANSFORM_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo::detail::transform
{
    template <typename Nullable, typename Action>
    consteval void print_diagnostics()
    {
        static_assert(
            std::is_invocable_v<Action, value_result_t<Nullable>>,
            "The transform algorithm requires an action invocable with the nullable’s value.");
        static_assert(
            rebindable_value_to<Nullable, std::invoke_result_t<Action, value_result_t<Nullable>>>,
            "The transform algorithm requires a nullable whose value-type can be rebound.");
    }

    template <typename Nullable, typename Action>
    using result_t = rebind_value_t<
        Nullable,
        std::invoke_result_t<Action, value_result_t<Nullable>>>;

    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr result_t<Nullable, Action> on_value([[maybe_unused]] Action&& action, Nullable&& opt)
    {
        return detail::construct_from_value<result_t<Nullable, Action>>(
            std::invoke(
                std::forward<Action>(action),
                detail::forward_value<Nullable>(opt)));
    }

    template <typename Action, nullable Nullable, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_value(
        [[maybe_unused]] Action&& action,
        Nullable&& opt,
        Next&& next,
        Steps&&... steps)
    {
        return std::forward<Next>(next).on_value(
            transform::on_value(std::forward<Action>(action), std::forward<Nullable>(opt)),
            std::forward<Steps>(steps)...);
    }

    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr result_t<Nullable, Action> on_null([[maybe_unused]] Action&& action, [[maybe_unused]] Nullable&& opt)
    {
        return detail::construct_empty<result_t<Nullable, Action>>();
    }

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr result_t<Expected, Action> on_null([[maybe_unused]] Action&& action, Expected&& expected)
    {
        return detail::rebind_error<result_t<Expected, Action>, Expected>(expected);
    }

    template <nullable Nullable, typename Action, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_null(Action&& action, Nullable&& opt, Next&& next, Steps&&... steps)
    {
        return std::forward<Next>(next).on_null(
            transform::on_null(std::forward<Action>(action), std::forward<Nullable>(opt)),
            std::forward<Steps>(steps)...);
    }

    struct traits
    {
        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires rebindable_value_to<
                Nullable,
                std::invoke_result_t<Action, value_result_t<Nullable>>>;
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            if constexpr (is_applicable_on<Nullable, Action>)
            {
                return transform::on_value(
                    std::forward<Action>(action),
                    std::forward<Nullable>(opt),
                    std::forward<Steps>(steps)...);
            }
            else
            {
                transform::print_diagnostics<Nullable, Action>();
                return std::forward<Nullable>(opt);
            }
        }

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            if constexpr (is_applicable_on<Nullable, Action>)
            {
                return transform::on_null(
                    std::forward<Action>(action),
                    std::forward<Nullable>(opt),
                    std::forward<Steps>(steps)...);
            }
            else
            {
                transform::print_diagnostics<Nullable, Action>();
                return std::forward<Nullable>(opt);
            }
        }
    };
}

namespace gimo
{
    namespace detail
    {
        template <typename Action>
        using transform_t = BasicAlgorithm<transform::traits, std::remove_cvref_t<Action>>;
    }

    template <typename Action>
    [[nodiscard]]
    constexpr auto transform(Action&& action)
    {
        using Algorithm = detail::transform_t<Action>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Action>(action)}};
    }
}

#endif
