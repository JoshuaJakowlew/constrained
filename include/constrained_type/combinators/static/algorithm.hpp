#pragma once

#include <constrained_type/combinators/static/macro.hpp>

#include <algorithm>
#include <iterator>

namespace ct
{
    CONSTRAINED_TYPE_PREDICATE(
        template <auto Value>,
        has,
        std::any_of(std::cbegin(x), std::cend(x), []<typename E>(E&& e) {
            return Value == std::forward<E>(e);
        })
    );

    CONSTRAINED_TYPE_PREDICATE(
        template <auto Value>,
        is,
        std::all_of(std::cbegin(x), std::cend(x), []<typename E>(E&& e) {
            return Value == std::forward<E>(e);
        })
    );

    CONSTRAINED_TYPE_PREDICATE(
        template <auto Value>,
        none,
        std::none_of(std::cbegin(x), std::cend(x), []<typename E>(E&& e) {
            return Value == std::forward<E>(e);
        })
    );

    CONSTRAINED_TYPE_PREDICATE(
        CONSTRAINED_TYPE_NO_TEMPLATE,
        sorted,
        std::is_sorted(std::begin(x), std::end(x))
    );
} // namespace ct
