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
    consteval Expected* print_diagnostics()
    {
        if constexpr (!expected_like<Expected>)
        {
            static_assert(always_false_v<Expected>, "The transform_error algorithm requires an expected-like input.");
        }
        else if constexpr (!std::is_invocable_v<Action, error_result_t<Expected>>)
        {
            static_assert(always_false_v<Expected>, "The transform_error algorithm requires an action invocable with the expected's error.");
        }
        else if constexpr (!rebindable_error_to<Expected, std::invoke_result_t<Action, error_result_t<Expected>>>)
        {
            static_assert(always_false_v<Expected>, "The transform_error algorithm requires an expected-like whose error-type can be rebound.");
        }

        return nullptr;
    }

    template <typename Expected, typename Action>
    using result_t = rebind_error_t<
        Expected,
        std::invoke_result_t<Action, error_result_t<Expected>>>;

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr result_t<Expected, Action> on_value([[maybe_unused]] Action&& action, Expected&& closure)
    {
        return detail::rebind_value<result_t<Expected, Action>, Expected>(closure);
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
    constexpr result_t<Expected, Action> on_null(Action&& action, Expected&& closure)
    {
        return detail::construct_from_error<result_t<Expected, Action>>(
            std::invoke(
                std::forward<Action>(action),
                detail::forward_error<Expected>(closure)));
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
        template <nullable Expected, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires rebindable_error_to<
                Expected,
                std::invoke_result_t<Action, error_result_t<Expected>>>;
        };

        template <typename Action, nullable Expected, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Expected&& closure, Steps&&... steps)
        {
            if constexpr (is_applicable_on<Expected, Action>)
            {
                return transform_error::on_value(
                    std::forward<Action>(action),
                    std::forward<Expected>(closure),
                    std::forward<Steps>(steps)...);
            }
            else
            {
                return *transform_error::print_diagnostics<Expected, Action>();
            }
        }

        template <typename Action, nullable Expected, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Expected&& closure, Steps&&... steps)
        {
            if constexpr (is_applicable_on<Expected, Action>)
            {
                return transform_error::on_null(
                    std::forward<Action>(action),
                    std::forward<Expected>(closure),
                    std::forward<Steps>(steps)...);
            }
            else
            {
                return *transform_error::print_diagnostics<Expected, Action>();
            }
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

    /**
     * \brief Creates a pipeline step that transforms the error of an expected-like type.
     * \ingroup ALGORITHM
     * \tparam Action The action type.
     * \param action A nullary operation.
     * \return A Pipeline step containing the `or_else` algorithm.
     * \details
     * - **On Value**: Propagates the value state immediately (i.e., `action` is not executed).
     * - **On Null**: Invokes the `action` with the underlying error of the input.
     * The result of this invocation is wrapped as an error into a new instance of the `expected_like` container.
     *
     * Let `T` be the (possibly cv-qualified) reference to the error extracted from the input `expected_like`.
     * `Action` must be invocable with an argument of type `T` (or a type to which `T` is implicitly convertible),
     * while the decayed return-type will become the error-type of the resulting nullable.
     * \see https://en.wikipedia.org/wiki/Monad_(functional_programming)
     *
     * \note The `expected_like` type must support error-type rebinding.
     * \see gimo::traits::rebind_error
     */
    template <typename Action>
    [[nodiscard]]
    constexpr auto transform_error(Action&& action)
    {
        using Algorithm = detail::transform_error_t<Action>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Action>(action)}};
    }
}

#endif
