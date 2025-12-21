//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_EXT_STD_SHARED_PTR_HPP
#define GIMO_EXT_STD_SHARED_PTR_HPP

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

    using Pointer = std::shared_ptr<T>;
};

#endif
