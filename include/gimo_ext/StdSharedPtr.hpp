//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_EXT_STD_SHARED_PTR_HPP
#define GIMO_EXT_STD_SHARED_PTR_HPP

#pragma once

#include <memory>

namespace gimo
{
    template <typename T>
    struct traits;
}

template <typename T>
    requires(!std::is_array_v<T>)
struct gimo::traits<std::shared_ptr<T>>
{
    static constexpr std::nullptr_t null{};

    template <typename V>
    using rebind_value = std::shared_ptr<V>;

    template <typename Arg>
        requires std::constructible_from<T, Arg&&>
    [[nodiscard]]
    static constexpr std::shared_ptr<T> from_value(Arg&& arg)
    {
        return std::make_shared<T>(std::forward<Arg>(arg));
    }
};

#endif
