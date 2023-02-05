#pragma once

#include <concepts>
#include <type_traits>

#include <constrained_type/constrained_type.hpp>

#define CONSTRAINED_TYPE_0_ARY_PREDICATE(name, expr) \
    struct name \
    { \
        [[nodiscard]] constexpr auto operator()(auto const & x) const \
            noexcept(noexcept(expr)) \
        { \
            return expr; \
        } \
    };

// Assumes x as passed runtime parameter and Y as template param pack
#define CONSTRAINED_TYPE_N_ARY_PREDICATE(name, expr) \
    template <auto... Y> \
    CONSTRAINED_TYPE_0_ARY_PREDICATE(name, expr)

// Assumes x as passed runtime parameter and Y as template param
#define CONSTRAINED_TYPE_PREDICATE(name, expr) \
    template <auto Y> \
    CONSTRAINED_TYPE_0_ARY_PREDICATE(name, expr)

#define CONSTRAINED_TYPE_ALIAS(alias, name) \
    template <auto Y> \
    using alias = name<Y>;

#define CONSTRAINED_TYPE_BIN_OP(name, op) \
    CONSTRAINED_TYPE_PREDICATE(name, \
        x op static_cast<std::remove_reference_t<decltype(x)>>(Y) \
    )

#define CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, name, op) \
    CONSTRAINED_TYPE_PREDICATE(method##_##name, \
        x.method() op static_cast<std::remove_reference_t<decltype(x.method())>>(Y) \
    )
