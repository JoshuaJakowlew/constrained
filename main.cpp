#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

#include <constrained_type.hpp>
#include <type_traits>

constexpr auto non_null_check = [](auto * x) { return x != nullptr; };

template <typename T>
using non_null = ct::constrained_type<T*, non_null_check>;

struct foo { int x = 42; };

auto deref(non_null<foo> ptr)
{
    return ptr->x;
}

void non_null_test()
{
    foo x;
    non_null<foo> ptr{&x};
    std::cout << deref(ptr) << '\n';
}

template <auto N>
struct size_gt_check
{
    constexpr bool operator()(auto const & x) const
    {
        return x.size() > N;
    }
};

template <auto E>
struct has_elem_check
{
    constexpr bool operator()(auto const & x) const
    {
        return std::end(x) != std::find(std::begin(x), std::end(x), E);
    }
};

template <typename T>
struct has_runtime_elem_check
{
    constexpr bool operator()(auto const & x) const
    {
        T c;
        std::cout << "Enter required character in email\n> ";
        std::cin >> c;
        return std::end(x) != std::find(std::begin(x), std::end(x), c);
    }
};

template <template <typename> typename Container>
using email_t = ct::constrained_type<Container<char>,
    size_gt_check<4>{},
    has_elem_check<'@'>{},
    has_elem_check<'.'>{},
    has_runtime_elem_check<char>{}
>;

void email_test()
{
    auto email_str = email_t<std::basic_string>{"hello@gmail.com"};
    std::cout << *email_str << '\n';
    
    auto email_vec = email_t<std::vector>{std::vector{'h', 'i', '@', 'y', 'a', '.', 'r', 'u'}};
    for (auto c : *email_vec)
    {
        std::cout << c;
    }
    std::cout << '\n';
}

template <>
struct ct::constrained_traits<int>
{
    static constexpr bool is_nullable = true;
    static constexpr int null = -1;
};

constexpr auto even_check = [](auto x) { return x % 2 == 0; };

template <typename T>
using even_t = ct::constrained_type<T, even_check>;

void even_test()
{
    even_t<int> x{1};
    std::cout << *x << '\n';

    try { even_t<unsigned> y{1u}; }
    catch (...) { std::cout << "Ooops\n"; }
}

int main()
{
    non_null_test();
    email_test();
    even_test();
}
