#pragma once

#include <concepts>
#include <iostream> //FIXME: Only for testing
#include <type_traits>
#include <functional>

namespace ct
{
    /*
     * We want to distinguish combinators from arbitrary callable objects 
     * Each combinator must define type combinator_tag inside it
     */
    // TODO: Check for operator()?
    template <typename T>
    concept combinator = requires(T x)
    {
        typename T::combinator_tag;
    };

    /*
     * We want to make sure, that lifted value is wrapped correctly
     * Also this unties us from val<V>
     */
    // TODO: Check for static constexpr value?
    // TODO: Better name?
    template <typename T>
    concept unlifting = requires(T x)
    {
        typename T::unlifting_tag;
    };

    /*
     * Checks if type-level value is callable with argument of type X
     * ValF must be val<V> because concepts doesn't allows non-type template parameters
     * So, we must hide value V inside a type and then extract it.
     */
    template <typename ValF, typename... Xs>
    concept callable_with =
        unlifting<ValF>
        && std::regular_invocable<decltype(ValF::value), Xs...>;

    /*
     * Combination of callable_with and combinator concepts
     */
    template <typename ValF, typename... Xs>
    concept combinator_with =
        unlifting<ValF>
        && callable_with<ValF, Xs...>
        && combinator<decltype(ValF::value)>;

    namespace detail
    {
        /*
        * Helper type that allows unlifting of V
        */
        template <auto V>
        struct [[nodiscard]] val final
        {
            using combinator_tag = void;
            using unlifting_tag = void;

            static constexpr auto value = V;

            [[nodiscard]] constexpr auto operator()(auto const & x) const noexcept
            { return value; }
        };
    }
    template <auto V> \
    inline constexpr auto val = detail::val<V>{};

    /*
     * Unifies usage of different value categories passed as non-type template parameter
     *
     * Maps V to val<...>
     *
     * ====================================================================
     * Value of V                   | return type
     * ====================================================================
     * combinator                   | val<V>
     * --------------------------------------------------------------------
     * callable with x object       | val<V>
     * --------------------------------------------------------------------
     * callable with no params      | val<[](const auto & x) { return V; }>
     * (e.g. wrapped runtime value) |
     * --------------------------------------------------------------------
     * othewrwise                   |
     * (e.g. simple value)          | val<[](const auto & x) { return V; }>
     * --------------------------------------------------------------------
     *
     * This class is usable only in unevaluated context
     * It's constructor is deleted and get methods cannot be called
     *
     * Example use:
     * using my_param_t = decltype(param<NonTypeParam>::get(x));
     * auto value = my_param::value(x);
     */

    // FIXME: Make get methods consteval with runtime code inside as intended.
    // Constexpr is used for testing only.
    template <auto V>
    struct param final
    {
        param() = delete;

        // Specialization for combinators
        static constexpr auto get(auto const & x) noexcept
            requires combinator<decltype(V)>
        {
            std::cout << "Combinator\n";
            return val<V>;
        }

        // Specialization for callable with x objects
        static constexpr auto get(auto const & x) noexcept
            requires callable_with<detail::val<V>, decltype(x)> && (!combinator<decltype(V)>)
        {
            std::cout << "Semi-combinator\n";
            return val<V>;
        }

        // Specialization for wrapped runtime values
        static constexpr auto get(auto const & x) noexcept
            requires requires() { V(); }
        {
            std::cout << "Callable\n";
            return val<
                [] [[nodiscard]] (const auto &) noexcept(noexcept(V())) { return V(); }
            >;
        }

        // Specialization for everything else (e.g. simple values)
        static constexpr auto get(auto const & x) noexcept
        {
            std::cout << "Value\n";
            return val<
                [] [[nodiscard]] (const auto & x) noexcept { return V; }
            >;
        }

        // Extracts callable from type level
        static constexpr auto value(auto const & x) noexcept
        {
            return decltype(get(x))::value;
        }

        // Extracts callable from type level and calls it with x
        static constexpr auto apply(auto const & x)
            noexcept(noexcept(std::invoke(value(x), x)))
        {
            // return value(x)(x);
            return std::invoke(value(x), x);
        }
    };

    /*
     * Utility function to reduce typing. Equivalent to param<V>::value(x)
     */
    template <auto V>
    [[nodiscard]] constexpr auto value(auto const & x)
        noexcept(noexcept(param<V>::value(x)))
    {
        return param<V>::value(x);
    }

    /*
     * Utility function to reduce typing. Equivalent to param<V>::apply(x)
     */
    template <auto V>
    [[nodiscard]] constexpr auto apply(auto const & x)
        noexcept(noexcept(param<V>::apply(x)))
    {
        return param<V>::apply(x);
    }
} // namespace ct
