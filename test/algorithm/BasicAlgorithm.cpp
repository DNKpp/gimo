//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#include "gimo/algorithm/BasicAlgorithm.hpp"
#include "gimo_ext/StdOptional.hpp"

#include "TestCommons.hpp"

using namespace gimo;

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

        mimicpp::Mock<
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

        mimicpp::Mock<bool() const> is_null{
            {.name = "NullableMock::is_null", .stacktraceSkip = 1u}
        };

        [[nodiscard]]
        constexpr bool operator==([[maybe_unused]] NullableNull const null) const
        {
            return is_null();
        }

        mimicpp::Mock<void()> set_null{
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

TEMPLATE_LIST_TEST_CASE(
    "BasicAlgorithm::operator() propagates the nullable as-is.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using matches::_;

    using ActionMock = mimicpp::Mock<void()>;
    testing::AlgorithmMock<ActionMock> algorithm{};
    NullableMock<int> opt{};
    mimicpp::ScopedSequence sequence{};

    using Cast = TestType;
    using ExpectedOptRef = Cast::template type<NullableMock<int>>;

    SECTION("When nullable contains a value.")
    {
        sequence += opt.is_null.expect_call()
                and finally::returns(false);

        SECTION("When algorithm is invoked as mutable lvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_value_<ActionMock&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = algorithm(Cast::cast(opt));
            CHECK(1337 == result);
        }

        SECTION("When algorithm is invoked as const lvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_value_<ActionMock const&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = std::as_const(algorithm)(Cast::cast(opt));
            CHECK(1337 == result);
        }

        SECTION("When algorithm is invoked as mutable rvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_value_<ActionMock&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = std::move(algorithm)(Cast::cast(opt));
            CHECK(1337 == result);
        }

        SECTION("When algorithm is invoked as const rvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_value_<ActionMock const&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = std::move(std::as_const(algorithm))(Cast::cast(opt));
            CHECK(1337 == result);
        }
    }

    SECTION("When nullable is empty.")
    {
        sequence += opt.is_null.expect_call()
                and finally::returns(true);

        SECTION("When algorithm is invoked as mutable lvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_null_<ActionMock&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = algorithm(Cast::cast(opt));
            CHECK(1337 == result);
        }

        SECTION("When algorithm is invoked as const lvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_null_<ActionMock const&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = std::as_const(algorithm)(Cast::cast(opt));
            CHECK(1337 == result);
        }

        SECTION("When algorithm is invoked as mutable rvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_null_<ActionMock&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = std::move(algorithm)(Cast::cast(opt));
            CHECK(1337 == result);
        }

        SECTION("When algorithm is invoked as const rvalue-ref.")
        {
            sequence += testing::AlgorithmMockTraits::on_null_<ActionMock const&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                    and finally::returns(1337);

            std::optional<int> const result = std::move(std::as_const(algorithm))(Cast::cast(opt));
            CHECK(1337 == result);
        }
    }
}

TEMPLATE_LIST_TEST_CASE(
    "BasicAlgorithm::on_value propagates the nullable as-is to Traits::on_value.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using matches::_;

    using ActionMock = mimicpp::Mock<void()>;
    testing::AlgorithmMock<ActionMock> algorithm{};
    NullableMock<int> opt{};
    mimicpp::ScopedSequence sequence{};

    using with_qualification = TestType;
    using ExpectedOptRef = with_qualification::template type<NullableMock<int>>;

    SECTION("When algorithm is invoked as mutable lvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_value_<ActionMock&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = algorithm.on_value(with_qualification::cast(opt));
        CHECK(1337 == result);
    }

    SECTION("When algorithm is invoked as const lvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_value_<ActionMock const&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = std::as_const(algorithm).on_value(with_qualification::cast(opt));
        CHECK(1337 == result);
    }

    SECTION("When algorithm is invoked as mutable rvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_value_<ActionMock&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = std::move(algorithm).on_value(with_qualification::cast(opt));
        CHECK(1337 == result);
    }

    SECTION("When algorithm is invoked as const rvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_value_<ActionMock const&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = std::move(std::as_const(algorithm)).on_value(with_qualification::cast(opt));
        CHECK(1337 == result);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "BasicAlgorithm::on_null propagates the nullable as-is to Traits::on_null.",
    "[algorithm]",
    testing::with_qualification_list)
{
    using matches::_;

    using ActionMock = mimicpp::Mock<void()>;
    testing::AlgorithmMock<ActionMock> algorithm{};
    NullableMock<int> opt{};
    mimicpp::ScopedSequence sequence{};

    using with_qualification = TestType;
    using ExpectedOptRef = with_qualification::template type<NullableMock<int>>;

    SECTION("When algorithm is invoked as mutable lvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_null_<ActionMock&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = algorithm.on_null(with_qualification::cast(opt));
        CHECK(1337 == result);
    }

    SECTION("When algorithm is invoked as const lvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_null_<ActionMock const&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = std::as_const(algorithm).on_null(with_qualification::cast(opt));
        CHECK(1337 == result);
    }

    SECTION("When algorithm is invoked as mutable rvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_null_<ActionMock&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = std::move(algorithm).on_null(with_qualification::cast(opt));
        CHECK(1337 == result);
    }

    SECTION("When algorithm is invoked as const rvalue-ref.")
    {
        sequence += testing::AlgorithmMockTraits::on_null_<ActionMock const&&, ExpectedOptRef>.expect_call(_, matches::instance(opt))
                and finally::returns(1337);

        std::optional<int> const result = std::move(std::as_const(algorithm)).on_null(with_qualification::cast(opt));
        CHECK(1337 == result);
    }
}
