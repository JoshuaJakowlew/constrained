#pragma once

#include <constrained_type/combinators/static/macro.hpp>

#define CONSTRAINED_TYPE_RELATIONAL() \
    CONSTRAINED_TYPE_BIN_OP(eq,  ==) \
    CONSTRAINED_TYPE_BIN_OP(neq, !=) \
    CONSTRAINED_TYPE_BIN_OP(gt,   >) \
    CONSTRAINED_TYPE_BIN_OP(ge,  >=) \
    CONSTRAINED_TYPE_BIN_OP(lt,   <) \
    CONSTRAINED_TYPE_BIN_OP(le,  <=)

#define CONSTRAINED_TYPE_RELATIONAL_BY_METHOD(method) \
    CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, eq,  ==) \
    CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, neq, !=) \
    CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, gt,   >) \
    CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, ge,  >=) \
    CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, lt,   <) \
    CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, le,  <=)

#define CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_MIN_MAX_ALIAS(method) \
    CONSTRAINED_TYPE_ALIAS(min_##method, method##_ge) \
    CONSTRAINED_TYPE_ALIAS(max_##method, method##_le)

namespace ct
{
    // Comparisons for <value> <op> <static_param>
    CONSTRAINED_TYPE_RELATIONAL();

    // Comparisons for <value.<method>()> <op> <static_param>
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD(size    );
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD(length  );
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD(capacity);

    // Aliases:
    // min_<method> ~ <method>_ge
    // max_<method> ~ <method>_le
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_MIN_MAX_ALIAS(size    )
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_MIN_MAX_ALIAS(length  )
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_MIN_MAX_ALIAS(capacity)
} //namespace ct