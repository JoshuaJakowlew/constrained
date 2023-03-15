# Constrained

This is a small utility library that helps **expressing code invariants in types**. Constrained provides `constrained_type` class, parametrized by the type of holding value and set of predicates, applied to it at creation.
In other words, you can associate conditions with a value at the type level. Сonditions are checked when the object is created, and if they are not met, an error occurs (an exception, or a special null value provided by the user).
Constrained types can be used, for example, to validate function parameters, to more strictly limit type values (e.g., limit the value range of int type, which stores a person's age, or limit the string length for a password), and to express various invariants in general.
## Fast tutorial for the impatient

### Fast example
Imagine, you have some code with invariants, that you must check unconditionally. For example, you have a function like this:
```c++
template <typename T>
T deref(T* ptr)
{
    return *ptr;
}
```
This function has its invariant - `ptr` must not be `nullptr`. You can manually check it like this:
```c++
template <typename T>
T deref(T* ptr)
{
    if (!ptr)
        throw std::runtime_exception{"ptr is null"};
    return *ptr;
}
```
That's OK, but you don't encode this invariant anywhere in function signature. User must remember, which values must be passed to the function. Also, you can forget to manually check required invariants.

With `constrained_type` you can write like this:
```c++
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

Our `not_null` type holds `non_null_check` in type (using non-type template parameters). So, it doesn't affect its size and has no overhead.
### Basic API reference
`constrained_type` is declared as the following code and parametrized by:
- `T` - type of holding value.
- `Constraints` - predicates. These are non-type parameters, so they hold values, not types.
```c++
template <typename T, auto... Constraints>
using constrained_type = /* Complicated stuff */
```
You should declare predicates (you can write them directly at the type declaration).
```c++
constexpr auto age_check = [](int x) { return x > 0 && x < 150; };
```
Then build your constrained type and use it.
```c++
using age_t = ct::constrained_type<int, age_check>;
age_t good_age{42};
```
Checks are performed only once - at creation time. So, there is no way to modify holding object. You may either take a const reference, or move it from constrained_type.
Value can be accessed by dereference operator, or by operator -> (e. g. for member access or method calls).
```c++
int copy = *good_age;
int move = *std::move(good_age);

age_t bad_age{-42}; // throws
```
When checks fails - constrained_type is in failed state. By default this leads to exception. But there is overload for `std::optional<T>` which doesn't throw. Then constrained_type becomes nullable. When constrained type is nullable, it doesn't throw when checks fail, and provides operator bool for validity checks. Nullability is controlled by trait objects and can be customized by user (look at **Nullability Traits** section).
```c++
template <>
struct ct::default_traits<int>
{
    using value_type = int;
    static constexpr bool is_nullable = true;
    static constexpr int null = -1;
};

nullable_age_t bad_age{-42}; // doesn't throw, holding type is in "null" state
static_cast<bool>(bad_age); // false
```
Writing constraints is cool, but boring. Constrained provides you pre-defined combinators for easier constraint writing. Let's build somewhat synthetic, but more interesting example:
```c++
using email_t = constrained_type<std::string,
    gt<&std::string::length, 10>, // Remember, we can pass different callables, not only lambdas
    lt<&std::string::length, 20> // Multiple constraints applied in order
>;

constexpr auto is_even = [](int x) { return x % 2 == 0; };
constexpr auto divisible_by_5 = [](int x) { return x % 5 == 0; };
using cool_int = constrained_type<int,
	gt<0>, lt<42>, // 0 < x < 42
	or_<is_even, divisible_by_five> // is_even(x) || divisible_by_five(x)
>;
```
You can manipulate constraint sets with template magic. Let's see, how we can add constraints to given constrained type:
```c++
// From previous samples
constexpr auto age_check = [](int x) { return x > 0 && x < 150; };
using age_t = ct::constrained_type<int, age_check>;

// Add new check
constexpr auto legal_age_check = [](int x) { return x > 18; };
using legal_age_t = age_t
    ::add_constraints<legal_age_check>; // Create new type with added constraint

auto child = legal_age_t{10}; // Fails
auto oldman = legal_age_t{60}; // OK
auto deadman = legal_age_t{666}; // Fails
```
## Constrained type API

Linrary provides two ways of using constrained types: a simple but somewhat limited approach and more difficult and powerful one:
- `basic_constrained_type<T, Trait, Config, Constraints...>` class parametrized by type of wrapped value, trait type, behaviour configuration and set of constraints . It's highly customizable and powerful.
- `constrained_type<T, Constraints...>` is an alias for `base_constrained_type`. Basically, it's `basic_constrained_type<T, default_traits<T>, configuration_point{}, Constraints...>`. This alias is way easier to use and is suitable for most of constraints. 
Basically, difference between `basic_constrained_type` and `constrained_type` is like difference between `basic_string` and `string`. Simple one is a specialization of the more complex and powerful class.

### Nullability traits
There are two ways to react on failed constraints:
- Throw an exception
- Hold specified "null" value ("" for `std::string`, empty state aka `std::nullopt` for `std::optinal`, etc.).
Both of these modes have pros and cons. So, it's up to the user to decide which one to use.
This is done via **Trait** template parameter of `basic_constrained_type`.
#### Trait
Every trait must satisfy `constrained_trait` concept.
Keeping it simple, here is reference implementation of `default_traits<T>`:
```c++
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
```
As you can see, trait must provide:
- `value_type` type, equal to the type `T` of wrapped value.
- `is_nullable` boolean constant. This one works like switch between throwing and nullable mode.
- You must provide `null` value if `is_nullable == true`. Wrapped value will be set to this value if constraints fail.
#### Custom traits
`default_traits<T>` are used by default, if you use simplified `constrained_type<T, Constraints...>` alias.
But sometimes you may need different modes for the same type. You can implement different traits and use them as you want.
#### Boolean conversion
If `Trait::is_nullable` is true then you can use `operator bool`. It returns true if wrapped value is equal to `Trait::null` value.
Explicitness of `operator bool` is defnend via `Config` template parameter.

### Configuration Point
This simple struct configures behavior of `basic_constrained_type`. The following struct is passed as `Config` non-type template parameter.
Simplified `constrained_type` alias uses default values:
```c++
struct configuration_point
{
    bool explicit_bool = true;
    bool explicit_forwarding_constructor = true;
    bool opaque_dereferencable = true;
    bool opaque_member_accessible = true;
    bool opaque_pointer_accessible = true;
};
```
#### Explicit bool
This flag controls if `operator bool` for nullable types is explicit.
#### Explicit forwarding constructor
This flag controls if the following constructor (code is simplified) is explicit.
```c++
template <typename... Args>
basic_constrained_type(Args&&... args)
    requires std::is_constructible_v<T, Args...>
    : _value{std::forward<Args>(args)...}
{...}
```
#### Opaque dereferencable
This switch controls if `operator*` returns just the wrapped value `x` or `*x` if `x` is dereferencable.
Imagine that we have wrapped `std::optional<std::string>`.
```c++
auto wrapped = wrapped_optional{"42"};
int x = **wrapped; // if opaque dereferencable flag is off.
int x = *wrapped; // if opaque dereferencable flag is on.
```
#### Opaque member accessible
Works like opaque dereferencable, but for `operator->`.
```c++
auto wrapped = wrapped_optional{"42"};

size_t x = wrapped->value().size(); // If opaque member accessible is off.
size_t x = wrapped->size(); // If opaque member accessible is on.
```
#### Opaque pointer accessible
This flag is not as useful as previous one. By default (with all opaque flags off) `operator->` returns `T*` or `&_value` (with const when needed). In generic code this matters, if you call `operator->()` directly. But for pointer types address of pointer is not very useful. Probably we want **value** of this pointer. This flag allows this behaviour.
```c++
using wrapped_optional = constrained_type<std::string*, ...>;
auto wrapped = wrapped_optional{new std::string{"42"}};

size_t x = (*wrapped.operator->())->size(); // If opaque pointer accessible is off
size_t x = wrapped->size(); // If opaque pointer accessible is on
```
### Constraints
The last template parameter of `basic_constrained_type` is `auto... Constraints`.
These are a set of any callables with `T const &`-compatible parameter returning `bool`.
Or, more formally, the following expression must evaluete to `true` to satisfy template requirements:
```c++
(std::predicate<decltype(Constraints), T const &> && ...)
```

- Constraints are evaluated in the same order as they were provided.
- Constraints should be **pure**. C++ doesn't allow purity checks, but most likely you want your constraints to return the same results with the same input values.
- Constraint evaluation is performed only once in constructor. You can't mutate wrapped type directly.
- This library assumes that `const` types can't be visibly changed (e. g. if you type has `mutable` fields, than `const` methods can't change invariants).
- If you **really want** (but you should never do this, obviously) to change wrapped type after checks are performed (and possibly destroy all the guarantees of constraint satisfaction) - make desired fields `mutable` and mutate them via `const` methods.
- If you want to change value - you should move it from wrapper and mutate as you wish. Moved-from constrained type may not satisfy constraints (if moved-from wrapped value doesn't).

### Flexible API for constraint manipulation
This API allows user to add, clear, select and replace constraints in flexible ways.

- You can extract constraints as a **value_pack** using `ct::constraint_pack`.
- You can **replace constraints with new ones** using `ct::set_constraint_pack<pack>`.
- You can **add constraints** using `ct::add_constraint_pack<pack>`.
- `add_constraint_pack` and `set_constraint_pack` have **overloads for non-type variadic packs** (named `add_constraints` and `set_constraints` respectively).
- You can **clear all constraints** using `ct::clear_constraints`.
- Every "method" excluding `ct::constraint_pack` can be rewritten from `ct::set_constraints<...>` form to `set_constraints<ct, ...>` form.

Overloads for non-type variadic packs allows simple usage of constraint manipulation API.
Advanced manipulations could be done via **value_pack API**.

```c++
using ct = constrained_type<T, Constraints...>;

using cs = ct::constraint_pack;

using set_ct = ct::set_constraints<...>;
using set_ct = ct::set_constraint_pack<pack>;

using add_ct = ct::add_constraints<...>;
using add_ct = ct::add_constraint_pack<pack>;

using empty_ct = ct::clear_constraints;
```

## Value Pack API
This API simplifies operation on value packs.
Non-type variadic packs are simple, but not very handy to be processed in some kind of advanced way. This class wraps non-type packs into type and defines some utility primitives to work with.

```c++
using pack = value_pack<1, 2>;

using p_add = pack::add<3, 4>; // value_pack<1, 2, 3, 4>
using p_add = pack::add_pack<value_pack<3, 4>>; // value_pack<1, 2, 3, 4>

using p_clear = pack::clear; // value_pack<>

using pack = value_pack<1, 2, 3, 4, 5>;
using p_get = pack::get<2>; // value_pack<3>
using p_get = pack::get_many<2, 0, 2>; // value_pack<3, 1, 3>
using p_get = pack::get_range<2, 3>; // value_pack<3, 4, 5>
```

## Combinators API

### Combinator parameter types
Combinators take values aka **non-type** template parameters as parameters.
There are four value categories for params. From user's side there is not much of a difference - you just pass what you want. But these categories are important to understand if you want to implement you own combinators.
#### Simple values
These are non-callable values which can be directly passed as non-type parameter.
```c++
val<42>;
```
#### Runtime values
These are nullary callables which can be passed as non-type parameters.
You can use nullary lambdas to provide runtime-only or non-simple values.
Under the hood these values are executed and combinators use returned value.
```c++
val<[]{reutrn "Hello"s;}>;
```
#### Object-callable values
These are unary callables with `T const &` compatible parameter. Under the hood checked value x is passed inside this callable and combinators use returned value.
```c++
val<[](int x){ return x % 2 == 0; }>;
val<[](auto const &x) { return x; }>;
val<&std::string::length>;
```
#### Combinator values
These are equivalent with **Object-callable values**. The only difference is that combinator values satisfy `combinator` concept. This category exists only for differentiation between combinators and random object-callables. Currently, there is no real difference between these two categories. Maybe in the future combinators will have uniqie properties.
```c++
val<val<42>>;
val<lt<1, 2, 3>>;
```

### Utility combinators

#### val
Returns parameter **A** ignoring passed value **x**.

```c++
val<A>(x) <=> A
```

### Binary Operator Combinators

C++ binary operator combinator subdivides into two combinator categories - **unary** and **n-ary**. Don't be confused with arity - C++ binary operators and combinators are different.

#### Unary
Acts like partially right-applied binary function with (e. g. `(< 2)`).
```c++
gt<c>(x) <=> x > c(x)
```

#### N-ary
Acts like n-ary function of **Args**.
```c++
gt<c1, ..., cn>(x) <=> c1(x) > ... > cn(x)
```

#### List of defined operator combinators

```
Relational:
eq  | ==
neq | !=
lt  | <
le  | <=
gt  | >
ge  | >=

Logical:
and_ | &&
or_  | ||
```
### Unary Operator Combinators

C++ unary operator combinator belongs to **unary** combinator category. **N-ary** unary operators doesn't make much sense.

#### Unary
Acts like partially right-applied binary function with (e. g. `(< 2)`).
```c++
not_<c>(x) <=> !c(x)
```
#### List of defined operator combinators

```c++
Logical:
not_ | !
```
### Ternary Operator Combinators

There is only one ternary operator combinator - `if_<Cond, WhenTrue, WhenFalse` combinator. It behaves like if statement.

```c++
if_<cond, true_branch, false_branch>(x) <=>
if (cond(x))
    return true_branch(x);
else
    return false_branch(x);
```
