#pragma once

#include <constrained_type/combinators/static/macro.hpp>

#include <algorithm>
#include <iterator>

#define CONSTRAINED_TYPE_ALL() \
    CONSTRAINED_TYPE_PREDICATE(all, \
        std::all_of(std::begin(x), std::end(x), []<typename E>(E&& e) { return Y == std::forward<E>(e); }) \
    )

#define CONSTRAINED_TYPE_ANY() \
    CONSTRAINED_TYPE_PREDICATE(any, \
        std::any_of(std::begin(x), std::end(x), []<typename E>(E&& e) { return Y == std::forward<E>(e); }) \
    )

#define CONSTRAINED_TYPE_NONE() \
    CONSTRAINED_TYPE_PREDICATE(none, \
        std::none_of(std::begin(x), std::end(x), []<typename E>(E&& e) { return Y == std::forward<E>(e); }) \
    )

#define CONSTRAINED_TYPE_SORTED() \
    CONSTRAINED_TYPE_0_ARY_PREDICATE(sorted, \
        std::is_sorted(std::begin(x), std::end(x)) \
    )

namespace ct
{
    CONSTRAINED_TYPE_ALL();
    
    CONSTRAINED_TYPE_ANY();
    CONSTRAINED_TYPE_ALIAS(has, any);
    
    CONSTRAINED_TYPE_NONE();

    CONSTRAINED_TYPE_SORTED();
} // namespace ct;
