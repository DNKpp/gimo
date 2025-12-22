//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_AND_THEN_HPP
#define GIMO_ALGORITHM_AND_THEN_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo::detail::and_then
{
    template <typename Nullable, typename Action>
    consteval Nullable* print_diagnostics()
    {
        if constexpr (!std::is_invocable_v<Action, value_result_t<Nullable>>)
        {
            static_assert(false, "The and_then algorithm requires an action invocable with the nullable’s value.");
        }
        else if constexpr (!nullable<std::invoke_result_t<Action, value_result_t<Nullable>>>)
        {
            static_assert(false, "The and_then algorithm requires an action with a nullable return-type.");
        }

        return nullptr;
    }

    template <typename Nullable, typename Action>
    using result_t = std::invoke_result_t<
        Action,
        value_result_t<Nullable>>;

    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr result_t<Nullable, Action> on_value(Action&& action, Nullable&& opt)
    {
        return std::invoke(
            std::forward<Action>(action),
            detail::forward_value<Nullable>(opt));
    }

    template <typename Action, nullable Nullable, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_value(
        Action&& action,
        Nullable&& opt,
        Next&& next,
        Steps&&... steps)
    {
        return std::invoke(
            std::forward<Next>(next),
            and_then::on_value(std::forward<Action>(action), std::forward<Nullable>(opt)),
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

    template <typename Action, nullable Nullable, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_null(Action&& action, Nullable&& opt, Next&& next, Steps&&... steps)
    {
        return std::forward<Next>(next).on_null(
            and_then::on_null(std::forward<Action>(action), std::forward<Nullable>(opt)),
            std::forward<Steps>(steps)...);
    }

    struct traits
    {
        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires nullable<result_t<Nullable, Action>>;
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            if constexpr (is_applicable_on<Nullable, Action>)
            {
                return and_then::on_value(
                    std::forward<Action>(action),
                    std::forward<Nullable>(opt),
                    std::forward<Steps>(steps)...);
            }
            else
            {
                return *and_then::print_diagnostics<Nullable, Action>();
            }
        }

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            if constexpr (is_applicable_on<Nullable, Action>)
            {
                return and_then::on_null(
                    std::forward<Action>(action),
                    std::forward<Nullable>(opt),
                    std::forward<Steps>(steps)...);
            }
            else
            {
                return *and_then::print_diagnostics<Nullable, Action>();
            }
        }
    };
}

namespace gimo
{
    namespace detail
    {
        template <typename Action>
        using and_then_t = BasicAlgorithm<and_then::traits, std::remove_cvref_t<Action>>;
    }

    template <typename Action>
    [[nodiscard]]
    constexpr auto and_then(Action&& action)
    {
        using Algorithm = detail::and_then_t<Action>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Action>(action)}};
    }
}

#endif
