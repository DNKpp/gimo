//          Copyright Dominic (DNKpp) Koepke 2026.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_AND_FORWARD_HPP
#define GIMO_ALGORITHM_AND_FORWARD_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Pipeline.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <concepts>
#include <functional>
#include <tuple>
#include <utility>

namespace gimo::detail::and_forward
{
    template <typename Nullable, typename Action>
    consteval void print_diagnostics()
    {
        if constexpr (!std::invocable<Action, value_result_t<Nullable>>)
        {
            static_assert(always_false_v<Nullable>, "The and_forward algorithm requires an action invocable with the nullable's value.");
        }
    }

    struct traits
    {
        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires std::invocable<Action, value_result_t<Nullable>>;
        };

        template <typename Action, nullable Nullable>
        static constexpr void on_value(Action&& action, Nullable&& opt)
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable is empty while it's expected to contain a value.");

            if constexpr (is_applicable_on<Nullable, Action>)
            {
                std::invoke(
                    std::forward<Action>(action),
                    detail::forward_value<Nullable>(opt));
            }
            else
            {
                and_forward::print_diagnostics<Nullable, Action>();
            }
        }

        template <typename Action, nullable Nullable>
        static constexpr void on_null(Action&& /*action*/, [[maybe_unused]] Nullable&& opt)
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable contains a value while it's expected to be empty.");

            if constexpr (!is_applicable_on<Nullable, Action>)
            {
                and_forward::print_diagnostics<Nullable, Action>();
            }
        }
    };
}

namespace gimo
{
    namespace detail
    {
        template <typename Action>
        using and_forward_t = BasicAlgorithm<
            and_forward::traits,
            std::remove_cvref_t<Action>>;
    }

    /**
     * \brief Creates a terminating pipeline step that forwards the contained value to the specified action.
     * \ingroup ALGORITHM
     * \tparam Action The action type.
     * \param action A unary operation.
     * \return A Pipeline step containing the `and_forward` algorithm.
     * \details
     * - **On Value**: Invokes the `action` with the underlying value of the input.
     *      Any returned value by `action` will be discarded, thus `action` may return `void`.
     * - **On Null**: Silently terminates the pipeline.
     */
    template <typename Action>
    [[nodiscard]]
    constexpr auto and_forward(Action&& action)
    {
        using Algorithm = detail::and_forward_t<Action>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Action>(action)}};
    }
}

#endif
