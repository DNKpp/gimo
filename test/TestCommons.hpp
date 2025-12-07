//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <tuple>
#include <type_traits>

namespace gimo::testing
{
    struct as_lvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T>&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    struct as_const_lvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T> const&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    struct as_rvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T>&&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T&& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    struct as_const_rvalue_ref
    {
        template <typename T>
        using type = std::remove_cvref_t<T> const&&;

        template <typename T>
        [[nodiscard]]
        static constexpr type<T> cast(T&& obj) noexcept
        {
            return static_cast<type<T>>(obj);
        }
    };

    using with_qualification_list = std::tuple<
        as_lvalue_ref,
        as_const_lvalue_ref,
        as_rvalue_ref,
        as_const_rvalue_ref>;

    struct AlgorithmMockTraits
    {
        template <nullable Nullable>
        using output = std::optional<std::remove_cvref_t<reference_type_t<Nullable>>>;

        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = true;

        template <typename Action, nullable Nullable, typename... Steps>
        inline static mimicpp::Mock<std::remove_cvref_t<output<Nullable>>(Action, Nullable, Steps...) const> on_value_{
            {"AlgorithmMock::on_value", 1u}
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return on_value_<Action&&, Nullable&&, Steps&&...>(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Action, nullable Nullable, typename... Steps>
        inline static mimicpp::Mock<std::remove_cvref_t<output<Nullable>>(Action, Nullable, Steps...) const> on_null_{
            {"AlgorithmMock::on_null", 1u}
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return on_null_<Action&&, Nullable&&, Steps&&...>(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }
    };

    template <typename Action>
    using AlgorithmMock = BasicAlgorithm<AlgorithmMockTraits, Action>;
}
