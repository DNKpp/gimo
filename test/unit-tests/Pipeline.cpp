//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/AndThen.hpp"

namespace
{
    struct NullableNull
    {
    };

    template <typename T>
    class NullableMock
    {
    public:
        NullableMock() = default;
        NullableMock(NullableMock&&) = default;
        NullableMock& operator=(NullableMock&&) = default;

        explicit(false) constexpr NullableMock([[maybe_unused]] NullableNull const null) noexcept
        {
        }

        inline static mimicpp::Mock<
            T&()&,
            T const&() const&,
            T && ()&&,
            T const && () const&&>
            value{
                {.name = "NullableMock::value", .stacktraceSkip = 1u}
        };

        [[nodiscard]]
        constexpr T& operator*() &
        {
            return value();
        }

        [[nodiscard]]
        constexpr T const& operator*() const&
        {
            return value();
        }

        [[nodiscard]]
        constexpr T&& operator*() &&
        {
            return std::move(value)();
        }

        [[nodiscard]]
        constexpr T const&& operator*() const&&
        {
            return std::move(std::as_const(value))();
        }

        inline static mimicpp::Mock<bool() const> is_null{
            {.name = "NullableMock::is_null", .stacktraceSkip = 1u}
        };

        [[nodiscard]]
        constexpr bool operator==([[maybe_unused]] NullableNull const null) const
        {
            return is_null();
        }

        inline static mimicpp::Mock<void()> set_null{
            {.name = "NullableMock::set_null", .stacktraceSkip = 1u}
        };

        NullableMock& operator=([[maybe_unused]] NullableNull const null)
        {
            set_null();
            return *this;
        }
    };
}

template <typename T>
struct gimo::traits<NullableMock<T>>
{
    static constexpr NullableNull null{};
};

static_assert(gimo::nullable<NullableMock<int>>);

TEST_CASE(
    "Pipelines can be appended.",
    "[pipeline]")
{
    mimicpp::Mock<NullableMock<int>(float)> action1{};
    mimicpp::Mock<NullableMock<bool>(int)> action2{};

    NullableMock<float> nullable{};
    auto pipeline = gimo::and_then(std::ref(action1))
                  | gimo::and_then(std::ref(action2));
    STATIC_CHECK(gimo::processable_by<NullableMock<float>, decltype(pipeline)>);

    mimicpp::ScopedSequence sequence{};
    SECTION("When the nullable is null.")
    {
        sequence += NullableMock<float>::is_null.expect_call()
                and finally::returns(true);

        decltype(auto) result = pipeline.apply(nullable);
        STATIC_CHECK(std::same_as<NullableMock<bool>, decltype(result)>);
    }

    SECTION("When nullable is not empty.")
    {
        sequence += NullableMock<float>::is_null.expect_call()
                and finally::returns(false);
        sequence += NullableMock<float>::value.expect_call()
                and finally::returns(42.f);
        sequence += action1.expect_call(42.f)
                and finally::returns_result_of([] { return NullableMock<int>{}; });

        SECTION("And the returned nullable is also not empty.")
        {
            sequence += NullableMock<int>::is_null.expect_call()
                    and finally::returns(false);
            sequence += std::move(NullableMock<int>::value).expect_call()
                    and finally::returns(1337);
            sequence += action2.expect_call(1337)
                    and finally::returns_result_of([] { return NullableMock<bool>{}; });

            decltype(auto) result = pipeline.apply(nullable);
            STATIC_CHECK(std::same_as<NullableMock<bool>, decltype(result)>);
        }

        SECTION("And the returned nullable is null.")
        {
            sequence += NullableMock<int>::is_null.expect_call()
                    and finally::returns(true);

            decltype(auto) result = pipeline.apply(nullable);
            STATIC_CHECK(std::same_as<NullableMock<bool>, decltype(result)>);
        }
    }
}
