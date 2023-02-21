#pragma once

#include <constrained_type/combinators/static/macro.hpp>

namespace ct
{
    CONSTRAINED_TYPE_PREDICATE(template <auto F>, neg, !F(x));
    [[nodiscard]] constexpr auto operator!(auto x) noexcept
    {
        return detail::neg<x>{};
    }

    CONSTRAINED_TYPE_PREDICATE(template <auto... Fs>, any, (Fs(x) || ...));
    [[nodiscard]] constexpr auto operator||(auto x, auto y) noexcept
    {
        return detail::any<x, y>{};
    }

    CONSTRAINED_TYPE_PREDICATE(template <auto... Fs>, all, (Fs(x) && ...));
    [[nodiscard]] constexpr auto operator&&(auto x, auto y) noexcept
    {
        return detail::all<x, y>{};
    }
} // namespace ct
