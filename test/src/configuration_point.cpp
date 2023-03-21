#include <optional>
#include <string>

#include <doctest/doctest.h>

#include <constrained_type.hpp>

TEST_SUITE("Operators") {
    TEST_CASE("operator bool") {
        using constrained = ct::basic_constrained_type<
            std::optional<int>,
            ct::default_traits<std::optional<int>>,
            ct::configuration_point{
                false,
                true,
                false,
                true,
                true
            },
            ct::eq<42>
        >;

        CHECK(constrained{42} == true);
        CHECK(constrained{43} == false);
        CHECK(*constrained{42} == std::optional{42});
        CHECK(*constrained{43} == std::nullopt);
        CHECK(*constrained{43} == ct::default_traits<std::optional<int>>::null);
    }

    TEST_CASE("access operators (non-opaque)") {
        constexpr auto config = ct::configuration_point{
            true,
            true,
            false,
            false,
            false
        };
        
        using flat = ct::basic_constrained_type<
            int,
            ct::default_traits<int>,
            config
        >;

        using ptr = ct::basic_constrained_type<
            int*,
            ct::default_traits<int*>,
            config
        >;

        using opt = ct::basic_constrained_type<
            std::optional<std::string>,
            ct::default_traits<std::optional<std::string>>,
            config
        >;

        int x = 42;

        CHECK(*flat{42} == 42);
        CHECK(*ptr{nullptr} == nullptr);
        CHECK(*ptr{&x} == &x);

        CHECK(opt{"abc"}->has_value() == true);
        CHECK(opt{"abc"}->value().length() == 3);
        CHECK(static_cast<void*>(ptr{&x}.operator->()) != static_cast<void*>(&x));
    }

    TEST_CASE("access operators (opaque)") {
        constexpr auto config = ct::configuration_point{
            true,
            true,
            true,
            true,
            true
        };
        
        using flat = ct::basic_constrained_type<
            int,
            ct::default_traits<int>,
            config
        >;

        using ptr = ct::basic_constrained_type<
            int*,
            ct::default_traits<int*>,
            config
        >;

        using opt = ct::basic_constrained_type<
            std::optional<std::string>,
            ct::default_traits<std::optional<std::string>>,
            config
        >;

        int x = 42;

        CHECK(*flat{42} == 42);
        CHECK(*ptr{&x} == x);

        CHECK(opt{"abc"}->size() == 3);
        CHECK(ptr{&x}.operator->() == &x);
    }
}
