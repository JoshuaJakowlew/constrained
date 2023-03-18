#pragma once

#include <constrained_type/combinators/core.hpp>

#define CONSTRAINED_TYPE_COMBINATOR_CONSTANT(name) \
    inline constexpr auto name = detail::name{};

#define CONSTRAINED_TYPE_TEMPLATED_COMBINATOR_CONSTANT(name) \
    template <auto... Args> \
    inline constexpr auto name = detail::name<Args...>{};

#define CONSTRAINED_TYPE_BINARY_OPERATOR(name, op) \
    [[nodiscard]] consteval auto operator op (combinator auto x, combinator auto y) noexcept \
    { \
        return name<x, y>; \
    }

#define CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(name, op) \
    namespace detail \
    { \
        template <auto... Args> \
        struct [[nodiscard]] name final \
        { \
            using combinator_tag = void; \
            [[nodiscard]] constexpr auto operator()(auto const & x) const \
                noexcept(noexcept((apply<Args>(x) op ...))) \
            { return (apply<Args>(x) op ...); } \
        }; \
    \
        template <auto A> \
        struct [[nodiscard]] name<A> final \
        { \
            using combinator_tag = void; \
            \
            [[nodiscard]] constexpr auto operator()(auto const & x) const \
                noexcept(noexcept(x op apply<A>(x))) \
            { return x op apply<A>(x); } \
        }; \
    } \
    CONSTRAINED_TYPE_TEMPLATED_COMBINATOR_CONSTANT(name) \
    CONSTRAINED_TYPE_BINARY_OPERATOR(name, op)

#define CONSTRAINED_TYPE_UNARY_OPERATOR(name, op) \
    [[nodiscard]] consteval auto operator op (combinator auto x) noexcept \
    { \
        return name<x>; \
    }

#define CONSTRAINED_TYPE_UNARY_OPERATOR_COMBINATOR(name, op) \
    namespace detail \
    { \
        template <auto A> \
        struct [[nodiscard]] name final \
        { \
            using combinator_tag = void; \
            \
            [[nodiscard]] constexpr auto operator()(auto const & x) const \
                noexcept(noexcept(op apply<A>(x))) \
            { return op apply<A>(x); } \
        }; \
    } \
    CONSTRAINED_TYPE_TEMPLATED_COMBINATOR_CONSTANT(name) \
    CONSTRAINED_TYPE_UNARY_OPERATOR(name, op)

namespace ct
{
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(eq,  ==);
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(neq, !=);
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(gt,   >);
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(ge,  >=);
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(lt,   <);
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(le,  <=);

    CONSTRAINED_TYPE_UNARY_OPERATOR_COMBINATOR (not_,   !);
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(and_,  &&);
    CONSTRAINED_TYPE_BINARY_OPERATOR_COMBINATOR(or_,   ||);

    namespace detail
    {
        template <auto Cond, auto WhenTrue, auto WhenFalse>
        struct [[nodiscard]] if_ final
        {
            using combinator_tag = void;
           
            [[nodiscard]] constexpr auto operator()(auto const & x) const
                noexcept(noexcept([](auto const & x){
                    if (apply<Cond>(x))
                        return apply<WhenTrue>(x);
                    else
                        return apply<WhenFalse>(x);
                }(x)))
            {
                return [](auto const & x){
                    if (apply<Cond>(x))
                        return apply<WhenTrue>(x);
                    else
                        return apply<WhenFalse>(x);
                }(x);
            }
        };
    }
    CONSTRAINED_TYPE_TEMPLATED_COMBINATOR_CONSTANT(if_);
} // namespace ct