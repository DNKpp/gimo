//          Copyright Dominic (DNKpp) Koepke 2025.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef GIMO_EXT_STD_UNIQUE_PTR_HPP
#define GIMO_EXT_STD_UNIQUE_PTR_HPP

#include <memory>

namespace gimo
{
    template <typename T>
    struct traits;
}

template <typename T, typename Deleter>
    requires(!std::is_array_v<T>)
struct gimo::traits<std::unique_ptr<T, Deleter>>
{
    static constexpr std::nullptr_t null{};

    using Pointer = std::unique_ptr<T, Deleter>;

    [[nodiscard]]
    static constexpr std::add_lvalue_reference_t<T> value(Pointer const& ptr) noexcept(noexcept(*ptr))
    {
        return *ptr;
    }

    [[nodiscard]]
    static constexpr std::add_rvalue_reference_t<T> value(Pointer&& ptr) noexcept(noexcept(*ptr))
    {
        return std::move(*ptr);
    }
};

#endif
