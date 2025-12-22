//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "gimo.hpp"

/*
<begin-expected-compile-error>
The transform_error algorithm requires an expected-like whose error-type can be rebound\.
<end-expected-compile-error>
*/

namespace
{
    class NonRebindableError
    {
    public:
        struct null_t
        {
            [[nodiscard]]
            friend constexpr bool operator==([[maybe_unused]] NonRebindableError const& expected, [[maybe_unused]] null_t const tag) noexcept
            {
                return true;
            }

            [[nodiscard]]
            explicit(false) constexpr operator NonRebindableError() const noexcept
            {
                return NonRebindableError{};
            }
        };

        [[nodiscard]]
        constexpr int operator*() const
        {
            return 42;
        }

        [[nodiscard]]
        constexpr int error() const
        {
            return 1337;
        }
    };
}

template <>
struct gimo::traits<NonRebindableError>
{
    static constexpr NonRebindableError::null_t null{};
};

static_assert(gimo::expected_like<NonRebindableError>);

void check()
{
    std::ignore = gimo::apply(
        NonRebindableError{},
        gimo::transform_error(std::identity{}));
}
