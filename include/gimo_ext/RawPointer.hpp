//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_EXT_RAW_POINTER_HPP
#define GIMO_EXT_RAW_POINTER_HPP

#pragma once

#include <cstddef>

namespace gimo
{
    template <typename T>
    struct traits;
}

template <typename T>
struct gimo::traits<T*>
{
    static constexpr std::nullptr_t null{};
};

#endif
