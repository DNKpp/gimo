//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_HPP
#define GIMO_HPP


/*** Start of inlined file: Version.hpp ***/
//          Copyright Dominic (DNKpp) Koepke 2025 - 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_VERSION_HPP
#define GIMO_VERSION_HPP

#pragma once

#define GIMO_VERSION_MAJOR 0
#define GIMO_VERSION_MINOR 1
#define GIMO_VERSION_PATCH 0

#endif

/*** End of inlined file: Version.hpp ***/


/*** Start of inlined file: Config.hpp ***/
//           Copyright Dominic (DNKpp) Koepke 2025
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_CONFIG_HPP
#define GIMO_CONFIG_HPP

#ifndef GIMO_ASSERT
    #include <cassert>
    #define GIMO_ASSERT(condition, msg, ...) assert((condition) && msg)
#endif

#endif

/*** End of inlined file: Config.hpp ***/


/*** Start of inlined file: Common.hpp ***/
//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_COMMON_HPP
#define GIMO_COMMON_HPP

#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace gimo::detail
{
    template <std::size_t priority>
    struct priority_tag
        /** \cond Help doxygen with recursion.*/
        : public priority_tag<priority - 1>
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

    template <typename T>
    concept nullable = requires {
        requires null_for<decltype(traits<std::remove_cvref_t<T>>::null), std::remove_cvref_t<T>>;
        requires detail::readable_value<T>;
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
               { detail::value(result) } -> std::convertible_to<Value const&>;
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

            return detail::value(std::forward<T>(nullable));
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

    template <typename T>
    concept expected_like = nullable<T>
                         && detail::readable_error<T>;

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
               { detail::error(result) } -> std::convertible_to<Error const&>;
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

/*** End of inlined file: Common.hpp ***/


/*** Start of inlined file: Pipeline.hpp ***/
//           Copyright Dominic (DNKpp) Koepke 2025
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_PIPELINE_HPP
#define GIMO_PIPELINE_HPP

#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo
{
    template <typename... Steps>
    class Pipeline
    {
        template <typename... Others>
        friend class Pipeline;

    public:
        [[nodiscard]]
        explicit constexpr Pipeline(std::tuple<Steps...> steps)
            : m_Steps{std::move(steps)}
        {
        }

        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) &
        {
            return apply(*this, std::forward<Nullable>(opt));
        }

        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) const&
        {
            return apply(*this, std::forward<Nullable>(opt));
        }

        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) &&
        {
            return apply(std::move(*this), std::forward<Nullable>(opt));
        }

        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) const&&
        {
            return apply(std::move(*this), std::forward<Nullable>(opt));
        }

        template <typename... SuffixSteps>
        constexpr auto append(Pipeline<SuffixSteps...> suffix) const&
        {
            return append(*this, std::move(suffix.m_Steps));
        }

        template <typename... SuffixSteps>
        constexpr auto append(Pipeline<SuffixSteps...> suffix) &&
        {
            return append(std::move(*this), std::move(suffix.m_Steps));
        }

        template <typename... SuffixSteps>
        [[nodiscard]]
        friend constexpr auto operator|(Pipeline const& prefix, Pipeline<SuffixSteps...> suffix)
        {
            return prefix.append(std::move(suffix));
        }

        template <typename... SuffixSteps>
        [[nodiscard]]
        friend constexpr auto operator|(Pipeline&& prefix, Pipeline<SuffixSteps...> suffix)
        {
            return std::move(prefix).append(std::move(suffix));
        }

    private:
        std::tuple<Steps...> m_Steps{};

        template <typename Self, typename Nullable>
        [[nodiscard]]
        static constexpr auto apply(Self&& self, Nullable&& opt)
        {
            return std::apply(
                [&]<typename First, typename... Others>(First&& first, Others&&... steps) {
                    return std::invoke(
                        std::forward<First>(first),
                        std::forward<Nullable>(opt),
                        std::forward<Others>(steps)...);
                },
                std::forward<Self>(self).m_Steps);
        }

        template <typename Self, typename... SuffixSteps>
        [[nodiscard]]
        static constexpr auto append(Self&& self, std::tuple<SuffixSteps...>&& suffixSteps)
        {
            using Appended = Pipeline<Steps..., SuffixSteps...>;

            return Appended{
                std::tuple_cat(std::forward<Self>(self).m_Steps, std::move(suffixSteps))};
        }
    };

    namespace detail
    {
        template <typename T>
        struct is_pipeline
            : public std::false_type
        {
        };

        template <typename... Steps>
        struct is_pipeline<Pipeline<Steps...>>
            : public std::true_type
        {
        };
    }

    template <typename T>
    concept pipeline = detail::is_pipeline<std::remove_cvref_t<T>>::value;

    template <nullable Nullable, pipeline Pipeline>
    [[nodiscard]]
    constexpr auto apply(Nullable&& opt, Pipeline&& steps)
    {
        return std::forward<Pipeline>(steps).apply(std::forward<Nullable>(opt));
    }
}

#endif

/*** End of inlined file: Pipeline.hpp ***/


/*** Start of inlined file: BasicAlgorithm.hpp ***/
//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_COMMON_HPP
#define GIMO_ALGORITHM_COMMON_HPP

#pragma once

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

        template <applicable_on<BasicAlgorithm&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) &
        {
            return detail::test_and_execute<Traits>(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm const&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) const&
        {
            return detail::test_and_execute<Traits>(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm&&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) &&
        {
            return detail::test_and_execute<Traits>(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm const&&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto operator()(Nullable&& opt, Steps&&... steps) const&&
        {
            return detail::test_and_execute<Traits>(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) &
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm const&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) const&
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm&&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) &&
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm const&&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_value(Nullable&& opt, Steps&&... steps) const&&
        {
            GIMO_ASSERT(detail::has_value(opt), "Nullable must contain a value.", opt);

            return Traits::on_value(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_null(Nullable&& opt, Steps&&... steps) &
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable must not contain a value.", opt);

            return Traits::on_null(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm const&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_null(Nullable&& opt, Steps&&... steps) const&
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable must not contain a value.", opt);

            return Traits::on_null(
                m_Action,
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm&&> Nullable, typename... Steps>
        [[nodiscard]]
        constexpr auto on_null(Nullable&& opt, Steps&&... steps) &&
        {
            GIMO_ASSERT(!detail::has_value(opt), "Nullable must not contain a value.", opt);

            return Traits::on_null(
                std::move(m_Action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <applicable_on<BasicAlgorithm const&&> Nullable, typename... Steps>
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

/*** End of inlined file: BasicAlgorithm.hpp ***/


/*** Start of inlined file: AndThen.hpp ***/
//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_AND_THEN_HPP
#define GIMO_ALGORITHM_AND_THEN_HPP

#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo::detail::and_then
{
    template <typename Nullable, typename Action>
    using result_t = std::invoke_result_t<
        Action,
        value_result_t<Nullable>>;

    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr auto on_value(Action&& action, Nullable&& opt)
    {
        return std::invoke(
            std::forward<Action>(action),
            detail::forward_value<Nullable>(opt));
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
        return detail::construct_empty<result_t<Nullable, Action>>();
    }

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr auto on_null([[maybe_unused]] Action&& action, Expected&& expected)
    {
        return detail::rebind_error<result_t<Expected, Action>, Expected>(expected);
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
            requires nullable<result_t<Nullable, Action>>;
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

/*** End of inlined file: AndThen.hpp ***/


/*** Start of inlined file: OrElse.hpp ***/
//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_OR_ELSE_HPP
#define GIMO_ALGORITHM_OR_ELSE_HPP

#pragma once

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

/*** End of inlined file: OrElse.hpp ***/


/*** Start of inlined file: Transform.hpp ***/
//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_TRANSFORM_HPP
#define GIMO_ALGORITHM_TRANSFORM_HPP

#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo::detail::transform
{
    template <typename Nullable, typename Action>
    using result_t = rebind_value_t<
        Nullable,
        std::invoke_result_t<Action, value_result_t<Nullable>>>;

    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr auto on_value([[maybe_unused]] Action&& action, Nullable&& opt)
    {
        return detail::construct_from_value<result_t<Nullable, Action>>(
            std::invoke(
                std::forward<Action>(action),
                detail::forward_value<Nullable>(opt)));
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
            transform::on_value(std::forward<Action>(action), std::forward<Nullable>(opt)),
            std::forward<Steps>(steps)...);
    }

    template <typename Action, nullable Nullable>
    [[nodiscard]]
    constexpr auto on_null([[maybe_unused]] Action&& action, [[maybe_unused]] Nullable&& opt)
    {
        return detail::construct_empty<result_t<Nullable, Action>>();
    }

    template <typename Action, expected_like Expected>
    [[nodiscard]]
    constexpr auto on_null([[maybe_unused]] Action&& action, Expected&& expected)
    {
        return detail::rebind_error<result_t<Expected, Action>, Expected>(expected);
    }

    template <nullable Nullable, typename Action, typename Next, typename... Steps>
    [[nodiscard]]
    constexpr auto on_null(Action&& action, Nullable&& opt, Next&& next, Steps&&... steps)
    {
        return std::forward<Next>(next).on_null(
            transform::on_null(std::forward<Action>(action), std::forward<Nullable>(opt)),
            std::forward<Steps>(steps)...);
    }

    struct traits
    {
        template <nullable Nullable, typename Action>
        static constexpr bool is_applicable_on = requires {
            requires rebindable_value_to<
                Nullable,
                std::invoke_result_t<Action, value_result_t<Nullable>>>;
        };

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_value(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return transform::on_value(
                std::forward<Action>(action),
                std::forward<Nullable>(opt),
                std::forward<Steps>(steps)...);
        }

        template <typename Action, nullable Nullable, typename... Steps>
        [[nodiscard]]
        static constexpr auto on_null(Action&& action, Nullable&& opt, Steps&&... steps)
        {
            return transform::on_null(
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
        using transform_t = BasicAlgorithm<transform::traits, std::remove_cvref_t<Action>>;
    }

    template <typename Action>
    [[nodiscard]]
    constexpr auto transform(Action&& action)
    {
        using Algorithm = detail::transform_t<Action>;

        return Pipeline{std::tuple<Algorithm>{std::forward<Action>(action)}};
    }
}

#endif

/*** End of inlined file: Transform.hpp ***/


/*** Start of inlined file: TransformError.hpp ***/
//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_ALGORITHM_TRANSFORM_ERROR_HPP
#define GIMO_ALGORITHM_TRANSFORM_ERROR_HPP

#pragma once

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
    constexpr auto on_null(Action&& action, Expected&& closure)
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

/*** End of inlined file: TransformError.hpp ***/

#endif

