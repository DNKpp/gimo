//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_TRANSFORM_ERROR_HPP
#define GIMO_ALGORITHM_TRANSFORM_ERROR_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo::detail::transform_error
{
    template <typename Expected, typename Action>
    using result_t = rebind_error_t<
        Expected,
        std::invoke_result_t<Action, error_result_t<Expected>>>;

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr auto on_value([[maybe_unused]] Action&& action, Expected&& closure)
    {
        return result_t<Expected, Action>{
            value(std::forward<Expected>(closure))};
    }

    template <typename Action, expected_like Expected, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_value(
        Action&& action,
        Expected&& closure,
        Next&& next,
        Steps&&... steps)
    {
        return std::forward<Next>(next).on_value(
            transform_error::on_value(std::forward<Action>(action), std::forward<Expected>(closure)),
            std::forward<Steps>(steps)...);
    }

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr auto on_null(Action&& action, Expected&& closure)
    {
        return traits<result_t<Expected, Action>>::bind_error(
            std::invoke(
                std::forward<Action>(action),
                error(std::forward<Expected>(closure))));
    }

    template <typename Action, expected_like Expected, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_null(Action&& action, Expected&& closure, Next&& next, Steps&&... steps)
    {
        return std::forward<Next>(next).on_null(
            transform_error::on_null(std::forward<Action>(action), std::forward<Expected>(closure)),
            std::forward<Steps>(steps)...);
    }

    struct traits
    {
        template <expected_like Expected, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires rebindable_error_to<
                Expected,
                std::invoke_result_t<Action, error_result_t<Expected>>>;
        };

        template <typename Action, expected_like Expected, typename... Steps>
        [[nodiscard]]
        static constexpr expected_like auto on_value(Action&& action, Expected&& closure, Steps&&... steps)
        {
            return transform_error::on_value(
                std::forward<Action>(action),
                std::forward<Expected>(closure),
                std::forward<Steps>(steps)...);
        }

        template <typename Action, expected_like Expected, typename... Steps>
        [[nodiscard]]
        static constexpr expected_like auto on_null(Action&& action, Expected&& closure, Steps&&... steps)
        {
            return transform_error::on_null(
                std::forward<Action>(action),
                std::forward<Expected>(closure),
                std::forward<Steps>(steps)...);
        }
    };
}

namespace gimo
{
    namespace detail
    {
        template <typename Action>
        using transform_error_t = BasicAlgorithm<transform_error::traits, std::remove_cvref_t<Action>>;
    }

    template <typename Action>
    [[nodiscard]]
    constexpr auto transform_error(Action&& action)
    {
        using Algorithm = detail::transform_error_t<Action>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Action>(action)}};
    }
}

#endif
