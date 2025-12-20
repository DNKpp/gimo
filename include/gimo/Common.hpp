//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_COMMON_HPP
#define GIMO_COMMON_HPP

#pragma once

#include "gimo/Config.hpp"

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
    constexpr decltype(auto) value(T&& nullable)
    {
        return *std::forward<T>(nullable);
    }

    template <typename T>
    concept nullable = requires(std::remove_cvref_t<T> closure) {
        requires null_for<decltype(traits<std::remove_cvref_t<T>>::null), std::remove_cvref_t<T>>;
        typename std::common_type_t<
            decltype(value(closure)),
            decltype(value(std::as_const(closure))),
            decltype(value(std::move(closure))),
            decltype(value(std::move(std::as_const(closure))))>;
    };

    template <typename T, typename Value>
    concept constructible_from_value = nullable<T>
                                    && detail::unqualified<T>
                                    && std::constructible_from<T, Value&&>;

    template <nullable Nullable, typename Value>
    using rebind_value_t = typename traits<std::remove_cvref_t<Nullable>>::template rebind_value<Value>;

    template <typename Nullable, typename Value>
    concept rebindable_value_to =
        nullable<Nullable>
        && requires(rebind_value_t<Nullable, Value> result) {
               requires constructible_from_value<decltype(result), Value>;
               { value(result) } -> std::convertible_to<Value const&>;
           };

    template <nullable Nullable>
    inline constexpr auto null_v{traits<std::remove_cvref_t<Nullable>>::null};

    namespace detail
    {
        template <nullable Nullable>
        using value_result_t = decltype(value(std::declval<Nullable&&>()));

        template <typename Nullable>
        [[nodiscard]]
        constexpr bool has_value(Nullable const& target)
        {
            return target != null_v<Nullable>;
        }

        template <nullable T>
        constexpr decltype(auto) forward_value(std::remove_reference_t<T>& nullable)
        {
            GIMO_ASSERT(detail::has_value(nullable), "Nullable must contain a value.", nullable);

            return value(std::forward<T>(nullable));
        }

        template <nullable Nullable>
        [[nodiscard]]
        constexpr auto construct_empty()
        {
            return Nullable{null_v<Nullable>};
        }

        template <nullable Nullable, typename Value>
        constexpr Nullable construct_from_value(Value&& value)
        {
            return Nullable{std::forward<Value>(value)};
        }

        template <nullable Nullable, nullable Source>
        [[nodiscard]]
        constexpr Nullable rebind_value(std::remove_reference_t<Source>& source)
        {
            return detail::construct_from_value<Nullable>(forward_value<Source>(source));
        }
    }

    template <typename T>
    concept readable_error = requires(T&& closure) {
        { std::forward<T>(closure).error() } -> detail::transferable;
    };

    template <readable_error T>
    constexpr decltype(auto) error(T&& nullable)
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

    template <typename T, typename Error>
    concept constructible_from_error =
        expected_like<T>
        && detail::unqualified<T>
        && requires(Error&& e) {
               { traits<T>::from_error(std::forward<Error>(e)) } -> std::same_as<T>;
           };

    template <expected_like Expected, typename Error>
    using rebind_error_t = typename traits<std::remove_cvref_t<Expected>>::template rebind_error<std::remove_cvref_t<Error>>;

    template <typename Expected, typename Error>
    concept rebindable_error_to =
        expected_like<Expected>
        && requires(rebind_error_t<Expected, Error> result) {
               requires constructible_from_error<decltype(result), Error>;
               { error(result) } -> std::convertible_to<Error const&>;
           };

    namespace detail
    {
        template <expected_like Expected>
        using error_result_t = decltype(error(std::declval<Expected&&>()));

        template <expected_like T>
        constexpr decltype(auto) forward_error(std::remove_reference_t<T>& expected)
        {
            GIMO_ASSERT(!detail::has_value(expected), "Expected must hold an error.", expected);

            return error(std::forward<T>(expected));
        }

        template <expected_like Expected, typename Error>
        constexpr Expected construct_from_error(Error&& error)
        {
            return traits<Expected>::from_error(std::forward<Error>(error));
        }

        template <expected_like Expected, expected_like Source>
        [[nodiscard]]
        constexpr Expected rebind_error(std::remove_reference_t<Source>& source)
        {
            return detail::construct_from_error<Expected>(forward_error<Source>(source));
        }
    }
}

#endif
