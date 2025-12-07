//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_OR_ELSE_HPP
#define GIMO_ALGORITHM_OR_ELSE_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo::detail::or_else
{
    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr auto on_value([[maybe_unused]] Action&& action, Nullable&& opt)
    {
        return std::forward<Nullable>(opt);
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
            std::forward<Nullable>(opt),
            std::forward<Steps>(steps)...);
    }

    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr auto on_null(Action&& action, [[maybe_unused]] Nullable&& opt)
    {
        return std::invoke(std::forward<Action>(action));
    }

    template <nullable Nullable, typename Action, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_null(Action&& action, Nullable&& opt, Next&& next, Steps&&... steps)
    {
        return std::invoke(
            std::forward<Next>(next),
            or_else::on_null(std::forward<Action>(action), std::forward<Nullable>(opt)),
            std::forward<Steps>(steps)...);
    }

    struct traits
    {
        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires std::same_as<
                std::remove_cvref_t<Nullable>,
                std::remove_cvref_t<std::invoke_result_t<Action>>>;
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return or_else::on_value(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return or_else::on_null(
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
        using or_else_t = BasicAlgorithm<or_else::traits, std::remove_cvref_t<Action>>;
    }

    template <typename Action>
    [[nodiscard]]
    constexpr auto or_else(Action&& action)
    {
        using Algorithm = detail::or_else_t<Action>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Action>(action)}};
    }
}

#endif
