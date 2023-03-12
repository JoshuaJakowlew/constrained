#pragma once

#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <optional>
#include <functional>

#include <constrained_type/value_pack.hpp>

namespace ct
{
    template <typename Lambda, int = (Lambda{}(), 0)>
    constexpr bool is_constexpr(Lambda) { return true;  }
    constexpr bool is_constexpr(...)    { return false; }

    template <typename T>
    concept constrained_trait = requires()
    {
        typename T::value_type;

        requires std::same_as<decltype(T::is_nullable), const bool>;
        requires is_constexpr([]{ T::is_nullable; });

        // If T::is_nullable is false - OK, no additional checks
        // Otherwise apply requirements for T::null
        requires (not T::is_nullable ? true : requires {
            requires std::same_as<decltype(T::null), const typename T::value_type>;
            requires is_constexpr([]{ T::null; });
        });
    };

    template <typename Trait>
    concept nullable = 
        constrained_trait<Trait>
        and Trait::is_nullable;

    template <typename Trait>
    concept nothrow_null_constructible = 
        nullable<Trait>
        and noexcept(typename Trait::value_type{Trait::null});

    template <typename F, typename T>
    concept nothrow_predicate = 
        std::predicate<F, T>
        and std::is_nothrow_invocable_v<F, T>;

    template <typename From, typename To>
    concept convertible_to = requires
    {
        static_cast<To>(std::declval<From>());
    };

    template <typename T>
    concept dereferenceable = requires (T x) { *x; };

    template <typename T>
    concept nothrow_dereferenceable =
        dereferenceable<T>
        and noexcept(*std::declval<T>());

    template <typename T>
    concept member_accessible = requires (T x) { x.operator->(); };

    template <typename T>
    concept nothrow_member_accessible = 
        member_accessible<T>
        and noexcept(std::declval<T>().operator->());

    template <typename T>
    struct default_traits
    {
        using value_type = T;
        static constexpr bool is_nullable = false;
    };

    template <typename T>
    struct default_traits<std::optional<T>>
    {
        using value_type = std::optional<T>;
        static constexpr bool is_nullable = true;
        static constexpr value_type null = std::nullopt;
    };

    struct configuration_point
    {
        bool explicit_bool = true;
        bool explicit_forwarding_constructor = true;
        bool opaque_dereferencable = true;
        bool opaque_member_accessible = true;
        bool opaque_pointer_accessible = true;
    };

    template <typename T, constrained_trait Trait, configuration_point Config, auto... Constraints>
        requires (std::predicate<decltype(Constraints), T const &> && ...)
            and std::same_as<T, typename Trait::value_type>
    class basic_constrained_type;
    
    namespace detail
    {
        template <typename T, constrained_trait Trait, configuration_point Config, auto... Constraints>
        consteval auto is_constrained(
            basic_constrained_type<T, Trait, Config, Constraints...>*
        ) -> std::true_type;

        consteval auto is_constrained(auto*) -> std::false_type;
    }

    template <typename T>
    concept constrained = std::same_as<
        decltype(detail::is_constrained(static_cast<T*>(nullptr))),
        std::true_type
    >;

    namespace detail
    {
        template <constrained ConstrainedType, auto... Constraints>
        consteval auto set_constraint_pack(
            value_pack<Constraints...>*
        ) -> basic_constrained_type<
            typename ConstrainedType::value_type,
            typename ConstrainedType::trait_type,
            ConstrainedType::config_value,
            Constraints...
        >;
    }

    template <constrained ConstrainedType, typename ConstraintPack>
    using set_constraint_pack = decltype(
        detail::set_constraint_pack<ConstrainedType>(
            static_cast<ConstraintPack*>(nullptr)
        )
    );

    template <constrained ConstrainedType, auto... Constraints>
    using set_constraints = set_constraint_pack<ConstrainedType, value_pack<Constraints...>>;

    template <constrained ConstrainedType, typename ConstraintPack>
    using add_constraint_pack = decltype(
        detail::set_constraint_pack<ConstrainedType>(
            static_cast<
                typename ConstrainedType::constraints_pack::template add_pack<ConstraintPack>*
            >(nullptr)
        )
    );

    template <constrained ConstrainedType, auto... Constraints>
    using add_constraints = add_constraint_pack<ConstrainedType, value_pack<Constraints...>>;

    template <constrained ConstrainedType>
    using clear_constraints = basic_constrained_type<
        typename ConstrainedType::value_type,
        typename ConstrainedType::trait_type,
        ConstrainedType::config_value
    >;

    template <typename T, constrained_trait Trait, configuration_point Config, auto... Constraints>
        requires (std::predicate<decltype(Constraints), T const &> && ...)
            and std::same_as<T, typename Trait::value_type>
    class basic_constrained_type
    {
    public:
#pragma region type_manipulations
        using value_type = T;
        using trait_type = Trait;
        static constexpr auto config_value = Config;
        using constraints_pack = value_pack<Constraints...>;
        using self_type = basic_constrained_type<T, Trait, Config, Constraints...>;

        template <typename ConstraintPack>
        using set_constraint_pack = ct::set_constraint_pack<self_type, ConstraintPack>;

        template <auto... NewConstraints>
        using set_constraints = ct::set_constraints<self_type, NewConstraints...>;

        template <typename ConstraintPack>
        using add_constraint_pack = ct::add_constraint_pack<self_type, ConstraintPack>;

        template <auto... AdditionalConstraints>
        using add_constraints = ct::add_constraints<self_type, AdditionalConstraints...>;

        using clear_constraints = ct::clear_constraints<self_type>;
#pragma endregion type_manipulations

#pragma region constructors
        constexpr basic_constrained_type() noexcept(
            std::is_nothrow_default_constructible_v<T>
            and noexcept(check())
        )
            requires std::is_default_constructible_v<T>
            : _value{}
        { check(); }

        template <typename... Args>
        constexpr explicit(Config.explicit_forwarding_constructor) basic_constrained_type(Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>
            and noexcept(check())
        )
            requires std::is_constructible_v<T, Args...>
            : _value{std::forward<Args>(args)...}
        { check(); }

        basic_constrained_type(basic_constrained_type const & other) noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires std::is_copy_constructible_v<T>
            : _value{other._value}
        {}

        constexpr basic_constrained_type(basic_constrained_type && other) noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_move_constructible_v<T>
            : _value{std::move(other._value)}
        {}
#pragma endregion constructors

#pragma region assignments
        constexpr auto operator=(basic_constrained_type const & other) noexcept(std::is_nothrow_copy_constructible_v<T>) -> basic_constrained_type &
            requires std::is_copy_constructible_v<T>
        {
            _value = other._value;
            return *this;
        }

        constexpr auto operator=(basic_constrained_type && other) noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_move_constructible_v<T>
        {
            _value = std::move(other._value);
            return *this;
        }
#pragma endregion

        [[nodiscard]] constexpr explicit(Config.explicit_bool) operator bool() const noexcept(
            noexcept(static_cast<bool>(_value == Trait::null))
        )
            requires nullable<Trait>
        {
            return static_cast<bool>(_value == Trait::null);
        }

#pragma region dereference_operators
        [[nodiscard]] constexpr decltype(auto) operator*() const & noexcept(nothrow_dereferenceable<T>)
            requires dereferenceable<T> and (Config.opaque_dereferencable)
        { return *_value; }

        [[nodiscard]] constexpr decltype(auto) operator*() && noexcept(nothrow_dereferenceable<T>)
            requires dereferenceable<T> and (Config.opaque_dereferencable)
        { return *std::move(_value); }

        [[nodiscard]] constexpr decltype(auto) operator*() const && noexcept(nothrow_dereferenceable<T>)
            requires dereferenceable<T> and (Config.opaque_dereferencable)
        { return *std::move(_value); }

        [[nodiscard]] constexpr auto operator*() const & noexcept -> const T&
        { return _value; }

        [[nodiscard]] constexpr auto operator*() && noexcept -> T&&
        { return std::move(_value); }

        [[nodiscard]] constexpr auto operator*() const && noexcept -> const T&&
        { return std::move(_value); }
#pragma endregion dereference_operators

#pragma region access_operators
        // Member accessible overloads
        [[nodiscard]] constexpr decltype(auto) operator->() const & noexcept(nothrow_member_accessible<T>)
            requires member_accessible<T> and (Config.opaque_member_accessible)
        { return _value.operator->(); }

        [[nodiscard]] constexpr decltype(auto) operator->() && noexcept(nothrow_member_accessible<T>)
            requires member_accessible<T> and (Config.opaque_member_accessible)
        { return std::move(_value).operator->(); }

        [[nodiscard]] constexpr decltype(auto) operator->() const && noexcept(nothrow_member_accessible<T>)
            requires member_accessible<T> and (Config.opaque_member_accessible)
        { return std::move(_value).operator->(); }

        // Pointer overloads
        [[nodiscard]] constexpr decltype(auto) operator->() const & noexcept
            requires std::is_pointer_v<T> and (Config.opaque_pointer_accessible)
        { return _value; }

        [[nodiscard]] constexpr decltype(auto) operator->() && noexcept
            requires std::is_pointer_v<T> and (Config.opaque_pointer_accessible)
        { return std::move(_value); }

        [[nodiscard]] constexpr decltype(auto) operator->() const && noexcept
            requires std::is_pointer_v<T> and (Config.opaque_pointer_accessible)
        { return std::move(_value); }

        // Fallback overloads
        [[nodiscard]] constexpr auto operator->() const & noexcept -> const T*
        { return &_value; }

        [[nodiscard]] constexpr auto operator->() && noexcept -> T*
        { return &_value; }

        [[nodiscard]] constexpr auto operator->() const && noexcept -> const T*
        { return &_value; }
#pragma endregion access_operators
    
    private:
        T _value;

        constexpr void check() noexcept(
            (nothrow_predicate<decltype(Constraints), T const &> && ...)
            and noexcept(fail())
        )
        {
            bool satisfied = (std::invoke(Constraints, _value) && ...);
            if (!satisfied)
            {
                fail();
            }
        }

        constexpr void fail() noexcept(nothrow_null_constructible<Trait>)
            requires nullable<Trait>
        {
            _value = Trait::null;
        }

        [[noreturn]] constexpr void fail() const
            requires (not nullable<Trait>)
        {
            throw std::runtime_error{"Constraints not satisfied"};
        }
    };

    template <typename T, auto... Constraints>
    using constrained_type = basic_constrained_type<T, default_traits<T>, configuration_point{}, Constraints...>;
} // namespace ct
