#pragma once

#include <nanobench.h>
#include <constrained_type.hpp>

namespace nb = ankerl::nanobench;
inline nb::Rng rng;

namespace manual::nullable::non_null
{
    template <typename T>
    auto dereference(T *x)
    {
        if (x != nullptr)
            return *x;
        else
            return T{};
    }

    template <typename T>
    void run()
    {
        T x{};
        nb::doNotOptimizeAway(
            dereference<T>(rng() % 2 ? &x : nullptr)
        );
    }
}

namespace manual::throwing::non_null
{
    template <typename T>
    auto dereference(T *x)
    {
        if (x != nullptr)
            return *x;
        else
            throw std::logic_error{"Constraints not satisfied"};
    }

    template <typename T>
    void run()
    {
        try {
            T x{};
            nb::doNotOptimizeAway(
                dereference<T>(rng() % 2 ? &x : nullptr)
            );
        } catch(std::logic_error const &) {}
    }
}

namespace constrained::nullable::non_null
{
    template <typename T>
    struct traits
    {
        using value_type = T;
        static constexpr bool is_nullable = true;
        static constexpr value_type null = nullptr;
    };

    template <typename T>
    using non_null = ct::basic_constrained_type<T*, traits<T*>, ct::configuration_point{},
        ct::neq<nullptr>
    >;

    template <typename T>
    auto dereference(non_null<T> x)
    {
        if (x)
            return *x;
        else
            return T{};
    }

    template <typename T>
    void run()
    {
        T x{};
        nb::doNotOptimizeAway(
            dereference<T>(non_null<T>{rng() % 2 ? &x : nullptr})
        );
    }
}

namespace constrained::throwing::non_null
{
    template <typename T>
    struct traits
    {
        using value_type = T;
        static constexpr bool is_nullable = false;
    };

    template <typename T>
    using non_null = ct::basic_constrained_type<T*, traits<T*>, ct::configuration_point{},
        ct::neq<nullptr>
    >;

    template <typename T>
    auto dereference(non_null<T> x)
    {
        return *x;
    }

    template <typename T>
    void run()
    {
        try {
            T x{};
            nb::doNotOptimizeAway(
                dereference<T>(non_null<T>{rng() % 2 ? &x : nullptr})
            );
        } catch(std::logic_error const &) {}
    }
}
