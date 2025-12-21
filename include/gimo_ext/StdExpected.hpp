
//           Copyright Dominic (DNKpp) Koepke 2025.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_EXT_STD_EXPECTED_HPP
#define GIMO_EXT_STD_EXPECTED_HPP

#pragma once

#include "gimo/Common.hpp"

#include <expected>

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
    static constexpr std::expected<Value, std::remove_cvref_t<E>> from_error(E&& error)
    {
        return std::unexpected{std::forward<E>(error)};
    }
};

#endif
