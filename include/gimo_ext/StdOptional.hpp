//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_EXT_STD_OPTIONAL_HPP
#define GIMO_EXT_STD_OPTIONAL_HPP

#pragma once

#include <optional>

namespace gimo
{
    template <typename T>
    struct traits;
}

template <typename T>
struct gimo::traits<std::optional<T>>
{
    static constexpr auto null{std::nullopt};

    template <typename V>
    using rebind_value =  std::optional<V>;
};

#endif
