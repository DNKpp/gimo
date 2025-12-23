//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_PIPELINE_HPP
#define GIMO_PIPELINE_HPP

#pragma once

#include "gimo/Common.hpp"
#include "gimo/algorithm/BasicAlgorithm.hpp"

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace gimo
{
    /**
     * \brief A composite object representing a sequence of monadic operations.
     * \tparam Steps The sequence of algorithm types contained in this pipeline.
     * \details
     * Pipelines are created by chaining algorithms (like `transform` or `and_then`) and are executed by calling `apply` member-function,
     * or `gimo::apply`.
     */
    template <typename... Steps>
    class Pipeline
    {
        template <typename... Others>
        friend class Pipeline;

    public:
        /**
         * \brief Constructs a pipeline from a tuple of steps.
         * \param steps The tuple containing the algorithm instances.
         */
        [[nodiscard]]
        explicit constexpr Pipeline(std::tuple<Steps...> steps)
            : m_Steps{std::move(steps)}
        {
        }

        /**
         * \brief Applies nullable input on the pipeline.
         * \tparam Nullable The input type.
         * \param opt The input value to process.
         * \return The result of the pipeline execution.
         */
        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) &
        {
            return apply(*this, std::forward<Nullable>(opt));
        }

        /**
         * \copydoc apply
         */
        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) const&
        {
            return apply(*this, std::forward<Nullable>(opt));
        }

        /**
         * \copydoc apply
         */
        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) &&
        {
            return apply(std::move(*this), std::forward<Nullable>(opt));
        }

        /**
         * \copydoc apply
         */
        template <nullable Nullable>
        constexpr auto apply(Nullable&& opt) const&&
        {
            return apply(std::move(*this), std::forward<Nullable>(opt));
        }

        /**
         * \brief Appends another pipeline to the end of this one.
         * \tparam SuffixSteps The steps of the appended pipeline.
         * \return A new Pipeline containing all steps from both pipelines.
         */
        template <typename... SuffixSteps>
        constexpr auto append(Pipeline<SuffixSteps...> suffix) const&
        {
            return append(*this, std::move(suffix.m_Steps));
        }

        /**
         * \copydoc append
         */
        template <typename... SuffixSteps>
        constexpr auto append(Pipeline<SuffixSteps...> suffix) &&
        {
            return append(std::move(*this), std::move(suffix.m_Steps));
        }

        /**
         * \brief Appends the right-hand-side pipeline to the end of the left-hand-side pipeline.
         * \tparam SuffixSteps The steps of the appended pipeline.
         * \return A new Pipeline containing all steps from both pipelines.
         */
        template <typename... SuffixSteps>
        [[nodiscard]]
        friend constexpr auto operator|(Pipeline const& prefix, Pipeline<SuffixSteps...> suffix)
        {
            return prefix.append(std::move(suffix));
        }

        /**
         * \copydoc operator|
         */
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

    /**
     * \brief Checks whether the given type is a specialization of `gimo::Pipeline`.
     * \tparam T The type to check.
     */
    template <typename T>
    concept pipeline = detail::is_pipeline<std::remove_cvref_t<T>>::value;

    /**
     * \brief Applies nullable input on the pipeline.
     * \relates Pipeline
     * \tparam Nullable The input type.
     * \tparam Pipeline The pipeline type.
     * \param opt The input value to process.
     * \param steps The pipeline to execute.
     * \return The result of the pipeline execution.
     */
    template <nullable Nullable, pipeline Pipeline>
    [[nodiscard]]
    constexpr auto apply(Nullable&& opt, Pipeline&& steps)
    {
        return std::forward<Pipeline>(steps).apply(std::forward<Nullable>(opt));
    }

    namespace detail
    {
        template <typename Nullable, typename ConstRefSource, typename... Steps>
        struct is_processable_by_impl
            : public std::bool_constant<0u == sizeof...(Steps)>
        {
        };

        template <typename Nullable, typename ConstRefSource, typename First, typename... Rest>
            requires applicable_to<Nullable, const_ref_like_t<ConstRefSource, First>>
        struct is_processable_by_impl<Nullable, ConstRefSource, First, Rest...>
            : public is_processable_by_impl<
                  std::invoke_result_t<const_ref_like_t<ConstRefSource, First>, Nullable>,
                  ConstRefSource,
                  Rest...>
        {
        };

        template <typename Nullable, typename Pipeline, typename StepList = std::remove_cvref_t<Pipeline>>
        struct is_processable_by;

        template <typename Nullable, typename ConstRefSource, typename... Steps>
        struct is_processable_by<Nullable, ConstRefSource, Pipeline<Steps...>>
            : public is_processable_by_impl<Nullable, ConstRefSource, Steps...>
        {
        };
    }

    /**
     * \brief Evaluates whether a `Nullable` type can be processed by the entire pipeline.
     * \tparam Nullable The nullable type.
     * \tparam Pipeline A `Pipeline` specialization.
     */
    template <typename Nullable, typename Pipeline>
    concept processable_by = nullable<Nullable>
                          && pipeline<Pipeline>
                          && detail::is_processable_by<Nullable, Pipeline>::value;
}

#endif
