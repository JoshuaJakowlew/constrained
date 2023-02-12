#pragma once

#include <concepts>
#include <type_traits>

#include <constrained_type/constrained_type.hpp>
// #include <constrained_type/combinators/static/rank2.hpp>

namespace ct
{
    template <typename T>
    concept combinator = requires()
    {
        typename T::combinator_tag;
    };
}

#define CONSTRAINED_TYPE_MAKE_PREDICATE(args, name, expr) \
    namespace detail \
    { \
        args \
        struct name \
        { \
            using combinator_tag = void;\
            \
            [[nodiscard]] constexpr auto operator()(auto const & x) const \
                noexcept(noexcept(expr)) \
            { return expr; } \
            \
            [[nodiscard]] constexpr auto operator!() const noexcept \
            { return neg<decltype(*this){}>{}; } \
        }; \
    } \
    \
    template <auto... Args> \
    inline static constexpr auto name = detail::name<Args...>{};

#define CONSTRAINED_TYPE_0_ARY_PREDICATE(name, expr) \
    CONSTRAINED_TYPE_MAKE_PREDICATE(template <int Dummy = 42>, name, expr)

// Assumes x as passed runtime parameter and Y as template param pack
#define CONSTRAINED_TYPE_N_ARY_PREDICATE(name, expr) \
    CONSTRAINED_TYPE_MAKE_PREDICATE(template <auto... Y>, name, expr)

// Assumes x as passed runtime parameter and Y as template param
#define CONSTRAINED_TYPE_PREDICATE(name, expr) \
    CONSTRAINED_TYPE_MAKE_PREDICATE(template <auto Y>, name, expr)

#define CONSTRAINED_TYPE_ALIAS(alias, name) \
    namespace detail \
    { \
        template <auto Y> \
        using alias = name<Y>; \
    } \
    \
    template <auto... Args> \
    inline static constexpr auto alias = detail::alias<Args...>{};

#define CONSTRAINED_TYPE_BIN_OP(name, op) \
    CONSTRAINED_TYPE_PREDICATE(name, \
        x op static_cast<std::remove_reference_t<decltype(x)>>(Y) \
    )

#define CONSTRAINED_TYPE_BIN_OP_BY_METHOD(method, name, op) \
    CONSTRAINED_TYPE_PREDICATE(method##_##name, \
        x.method() op static_cast<std::remove_reference_t<decltype(x.method())>>(Y) \
    )
