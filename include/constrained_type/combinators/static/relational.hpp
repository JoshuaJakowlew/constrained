#pragma once

#include <concepts>
#include <type_traits>

#include <constrained_type/constrained_type.hpp>

#pragma region by_value
#define CONSTRAINED_TYPE_RELATIONAL_BY_VALUE(name, op) \
    template <auto Value> \
    struct name \
    { \
        [[nodiscard]] constexpr bool operator()(auto const & x) const \
            noexcept(noexcept(x op static_cast<std::remove_reference_t<decltype(x)>>(Value))) \
            requires std::convertible_to<decltype(Value), std::remove_reference_t<decltype(x)>> \
        { \
            return x op static_cast<std::remove_reference_t<decltype(x)>>(Value); \
        } \
    };

#define CONSTRAINED_TYPE_RELATION_BY_VALUE() \
    CONSTRAINED_TYPE_RELATIONAL_BY_VALUE(eq,  ==) \
    CONSTRAINED_TYPE_RELATIONAL_BY_VALUE(neq, !=) \
    CONSTRAINED_TYPE_RELATIONAL_BY_VALUE(gt,   >) \
    CONSTRAINED_TYPE_RELATIONAL_BY_VALUE(ge,  >=) \
    CONSTRAINED_TYPE_RELATIONAL_BY_VALUE(lt,   <) \
    CONSTRAINED_TYPE_RELATIONAL_BY_VALUE(le,  <=)
#pragma endregion by_value
#pragma region by_method
#define CONSTRAINED_TYPE_RELATION_BY_METHOD_SIZE_T(method) \
    namespace detail::method_##method \
    { \
        template <typename T> \
        using size_t = decltype(std::declval<T>().method()); \
    }

// Currently decltype(x) is const reference, so for std::string and size method
// detail::method_size::size_t will be parametrized by const std::string &, not by std::string
// This doesn't affect type uniqueness and simplifies compiler work (e.g. no std::remove_reference_t)
#define CONSTRAINED_TYPE_RELATIONAL_BY_METHOD(prefix, postfix, op, method) \
    template <auto Value> \
    struct prefix##_##postfix \
    { \
        [[nodiscard]] constexpr bool operator()(auto const & x) const \
            noexcept(noexcept(x.method() op static_cast<detail::method_##method::size_t<decltype(x)>>(Value))) \
            requires std::convertible_to<decltype(Value), detail::method_##method::size_t<decltype(x)>> \
        { \
            return x.method() op static_cast<detail::method_##method::size_t<decltype(x)>>(Value); \
        } \
    }; \

#define CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_POSTFIX(postfix, op, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD(method, postfix, op, method)

#define CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_PREFIX(prefix, op, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD(prefix, method, op, method)

#define CONSTRAINED_TYPE_OP_RELATION_BY_METHOD(method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_POSTFIX(eq,  ==, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_POSTFIX(neq, !=, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_POSTFIX(gt,   >, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_POSTFIX(ge,  >=, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_POSTFIX(lt,   <, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_POSTFIX(le,  <=, method)

#define CONSTRAINED_TYPE_MINMAX_RELATION_BY_METHOD(method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_PREFIX(min,  >=, method) \
    CONSTRAINED_TYPE_RELATIONAL_BY_METHOD_PREFIX(max,  <=, method)

#define CONSTRAINED_TYPE_RELATION_BY_METHOD(method) \
    CONSTRAINED_TYPE_RELATION_BY_METHOD_SIZE_T(method) \
    CONSTRAINED_TYPE_OP_RELATION_BY_METHOD(method) \
    CONSTRAINED_TYPE_MINMAX_RELATION_BY_METHOD(method)
#pragma endregion by_method

namespace ct
{
    CONSTRAINED_TYPE_RELATION_BY_VALUE();

    CONSTRAINED_TYPE_RELATION_BY_METHOD(size    );
    CONSTRAINED_TYPE_RELATION_BY_METHOD(length  );
    CONSTRAINED_TYPE_RELATION_BY_METHOD(capacity);
    CONSTRAINED_TYPE_RELATION_BY_METHOD(front   );
    CONSTRAINED_TYPE_RELATION_BY_METHOD(back    );
} //namespace ct