# Constrained

This is a small utility library that helps expressing code invariants in types. 
Constrained provides `constrained_type` class, parametrized by the type of holding value and set of predicates, applied to it at creation.

```cpp
template <typename T, auto... Constraints>
    requires (std::predicate<decltype(Constraints), T const &> && ...)
class constrained_type { ... };
```

Here `Constraints` is a set of predicates (e.g. callables which evaluates to `bool`) applied to the holding value.

Imagine, you have some code with invariants, that you must check unconditionally. For example, you have a function like this:

```cpp
template <typename T>
T deref(T* ptr)
{
    return *ptr;
}
```

This function has its invariant - `ptr` must not be `nullptr`. You can manually check it like this:

```cpp
template <typename T>
T deref(T* ptr)
{
    if (!ptr)
        throw std::runtime_exception{"ptr is null"};
    return *ptr;
}
```

That's OK, but you don't encode this invariant anywhere in function type. User must remember, which values must be passed to the function.
Also, you can forget to manually check required invariants.

With `constrained_type` you can write something like this:

```cpp
constexpr auto non_null_check = [](auto * x) { return x != nullptr; };

template <typename T>
using non_null = ct::constrained_type<T*, non_null_check>;

template <typename T>
auto deref(non_null<T> ptr)
{
    return *ptr;
}

...

int x;
non_null<int> ptr{&x}; // Exception if &x is null
std::cout << deref(ptr) << '\n';
```

This time we encoded invariant in our type system. Now we know, which invariants we should observe as a user of this function. Also, we don't forget to write checks by hand.

We can use more complex conditions. Imagine, that we have to validate an email address, using the following rules:

- Address must be longer than 4 symbols
- Address must contain `@` and `.` symbols
- User can enter a specific character, that address must contain
- Address may be contained in arbitrary container, e.g. `std::string`, `std::vector`, etc.

First we define check for size. Unfortunately, we can't provide a non-type template parameter for lambda type, so, let's use good old structs:

```cpp
template <auto N>
struct size_gt_check
{
    constexpr bool operator()(auto const & x) const
    {
        return x.size() > N;
    }
};
```

Then we must provide checks for `@` and `.` symbols:

```cpp
template <auto E>
struct has_elem_check
{
    constexpr bool operator()(auto const & x) const
    {
        return std::end(x) != std::find(std::begin(x), std::end(x), E);
    }
};
```

And finally check for user-passed symbol:

```cpp
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
```

Now the tasty part. Let's define our constrained type:

```cpp
template <template <typename> typename Container>
using email_t = ct::constrained_type<Container<char>,
    size_gt_check<4>{},
    has_elem_check<'@'>{},
    has_elem_check<'.'>{},
    has_runtime_elem_check<char>{}
>;
```

As you can see, we provide several callables, which perform needed checks. If any of them will fail - an exception will be raised.

Now we tie evetything together:

```cpp
auto email_str = email_t<std::basic_string>{"hello@gmail.com"}; // exception, if user provided missing symbol
std::cout << *email_str << '\n';

auto email_vec = email_t<std::vector>{
    std::vector{'h', 'i', '@', 'y', 'a', '.', 'r', 'u'}
}; // exception, if user provided missing symbol
for (auto c : *email_vec) { std::cout << c; }
std::cout << '\n';

auto wrong_email = email_t<std::basic_string>{"hellogmail.com"}; // Exception - missing '@' symbol
```

Sometimes we don't want exceptions to be thrown and want some kind of nullable types like `std::optional`.

`constrained_type` uses `constrained_traits<T>` traits for nullable/throwing behavior.

By default traits treat any type as non-nullable:

```cpp
template <typename T>
struct constrained_traits
{
    static constexpr bool is_nullable = false;
};
```

But if we want our `constrained_type` to be nullable, we can specialize this trait as we want. By default this libraty provides specialization for `std::optional<T>`:

```cpp
template <typename T>
struct constrained_traits<std::optional<T>>
{
    static constexpr bool is_nullable = true;
    static constexpr std::optional<T> null = std::nullopt;
};
```

Specialization must provide null value, if `is_nullable` is true.

Let's write our own specialization. Let `int` type be nullable with `-1` as a null value:

```cpp
template <>
struct ct::constrained_traits<int>
{
    static constexpr bool is_nullable = true;
    static constexpr int null = -1;
};
```

And let's implement `even_int` type for even integers:

```cpp
constexpr auto even_check = [](auto x) { return x % 2 == 0; };

template <typename T>
using even_t = ct::constrained_type<T, even_check>;
```

Now if we try different types, we would get different behavior:

```cpp
even_t<int> x{1};
std::cout << *x << '\n'; // Outputs -1 as it is a null value for int

try { even_t<unsigned> y{1u}; } // throws an exception, because unsigned is not nullable according to constrained_traits
catch (...) { std::cout << "Ooops\n"; } 
```