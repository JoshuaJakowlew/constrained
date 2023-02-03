# Constrained

This is a small utility library that helps expressing code invariants in types. 
Constrained provides `constrained_type` class, parametrized by the type of holding value and set of predicates, applied to it at creation.

In other words, you can associate conditions with a value at the type level. The conditions are checked when the object is created, and if they are not met, an error occurs (an exception, or a special null value provided by the user).

`constrained_type` can be used, for example, to validate function parameters, to more strictly limit type values (e.g., limit the value range of int type, which stores a person's age, or limit the string length for a password), and to express various invariants in general.

## Fast tutorial for the impatient

### Basic API reference

```c++
/*
 * constreained_type declared as the following code and parametrized by:
 * T - type of holding value
 * Constraints - predicates. These are non-type parameters, so they hold values, not types
 */

template <typename T, auto... Constraints>
using constrained_type = /* Complicated stuff */

// You should declare predicates
constexpr auto age_check = [](int x) { return x > 0 && x < 150; };

// Then build your constrained type
using age_t = ct::constrained_type<int, age_check>;

// Then use it like this
age_t good_age{42};

/*
 * Checks are performed only once - at creation time.
 * So, there is no way to modify holding object. You may either take a const reference, or move it from constrained_type
 * Value can be accessed by dereference operator, or by operator -> (e. g. for member access or method calls)
 */
int copy = *good_age;
int move = *std::move(good_age);

age_t bad_age{-42}; // throws

/* When checks fails - constrained_type is in failed state.
 * By default this leads to exception.
 * But there is overload for std::optional<T> which doesn't throw. Then constrained_type becomes nullable
 * When constrained type is nullable, it doesn't throw when checks fail, and provides operator bool for validity checks
 * Nullability is controlled by trait objects and can be customized by user.
 */

constexpr auto nullable_age_check = [](std::optional<int> x) { return x > 0 && x < 150; };
using nullable_age_t = ct::constrained_type<std::optional<int>, nullable_age_check>;

nullable_age_t bad_age{-42}; // doesn't throw, holding type is in "null" state
static_cast<bool>(bad_age); // false

```

### Non-null pointer

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

That's OK, but you don't encode this invariant anywhere in function signature. User must remember, which values must be passed to the function.
Also, you can forget to manually check required invariants.

With `constrained_type` you can write like this:

```cpp
constexpr auto non_null_check = [](auto * ptr) { return ptr != nullptr; };

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

Our `not_null` type holds `non_null_check` in type (using non-type template parameters). So, it doesn't affect its size and has no overhead.

### Email validation

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
### Even int

Sometimes we don't want exceptions to be thrown and want some kind of nullable types like `std::optional`.

`constrained_type` uses `default_traits<T>` traits for nullable/throwing behavior.

By default traits treat any type as non-nullable:

```cpp
template <typename T>
struct default_traits
{
    using value_type = T;
    static constexpr bool is_nullable = false;
};
```

But if we want our `constrained_type` to be nullable, we can specialize this trait as we want. By default this libraty provides specialization for `std::optional<T>`:

```cpp
template <typename T>
struct default_traits<std::optional<T>>
{
    using value_type = std::optional<T>;
    static constexpr bool is_nullable = true;
    static constexpr value_type null = std::nullopt;
};
```

Specialization must provide null value, if `is_nullable` is true.

Let's write our own specialization. Let `int` type be nullable with `-1` as a null value:

```cpp
template <>
struct ct::default_traits<int>
{
    using value_type = int;
    static constexpr bool is_nullable = true;
    static constexpr value_type null = -1;
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