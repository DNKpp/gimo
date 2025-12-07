//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_AND_THEN_HPP
#define GIMO_ALGORITHM_AND_THEN_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo::detail::transform
{
    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr auto on_value([[maybe_unused]] Action&& action, Nullable&& opt)
    {
        using Result = rebind_value_t<
            Nullable,
            std::invoke_result_t<Action, value_result_t<Nullable>>>;

        return Result{
            std::invoke(
                std::forward<Action>(action),
                value(std::forward<Nullable>(opt)))};
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
    constexpr auto on_null([[maybe_unused]] Action&& action, [[maybe_unused]] Nullable&& opt)
    {
        using Result = rebind_value_t<
            Nullable,
            std::invoke_result_t<Action, value_result_t<Nullable>>>;

        return detail::construct_empty<Result>();
    }

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr auto on_null([[maybe_unused]] Action&& action, Expected&& expected)
    {
        using Result = rebind_value_t<
            Expected,
            std::invoke_result_t<Action, value_result_t<Expected>>>;

        return detail::rebind_error<Result, Expected>(expected);
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
            requires adaptable_value_by<
                std::invoke_result_t<Action, value_result_t<Nullable>>,
                std::remove_cvref_t<Nullable>>;
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return transform::on_value(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return transform::on_null(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
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
