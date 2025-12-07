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

namespace gimo::detail::and_then
{
    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr auto on_value(Action&& action, Nullable&& opt)
    {
        return std::invoke(
            std::forward<Action>(action),
            gimo::value(std::forward<Nullable>(opt)));
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
    constexpr auto on_null([[maybe_unused]] Action&& action, [[maybe_unused]] Nullable&& opt)
    {
        using Result = std::invoke_result_t<Action, reference_type_t<Nullable>>;

        return detail::construct_empty<Result>();
    }

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr auto on_null([[maybe_unused]] Action&& action, [[maybe_unused]] Expected&& expected)
    {
        using Result = std::invoke_result_t<Action, reference_type_t<Expected>>;

        return detail::rebind_error<Result>(
            forward_error<Expected>(expected));
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
            requires nullable<
                std::invoke_result_t<
                    Action,
                    reference_type_t<Nullable>>>;
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return and_then::on_value(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return and_then::on_null(
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
