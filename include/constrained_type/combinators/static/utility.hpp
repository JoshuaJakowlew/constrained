#pragma once

#include <constrained_type/combinators/static/comparisons.hpp>
#include <constrained_type/combinators/static/logical.hpp>
#include <constrained_type/combinators/static/algorithm.hpp>

#define CONSTRAINED_TYPE_CONTAINS_BY_METHOD(method) \
    CONSTRAINED_TYPE_PREDICATE_IMPL( \
        method##_between, \
        (ct::method##_ge<Low> && ct::method##_lt<High>)(x), \
        template <auto Low, auto High> \
    )

namespace ct
{
    CONSTRAINED_TYPE_PREDICATE_IMPL(
        between,
        (ct::ge<Low> && ct::lt<High>)(x),
        template <auto Low, auto High>
    );

    CONSTRAINED_TYPE_CONTAINS_BY_METHOD(size    );
    CONSTRAINED_TYPE_CONTAINS_BY_METHOD(length  );
    CONSTRAINED_TYPE_CONTAINS_BY_METHOD(capacity);

    CONSTRAINED_TYPE_PREDICATE(
        template <auto... Xs>,
        one_of,
        ((ct::has<Xs> || ...))(x)
    );
} // namespace ct
