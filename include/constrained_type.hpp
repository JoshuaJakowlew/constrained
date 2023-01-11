#pragma once

#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <optional>

namespace ct
{
    template <typename T>
    struct constrained_traits
    {
        static constexpr bool is_nullable = false;
    };

    template <typename T>
    struct constrained_traits<std::optional<T>>
    {
        static constexpr bool is_nullable = true;
        static constexpr std::optional<T> null = std::nullopt;
    };

    template <typename T>
    concept nullable = constrained_traits<T>::is_nullable;

    template <typename T>
    concept nothrow_null_constructible = 
        nullable<T>
        and noexcept(T{constrained_traits<T>::null});

    template <typename F, typename T>
    concept predicate = std::predicate<F, T const &>;

    template <typename F, typename T>
    concept nothrow_predicate = 
        predicate<F, T>
        and std::is_nothrow_invocable_v<F, T const &>;

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

    template <typename T, auto... Constraints>
        requires (predicate<decltype(Constraints), T const &> && ...)
    class constrained_type
    {
    public:
        constexpr constrained_type() noexcept(
            std::is_nothrow_default_constructible_v<T>
            and noexcept(check())
        )
            requires std::is_default_constructible_v<T>
            : _value{}
        { check(); }

        template <typename... Args>
        constexpr constrained_type(Args&&... args) noexcept(
            std::is_nothrow_constructible_v<T, Args...>
            and noexcept(check())
        )
            requires std::is_constructible_v<T, Args...>
            : _value{std::forward<Args>(args)...}
        { check(); }

        constrained_type(constrained_type const & other) noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires std::is_copy_constructible_v<T>
            : _value{other._value}
        {}

        constexpr constrained_type(constrained_type && other) noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_move_constructible_v<T>
            : _value{std::move(other._value)}
        {}

        [[nodiscard]] constexpr operator bool() const noexcept(
            noexcept(static_cast<bool>(_value))
        )
            requires nullable<T> and convertible_to<T, bool>
        {
            return static_cast<bool>(_value);
        }

        // Dereference operators

        [[nodiscard]] constexpr decltype(auto) operator*() const & noexcept(nothrow_dereferenceable<T>)
            requires dereferenceable<T>
        { return *_value; }

        [[nodiscard]] constexpr decltype(auto) operator*() && noexcept(nothrow_dereferenceable<T>)
            requires dereferenceable<T>
        { return *std::move(_value); }

        [[nodiscard]] constexpr decltype(auto) operator*() const && noexcept(nothrow_dereferenceable<T>)
            requires dereferenceable<T>
        { return *std::move(_value); }

        [[nodiscard]] constexpr auto operator*() const & noexcept -> const T&
        { return _value; }

        [[nodiscard]] constexpr auto operator*() && noexcept -> T&&
        { return std::move(_value); }

        [[nodiscard]] constexpr auto operator*() const && noexcept -> const T&&
        { return std::move(_value); }

        // Member acces operators

        [[nodiscard]] constexpr decltype(auto) operator->() const & noexcept(nothrow_member_accessible<T>)
            requires member_accessible<T>
        { return _value.operator->(); }

        [[nodiscard]] constexpr decltype(auto) operator->() && noexcept(nothrow_member_accessible<T>)
            requires member_accessible<T>
        { return std::move(_value).operator->(); }

        [[nodiscard]] constexpr decltype(auto) operator->() const && noexcept(nothrow_member_accessible<T>)
            requires member_accessible<T>
        { return std::move(_value).operator->(); }

        [[nodiscard]] constexpr auto operator->() const & noexcept -> const T*
        { return &_value; }

        [[nodiscard]] constexpr auto operator->() && noexcept -> T*
        { return &_value; }

        [[nodiscard]] constexpr auto operator->() const && noexcept -> const T*
        { return &_value; }
    private:
        T _value;

        constexpr void check() noexcept(
            (nothrow_predicate<decltype(Constraints), T> && ...)
            and noexcept(fail())
        )
        {
            std::puts("Check");
            bool satisfied = (Constraints(_value) && ...);
            if (!satisfied)
            {
                fail();
            }
        }

        constexpr void fail() noexcept(nothrow_null_constructible<T>)
            requires nullable<T>
        {
            _value = constrained_traits<T>::null;
        }

        [[noreturn]] constexpr void fail() const
            requires (not nullable<T>)
        {
            throw std::runtime_error{"Constraints not satisfied"};
        }
    };
} // namespace ct
