//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_COMMON_HPP
#define GIMO_COMMON_HPP

#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace gimo::detail
{
    template <typename T, typename U>
    struct const_ref_like
    {
        using type = std::conditional_t<
            std::is_const_v<std::remove_reference_t<T>>,
            std::remove_reference_t<U> const&&,
            std::remove_reference_t<U>&&>;
    };

    template <typename T, typename U>
    struct const_ref_like<T&, U>
    {
        using type = std::conditional_t<
            std::is_const_v<std::remove_reference_t<T>>,
            std::remove_reference_t<U> const&,
            std::remove_reference_t<U>&>;
    };

    template <typename T, typename U>
    using const_ref_like_t = const_ref_like<T, U>::type;

    template <typename T, typename U>
    [[nodiscard]]
    constexpr auto&& forward_like(U&& x) noexcept
    {
        return static_cast<const_ref_like_t<T, U>>(x);
    }

    template <typename T>
    concept unqualified = std::same_as<T, std::remove_cvref_t<T>>;

    template <typename T>
    concept transferable = std::constructible_from<std::remove_cvref_t<T>, T&&>;

    template <typename B>
    concept boolean_testable =
        std::convertible_to<B, bool>
        && requires(B&& b) {
               { !std::forward<B>(b) } -> std::convertible_to<bool>;
           };

    template <typename T, typename U>
    concept weakly_equality_comparable_with =
        requires(T const& t, U const& u) {
            { t == u } -> boolean_testable;
            { t != u } -> boolean_testable;
            { u == t } -> boolean_testable;
            { u != t } -> boolean_testable;
        };
}

namespace gimo
{
    template <typename Nullable>
    struct traits;

    namespace detail
    {
        template <typename T, typename U, typename C = std::common_reference_t<T const&, U const&>>
        concept regular_relationship_impl =
            std::same_as<
                std::common_reference_t<T const&, U const&>,
                std::common_reference_t<U const&, T const&>>
            && (std::convertible_to<T const&, C const&>
                || std::convertible_to<T, C const&>)
            && (std::convertible_to<U const&, C const&>
                || std::convertible_to<U, C const&>);

        template <typename Lhs, typename Rhs>
        concept weakly_assignable_from =
            std::is_lvalue_reference_v<Lhs>
            && requires(Lhs lhs, Rhs&& rhs) {
                   { lhs = std::forward<Rhs>(rhs) } -> std::same_as<Lhs>;
               };
    }

    template <typename T, typename U>
    concept regular_relationship =
        detail::regular_relationship_impl<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

    template <typename Null, typename Nullable>
    concept null_for = regular_relationship<Nullable, Null>
                    && detail::weakly_equality_comparable_with<Null, Nullable>
                    && std::constructible_from<Nullable, Null const&>
                    && detail::weakly_assignable_from<Nullable&, Null const&>;

    template <typename T>
    concept readable_value = requires(T&& closure) {
        { *std::forward<T>(closure) } -> detail::transferable;
    };

    template <readable_value T>
    constexpr auto&& value(T&& nullable)
    {
        return *std::forward<T>(nullable);
    }

    template <typename T>
    concept nullable = requires(T&& closure) {
        requires null_for<decltype(traits<std::remove_cvref_t<T>>::null), std::remove_cvref_t<T>>;
        typename std::common_type_t<
            decltype(value(closure)),
            decltype(value(std::as_const(closure))),
            decltype(value(std::move(closure))),
            decltype(value(std::move(std::as_const(closure))))>;
    };

    template <typename T, typename Nullable>
    concept adaptable_value_by = nullable<Nullable>
                              && detail::unqualified<Nullable>
                              && std::constructible_from<Nullable, T&&>;

    template <nullable Nullable>
    using reference_type_t = decltype(value(std::declval<Nullable&&>()));

    template <nullable Nullable>
    inline constexpr auto null_v{traits<std::remove_cvref_t<Nullable>>::null};

    template <nullable Nullable, typename Value>
    using rebind_value_t = typename traits<std::remove_cvref_t<Nullable>>::template rebind_value<Value>;

    namespace detail
    {
        template <nullable Nullable>
        [[nodiscard]]
        constexpr auto construct_empty()
        {
            return Nullable{null_v<Nullable>};
        }

        template <typename Nullable>
        [[nodiscard]]
        constexpr bool has_value(Nullable const& target)
        {
            return target != null_v<Nullable>;
        }
    }

    template <typename T>
    concept readable_error = requires(T&& closure) {
        { std::forward<T>(closure).error() } -> detail::transferable;
    };

    template <readable_error T>
    constexpr auto&& error(T&& nullable)
    {
        return std::forward<T>(nullable).error();
    }

    template <typename T>
    concept expected_like = nullable<T>
                         && requires(std::remove_cvref_t<T> closure) {
                                typename std::common_type_t<
                                    decltype(error(closure)),
                                    decltype(error(std::as_const(closure))),
                                    decltype(error(std::move(closure))),
                                    decltype(error(std::move(std::as_const(closure))))>;
                            };

    namespace detail
    {
        template <expected_like Expected, expected_like Source>
        [[nodiscard]]
        constexpr expected_like auto rebind_error(Source&& source)
        {
            GIMO_ASSERT(!detail::has_value(source), "Expected must not contain a value.", source);
            using traits = gimo::traits<std::remove_cvref_t<Expected>>;

            return traits::bind_error(
                error(std::forward<Source>(source)));
        }
    }
}

#endif
