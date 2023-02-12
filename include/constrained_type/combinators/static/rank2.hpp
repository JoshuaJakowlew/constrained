#pragma once

#include <constrained_type/combinators/static/macro.hpp>

#define CONSTRAINED_TYPE_RANK2_FUNC(name, expr) \
    CONSTRAINED_TYPE_MAKE_PREDICATE(template <combinator auto F>, name, expr)

#define CONSTRAINED_TYPE_RANK2_BINARY_FUNC(name, expr) \
    CONSTRAINED_TYPE_MAKE_PREDICATE(template <combinator auto F1, combinator auto F2>, name, expr)

#define CONSTRAINED_TYPE_RANK2_N_ARY_FUNC(name, expr) \
    CONSTRAINED_TYPE_MAKE_PREDICATE(template <combinator auto... F>, name, expr)

#define CONSTRAINED_TYPE_RANK2_NEG() \
    CONSTRAINED_TYPE_RANK2_FUNC(neg, !F(x))

#define CONSTRAINED_TYPE_RANK2_CHOICE() \
    CONSTRAINED_TYPE_RANK2_N_ARY_FUNC(choice, (F(x) || ...))

namespace ct
{
    CONSTRAINED_TYPE_RANK2_NEG();
    CONSTRAINED_TYPE_RANK2_CHOICE();
} // namespace ct