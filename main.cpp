#include <iostream>

#include <optional.hpp>
#include <constrained_type.hpp>

struct greeter
{
    void greet() const { std::puts("Hi! Greetings to everyone!"); }
};

using type = greeter;//std::optional<std::string>;
constexpr auto is_even = [](type const & x) noexcept {
    // return std::stoull(*x) % 2 == 0;
    return true;
};
using even_int = ct::constrained_type<type, is_even>;

template <typename T>
struct ct::constrained_traits<swl::optional<T>>
{
    static constexpr bool is_nullable = true;
    static constexpr swl::optional<T> null = swl::nullopt;
};

// static_assert(
//     ct::nullable<type>
//     and ct::convertible_to<type, bool>
//     and ct::nothrow_dereferenceable<type>
//     and ct::nothrow_member_accessible<type>
// , "message");

int main()
{
    even_int x;
    even_int y = x;
    // auto v = *x;
    // std::cout << *x << '\n';
}
