#include <string>

#include <doctest/doctest.h>

#include <constrained_type.hpp>

namespace
{
    struct nothrow_helper_t
    {
        constexpr int operator*() const noexcept { return 42; }
        constexpr auto operator<=>(nothrow_helper_t const &) const noexcept = default;
        constexpr operator bool() noexcept { return true; } 
    };

    struct throwing_helper_t
    {
        int operator->() const noexcept(false) { throw std::runtime_error{"Oops!"}; }
        int operator*() const noexcept(false) { throw std::runtime_error{"Oops!"}; }
        constexpr auto operator<=>(throwing_helper_t const &) const noexcept(false) = default;
        constexpr operator bool() noexcept(false) { return true; } 
    };
}

template <>
struct ct::default_traits<nothrow_helper_t>
{
    using value_type = nothrow_helper_t;
    static constexpr bool is_nullable = true;
    static constexpr value_type null = value_type{};
};
template <>
struct ct::default_traits<throwing_helper_t>
{
    using value_type = throwing_helper_t;
    static constexpr bool is_nullable = true;
    static constexpr value_type null = value_type{};
};

TEST_SUITE("Operators")
{
    using non_nullable_t = ct::constrained_type<int>;
    using throwing_t = ct::constrained_type<throwing_helper_t>;
    using nothrow_t = ct::constrained_type<nothrow_helper_t>;

    TEST_CASE("operator bool")
    {
        using explicit_t = ct::basic_constrained_type<
            std::optional<int>,
            ct::default_traits<std::optional<int>>,
            ct::configuration_point{.explicit_bool = true},
            ct::eq<42>
        >;
        using implicit_t = ct::basic_constrained_type<
            std::optional<int>,
            ct::default_traits<std::optional<int>>,
            ct::configuration_point{.explicit_bool = false},
            ct::eq<42>
        >;

        // Nullable concept satisfaction
        static_assert(
            !ct::nullable<non_nullable_t::trait_type>,
            "Non nullable type satisfies nullable concept"
        );
        static_assert(
            ct::nullable<explicit_t::trait_type>,
            "Nullable type doesn't satisfy nullable concept"
        );

        // Noexceptness checks
        static_assert(
            noexcept(nothrow_t{}.operator bool()),
            "operator bool is not noexcept"
        );
        static_assert(
            !noexcept(throwing_t{}.operator bool()),
            "operator bool is noexcept"
        );

        // Explicitness checks      
        static_assert(
            std::is_convertible_v<implicit_t, bool>,
            "Implicitly convertible to bool type can't be implicitly converted"
        );
        static_assert(
            ct::convertible_to<explicit_t, bool> && !std::is_convertible_v<explicit_t, bool>,
            "Explicitly convertible to bool type can be implicitly converted"
        );
        
        CHECK(implicit_t{42} == true);
        CHECK(*implicit_t{42} == std::optional{42});

        CHECK(implicit_t{43} == false);
        CHECK(*implicit_t{43} == implicit_t::trait_type::null);

        CHECK(implicit_t{implicit_t::trait_type::null} == false);
        CHECK(*implicit_t{implicit_t::trait_type::null} == implicit_t::trait_type::null);
    }
    TEST_CASE("access and deref operators")
    {
        using flat_opaque_t = ct::basic_constrained_type<
            int,
            ct::default_traits<int>,
            ct::configuration_point{.transparent_dereferencable = false, .transparent_member_accessible = false, .transparent_pointer_accessible = false},
            ct::eq<42>
        >;
        using deep_opaque_t = ct::basic_constrained_type<
            std::optional<int>,
            ct::default_traits<std::optional<int>>,
            ct::configuration_point{.transparent_dereferencable = false, .transparent_member_accessible = false, .transparent_pointer_accessible = false},
            ct::eq<42>
        >;

        using flat_transparent_t = ct::basic_constrained_type<
            int,
            ct::default_traits<int>,
            ct::configuration_point{.transparent_dereferencable = true, .transparent_member_accessible = true, .transparent_pointer_accessible = true},
            ct::eq<42>
        >;
        using deep_transparent_t = ct::basic_constrained_type<
            std::optional<int>,
            ct::default_traits<std::optional<int>>,
            ct::configuration_point{.transparent_dereferencable = true, .transparent_member_accessible = true, .transparent_pointer_accessible = true},
            ct::eq<42>
        >;

        using deep_throwing_t = ct::basic_constrained_type<
            throwing_helper_t,
            ct::default_traits<throwing_helper_t>,
            ct::configuration_point{.transparent_dereferencable = true, .transparent_member_accessible = true}
        >;

        SUBCASE("operator *")
        {
            // Noecxeptness checks
            static_assert(
                noexcept(std::declval<flat_opaque_t>().operator*()),
                "Opaque operator * is not noexcept"
            );
            static_assert(
                noexcept(std::declval<deep_transparent_t>().operator*()),
                "Transparent operator * is not noexcept"
            );
            static_assert(
                !noexcept(std::declval<deep_throwing_t>().operator*()),
                "Transparent operator * is noexcept"
            );

            // Overaload selection
            static_assert(
                std::same_as<
                    flat_opaque_t::value_type,
                    std::remove_reference_t<decltype(std::declval<flat_opaque_t>().operator*())>
                >,
                "Opaque dereferencing is not working"
            );
            static_assert(
                std::same_as<
                    deep_opaque_t::value_type,
                    std::remove_reference_t<decltype(std::declval<deep_opaque_t>().operator*())>
                >,
                "Opaque dereferencing is not working"
            );

            static_assert(
                std::same_as<
                    flat_transparent_t::value_type,
                    std::remove_reference_t<decltype(std::declval<flat_transparent_t>().operator*())>
                >,
                "Transparent dereferencing is not working"
            );
            static_assert(
                std::same_as<
                    std::remove_reference_t<decltype(std::declval<deep_transparent_t::value_type>().operator*())>,
                    std::remove_reference_t<decltype(std::declval<deep_transparent_t>().operator*())>
                >,
                "Transparent dereferencing is not working"
            );

            CHECK(*flat_opaque_t{42} == 42);
            CHECK_THROWS(*flat_opaque_t{43}); // NOLINT

            CHECK(*deep_opaque_t{42} == deep_opaque_t::value_type{42});
            CHECK(*deep_opaque_t{43} == deep_opaque_t::trait_type::null);

            CHECK(*flat_transparent_t{42} == 42);
            CHECK_THROWS(*flat_transparent_t{43}); // NOLINT
            
            CHECK(*deep_transparent_t{42} == deep_transparent_t::value_type::value_type{42});
            CHECK_THROWS(*deep_throwing_t{}); // NOLINT
        }
        SUBCASE("operator ->")
        {
            using pointer_t = ct::basic_constrained_type<
                int*,
                ct::default_traits<int*>,
                ct::configuration_point{.transparent_dereferencable = true, .transparent_member_accessible = true, .transparent_pointer_accessible = true}
            >;

            // Noecxeptness checks
            static_assert(
                noexcept(std::declval<deep_transparent_t>().operator->()),
                "Transparent operator -> is not noexcept"
            );
            static_assert(
                !noexcept(std::declval<deep_throwing_t>().operator->()),
                "Transparent operator -> is noexcept"
            );

            // Overaload selection
            static_assert(
                std::same_as<
                    flat_opaque_t::value_type *,
                    std::remove_reference_t<decltype(std::declval<flat_opaque_t>().operator->())>
                >,
                "Opaque access is not working"
            );
            static_assert(
                std::same_as<
                    deep_opaque_t::value_type *,
                    std::remove_reference_t<decltype(std::declval<deep_opaque_t>().operator->())>
                >,
                "Opaque access is not working"
            );

            static_assert(
                std::same_as<
                    flat_transparent_t::value_type *,
                    std::remove_reference_t<decltype(std::declval<flat_transparent_t>().operator->())>
                >,
                "Transparent dereferencing is not working"
            );
            static_assert(
                std::same_as<
                    std::remove_reference_t<decltype(std::declval<deep_transparent_t::value_type>().operator->())>,
                    std::remove_reference_t<decltype(std::declval<deep_transparent_t>().operator->())>
                >,
                "Transparent dereferencing is not working"
            );

            static_assert(
                std::same_as<
                    pointer_t::value_type,
                    std::remove_reference_t<decltype(std::declval<pointer_t>().operator->())>
                >,
                "Opaque access is not working"
            );

            CHECK(*(flat_opaque_t{42}.operator->()) == 42);
            CHECK_THROWS(flat_opaque_t{43}); // NOLINT

            CHECK(deep_opaque_t{42}->value() == 42);
            CHECK(deep_opaque_t{43}->has_value() == false);

            CHECK(*(flat_transparent_t{42}.operator->()) == 42);
            CHECK_THROWS(*flat_transparent_t{43}); // NOLINT
            
            CHECK(*(deep_transparent_t{42}.operator->()) == 42);
            CHECK_THROWS(deep_throwing_t{}.operator->()); // NOLINT

            int x = 42;
            CHECK(pointer_t{&x}.operator->() == &x);
        }
    }
}
