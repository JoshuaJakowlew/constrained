#pragma once

#include <concepts>
#include <type_traits>

#define CONSTRAINED_TYPE_PREDICATE(template_decl, name, expr) \
    CONSTRAINED_TYPE_PREDICATE_IMPL(name, expr, template_decl)

#define CONSTRAINED_TYPE_PREDICATE_IMPL(name, expr, ...) \
    namespace detail \
    { \
        __VA_ARGS__ \
        CONSTRAINED_TYPE_PREDICATE_STRUCT(name, expr) \
    } \
    CONSTRAINED_TYPE_PREDICATE_##__VA_OPT__(TEMPLATED_)##CONSTANT(name)

#define CONSTRAINED_TYPE_PREDICATE_STRUCT(name, expr) \
    struct name \
    { \
        [[nodiscard]] constexpr auto operator()(auto const & x) const \
            noexcept(noexcept(expr)) \
        { return expr; } \
    };

#define CONSTRAINED_TYPE_NO_TEMPLATE

#define CONSTRAINED_TYPE_PREDICATE_CONSTANT(name) \
    inline constexpr auto name = detail::name{};

#define CONSTRAINED_TYPE_PREDICATE_TEMPLATED_CONSTANT(name) \
    template <auto... Args> \
    inline constexpr auto name = detail::name<Args...>{};

#define CONSTRAINED_TYPE_ALIAS(alias, name) \
    namespace detail \
    { \
        using alias = name; \
    } \
    \
    inline constexpr auto alias = detail::alias{};

#define CONSTRAINED_TYPE_TEMPLATED_ALIAS(alias, name) \
    namespace detail \
    { \
        template <auto... Xs> \
        using alias = name<Xs...>; \
    } \
    \
    template <auto... Xs> \
    inline constexpr auto alias = detail::alias<Xs...>{};

namespace ct
{
    CONSTRAINED_TYPE_PREDICATE(template <auto N>, add, x + N);
    CONSTRAINED_TYPE_PREDICATE(CONSTRAINED_TYPE_NO_TEMPLATE, add42, x + 42);
}