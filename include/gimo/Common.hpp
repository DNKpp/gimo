//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_COMMON_HPP
#define GIMO_COMMON_HPP

#pragma once

#include "gimo/Config.hpp"

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace gimo::detail
{
    template <typename...>
    struct always_false
        : public std::bool_constant<false>
    {
    };

    template <typename... Args>
    inline constexpr bool always_false_v = always_false<Args...>::value;

    template <std::size_t priority>
    struct priority_tag
        /** \cond Help doxygen with recursion.*/
        : public priority_tag<priority - 1u>
    /** \endcond */
    {
    };

    template <>
    struct priority_tag<0u>
    {
    };

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
    concept transferable = !std::is_void_v<std::remove_cvref_t<T>>;

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
    /**
     * \brief The central customization point for the library.
     * \tparam Nullable The type to be adapted.
     * \details
     * To adapt a custom type for use with the monadic algorithms, you must specialize this struct.
     */
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

        template <typename T, typename U>
        concept regular_relationship =
            regular_relationship_impl<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

        template <typename Lhs, typename Rhs>
        concept weakly_assignable_from =
            std::is_lvalue_reference_v<Lhs>
            && requires(Lhs lhs, Rhs&& rhs) {
                   { lhs = std::forward<Rhs>(rhs) } -> std::same_as<Lhs>;
               };
    }

    /**
     * \brief Concept describing the relationship between a Nullable type and its *null* state.
     * \tparam Null The null type.
     * \tparam Nullable the nullable type.
     * \details
     * Requires that the Nullable type can be compared to, constructed from, and assigned from the Null type defined in its traits.
     */
    template <typename Null, typename Nullable>
    concept null_for = detail::regular_relationship<Nullable, Null>
                    && detail::weakly_equality_comparable_with<Null, Nullable>
                    && std::constructible_from<Nullable, Null const&>
                    && detail::weakly_assignable_from<Nullable&, Null const&>;

    namespace detail
    {
        template <typename T>
        concept trait_readable_value = requires(T&& closure) {
            { traits<std::remove_cvref_t<T>>::value(std::forward<T>(closure)) } -> transferable;
        };

        template <trait_readable_value T>
        constexpr decltype(auto) value_impl([[maybe_unused]] priority_tag<2u> const tag, T&& closure)
        {
            return traits<std::remove_cvref_t<T>>::value(std::forward<T>(closure));
        }

        template <typename T>
        concept indirectly_readable_value = requires(T&& closure) {
            { *std::forward<T>(closure) } -> transferable;
        };

        template <indirectly_readable_value T>
        constexpr decltype(auto) value_impl([[maybe_unused]] priority_tag<1u> const tag, T&& closure)
        {
            return *std::forward<T>(closure);
        }

        template <typename T>
        concept adl_readable_value = requires(T&& closure) {
            { value(std::forward<T>(closure)) } -> transferable;
        };

        template <adl_readable_value T>
        constexpr decltype(auto) value_impl([[maybe_unused]] priority_tag<0u> const tag, T&& closure)
        {
            return value(std::forward<T>(closure));
        }

        inline constexpr priority_tag<2u> max_value_tag{};

        template <typename T>
        concept readable_value = requires(T&& closure) {
            { detail::value_impl(max_value_tag, std::forward<T>(closure)) } -> transferable;
        };

        template <readable_value T>
        constexpr decltype(auto) value(T&& closure)
        {
            return detail::value_impl(max_value_tag, std::forward<T>(closure));
        }
    }

    /**
     * \brief Concept describing a type that can be used as a monad in the pipeline.
     * \tparam T The type to check.
     * \details
     * A type is `nullable` if:
     * 1. It has a valid `gimo::traits` specialization defining a `null` data member.
     * 2. It satisfies the `null_for` relationship with that `null`.
     * 3. It allows value extraction via `gimo::traits::value()`, member `operator*`, or ADL `value()` (in that priority).
     */
    template <typename T>
    concept nullable = requires {
        requires null_for<decltype(traits<std::remove_cvref_t<T>>::null), std::remove_cvref_t<T>>;
        requires detail::readable_value<T>;
    };

    namespace detail
    {
        template <typename Nullable, typename Value>
        concept trait_value_constructible = requires(Value&& value) {
            { traits<Nullable>::from_value(std::forward<Value>(value)) } -> std::same_as<Nullable>;
        };

        template <typename Nullable, typename Arg>
            requires trait_value_constructible<Nullable, Arg>
        constexpr Nullable construct_from_value_impl([[maybe_unused]] priority_tag<1u> const tag, Arg&& arg)
            noexcept(noexcept(traits<Nullable>::from_value(std::forward<Arg>(arg))))
        {
            return traits<Nullable>::from_value(std::forward<Arg>(arg));
        }

        template <typename Nullable, typename Arg>
            requires std::constructible_from<Nullable, Arg&&>
        constexpr Nullable construct_from_value_impl([[maybe_unused]] priority_tag<0u> const tag, Arg&& arg)
            noexcept(std::is_nothrow_constructible_v<Nullable, Arg&&>)
        {
            return Nullable{std::forward<Arg>(arg)};
        }

        inline constexpr priority_tag<1u> max_value_constructible_tag{};

        template <typename Nullable, typename Value>
        concept constructible_from_value = requires(Value&& value) {
            { detail::construct_from_value_impl<Nullable>(max_value_tag, std::forward<Value>(value)) } -> std::same_as<Nullable>;
        };

        template <typename Nullable, typename Value>
        concept nothrow_constructible_from_value = requires(Value&& value) {
            { detail::construct_from_value_impl<Nullable>(max_value_tag, std::forward<Value>(value)) } noexcept;
        };
    }

    /**
     * \brief Concept determining whether the `Nullable` is constructible with the specified argument.
     * \tparam T The type to check.
     * \tparam Arg The construction argument type to forward.
     * \details
     * A `nullable` is considered *constructible from value* if at least one of the following is true:
     * - **Priority 1:** Its `gimo::traits` specialization has an accessible `from_value` static function, which takes `Arg` as argument and returns `T`.
     * - **Priority 2:** It's (possibly explicitly) constructible from `Arg`.
     */
    template <typename T, typename Arg>
    concept constructible_from_value = nullable<T>
                                    && detail::unqualified<T>
                                    && detail::constructible_from_value<T, Arg&&>;

    /**
     * \brief Constructs the specified `Nullable` with the provided value.
     * \tparam Nullable The target-nullable type.
     * \tparam Arg The construction argument type to forward.
     * \param arg The argument forwarded to the actual construction strategy.
     * \return A newly created `Nullable` instance in a non-null state.
     * \note The exception specification matches the underlying construction strategy.
     * \details
     * The construction strategy is selected based on the following precedence:
     * - **Priority 1:** `gimo::traits<Nullable>::from_value`
     * - **Priority 2:** Direct initialization
     */
    template <nullable Nullable, typename Arg>
        requires constructible_from_value<Nullable, Arg&&>
    constexpr Nullable construct_from_value(Arg&& arg) noexcept(detail::nothrow_constructible_from_value<Nullable, Arg&&>)
    {
        return detail::construct_from_value_impl<Nullable>(detail::max_value_tag, std::forward<Arg>(arg));
    }

    /**
     * \brief Helper alias to obtain the `Nullable` type with the rebound `Value` type.
     * \tparam Nullable The source-nullable type to adapt the new value-type.
     * \tparam Value The new value-type to rebind to.
     */
    template <nullable Nullable, typename Value>
    using rebind_value_t = typename traits<std::remove_cvref_t<Nullable>>::template rebind_value<Value>;

    /**
     * \brief Concept determining whether the `Nullable` type supports rebinding its value-type.
     * \tparam Nullable The source-nullable type to adapt the new value-type.
     * \tparam Value The new value-type to rebind to.
     */
    template <typename Nullable, typename Value>
    concept rebindable_value_to =
        nullable<Nullable>
        && requires {
               requires constructible_from_value<rebind_value_t<Nullable, Value>, Value>;
               { detail::value(std::declval<rebind_value_t<Nullable, Value>>()) } -> std::convertible_to<Value const&>;
           };

    /**
     * \brief Helper to obtain the `null` value for a specific Nullable type.
     * \tparam Nullable The type for which to retrieve the null value.
     */
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

            return detail::value(std::forward<T>(nullable));
        }

        template <nullable Nullable>
        [[nodiscard]]
        constexpr auto construct_empty()
        {
            return Nullable{null_v<Nullable>};
        }

        template <nullable Nullable, nullable Source>
        [[nodiscard]]
        constexpr Nullable rebind_value(std::remove_reference_t<Source>& source)
        {
            return construct_from_value<Nullable>(forward_value<Source>(source));
        }
    }

    namespace detail
    {
        template <typename T>
        concept trait_readable_error = requires(T&& closure) {
            { traits<std::remove_cvref_t<T>>::error(std::forward<T>(closure)) } -> transferable;
        };

        template <trait_readable_error T>
        constexpr decltype(auto) error_impl([[maybe_unused]] priority_tag<2u> const tag, T&& closure)
        {
            return traits<std::remove_cvref_t<T>>::error(std::forward<T>(closure));
        }

        template <typename T>
        concept member_readable_error = requires(T&& closure) {
            { std::forward<T>(closure).error() } -> transferable;
        };

        template <member_readable_error T>
        constexpr decltype(auto) error_impl([[maybe_unused]] priority_tag<1u> const tag, T&& closure)
        {
            return std::forward<T>(closure).error();
        }

        template <typename T>
        concept adl_readable_error = requires(T&& closure) {
            { error(std::forward<T>(closure)) } -> transferable;
        };

        template <adl_readable_error T>
        constexpr decltype(auto) error_impl([[maybe_unused]] priority_tag<0u> const tag, T&& closure)
        {
            return error(std::forward<T>(closure));
        }

        inline constexpr priority_tag<2u> max_error_tag{};

        template <typename T>
        concept readable_error = requires(T&& closure) {
            { detail::error_impl(max_error_tag, std::forward<T>(closure)) } -> transferable;
        };

        template <readable_error T>
        constexpr decltype(auto) error(T&& closure)
        {
            return detail::error_impl(max_error_tag, std::forward<T>(closure));
        }
    }

    /**
     * \brief Concept describing a type that acts like `std::expected` (has both a value and an error channel).
     * \tparam T The type to check.
     * \details
     * An `expected_like` type must satisfy `nullable` and additionally support error extraction
     * via `gimo::traits::error()`, member `.error()`, or ADL `error()` (in that priority).
     */
    template <typename T>
    concept expected_like = nullable<T>
                         && detail::readable_error<T>;

    /**
     * \brief Concept determining whether the `Expected` type supports rebinding its error-type.
     * \tparam T The source-expected type to adapt the new error-type.
     * \tparam Error The new error-type to rebind to.
     */
    template <typename T, typename Error>
    concept constructible_from_error =
        expected_like<T>
        && detail::unqualified<T>
        && requires(Error&& e) {
               { traits<T>::from_error(std::forward<Error>(e)) } -> std::same_as<T>;
           };

    /**
     * \brief Helper alias to obtain the `Expected` type with the rebound `Error` type.
     * \tparam Expected The source-expected type to adapt the new error-type.
     * \tparam Error The new error-type to rebind to.
     */
    template <expected_like Expected, typename Error>
    using rebind_error_t = typename traits<std::remove_cvref_t<Expected>>::template rebind_error<std::remove_cvref_t<Error>>;

    /**
     * \brief Concept determining whether the `Expected` type supports rebinding its error-type.
     * \tparam Expected The source-expected type to adapt the new error-type.
     * \tparam Error The new error-type to rebind to.
     */
    template <typename Expected, typename Error>
    concept rebindable_error_to =
        expected_like<Expected>
        && requires {
               requires constructible_from_error<rebind_error_t<Expected, Error>, Error>;
               { detail::error(std::declval<rebind_error_t<Expected, Error>&&>()) } -> std::convertible_to<Error const&>;
           };

    namespace detail
    {
        template <expected_like Expected>
        using error_result_t = decltype(error(std::declval<Expected&&>()));

        template <expected_like T>
        constexpr decltype(auto) forward_error(std::remove_reference_t<T>& expected)
        {
            GIMO_ASSERT(!detail::has_value(expected), "Expected must hold an error.", expected);

            return detail::error(std::forward<T>(expected));
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
