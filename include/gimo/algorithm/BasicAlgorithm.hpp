//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_COMMON_HPP
#define GIMO_ALGORITHM_COMMON_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/Config.hpp"

#include <type_traits>
#include <utility>

namespace gimo
{
    namespace detail
    {
        template <typename Traits, typename Action, typename Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto test_and_execute(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            if (detail::has_value(opt))
            {
                return Traits::on_value(
                    std::forward<Action>(action),
                    std::forward<Nullable>(opt),
                    std::forward<Steps>(steps)...);
            }

            return Traits::on_null(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename Traits, typename Action>
        concept applicable_on = Traits::template is_applicable_on<Nullable, Action>;
    }

    template <typename Nullable, typename Algorithm>
    concept applicable_on = requires {
        requires detail::applicable_on<
            Nullable,
            typename std::remove_cvref_t<Algorithm>::traits_type,
            detail::const_ref_like_t<Algorithm, typename std::remove_cvref_t<Algorithm>::action_type>>;
    };

    template <detail::unqualified Traits, detail::unqualified Action>
    class BasicAlgorithm
    {
    public:
        using traits_type = Traits;
        using action_type = Action;

        template <typename... Args>
            requires std::constructible_from<Action, Args&&...>
        [[nodiscard]] //
        explicit constexpr BasicAlgorithm(Args&&... args) noexcept(std::is_nothrow_constructible_v<Action, Args&&...>)
            : m_Action{std::forward<Args>(args)...}
        {
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) &
        {
            return detail::test_and_execute<Traits>(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) const&
        {
            return detail::test_and_execute<Traits>(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) &&
        {
            return detail::test_and_execute<Traits>(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) const&&
        {
            return detail::test_and_execute<Traits>(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) &
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) const&
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) &&
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) const&&
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_null(Nullable&& opt, Steps&&... steps) &
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable must not contain a value.", opt);

            return Traits::on_null(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_null(Nullable&& opt, Steps&&... steps) const&
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable must not contain a value.", opt);

            return Traits::on_null(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_null(Nullable&& opt, Steps&&... steps) &&
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable must not contain a value.", opt);

            return Traits::on_null(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_null(Nullable&& opt, Steps&&... steps) const&&
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable must not contain a value.", opt);

            return Traits::on_null(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

    private:
        [[no_unique_address]] Action m_Action;
    };
}

#endif
