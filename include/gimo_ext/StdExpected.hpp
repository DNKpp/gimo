//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_EXT_STD_EXPECTED_HPP
#define GIMO_EXT_STD_EXPECTED_HPP

#pragma once

#include <expected>

namespace gimo
{
    template <typename T>
    struct traits;
}

template <typename Value, typename Error>
struct gimo::traits<std::expected<Value, Error>>
{
    using expected = std::expected<Value, Error>;

    struct null_t
    {
        [[nodiscard]]
        friend constexpr bool operator==(expected const& expected, [[maybe_unused]] null_t const tag) noexcept
        {
            return !expected.has_value();
        }

        [[nodiscard]]
        explicit(false) constexpr operator expected() const noexcept
        {
            return expected{};
        }
    };

    static constexpr null_t null{};

    template <typename V>
    using rebind_value =  std::expected<V, Error>;

    template <typename E>
    using rebind_error = std::expected<Value, E>;

    template <typename E>
        requires std::constructible_from<expected, std::unexpect_t, E&&>
    static constexpr expected from_error(E&& error)
    {
        return expected{std::unexpect, std::forward<E>(error)};
    }
};

#endif
