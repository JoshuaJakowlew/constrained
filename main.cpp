#include <iostream>

#include <constrained_type.hpp>

constexpr auto is_even = [](std::optional<int> const & x) noexcept { return x.value() % 2 == 0; };
using even_int = ct::constrained_type<std::optional<int>, is_even>;

// static_assert(
//     ct::nullable<std::optional<int>>
//     and std::convertible_to<std::optional<int>, bool>
// , "message");

int main()
{
    even_int x{42};
    std::cout << static_cast<bool>(x._value) << '\n';
}
