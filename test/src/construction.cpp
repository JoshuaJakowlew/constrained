#include <string>

#include <doctest/doctest.h>

#include <constrained_type.hpp>

TEST_SUITE("Construction")
{
    using name_t = ct::constrained_type<std::string,
        ct::lt<&std::string::length, 10uz>
    >;

    TEST_CASE("Sanity checks")
    {
        CHECK_THROWS(name_t{"0123456789"});
        CHECK(*name_t{"Joshua"} == "Joshua");
    }
    TEST_CASE("Constructors")
    {
        CHECK(*name_t{} == "");
        CHECK(*name_t{3uz, 'a'} == "aaa");
        CHECK([]{
            auto x = name_t{"abc"};
            auto y(x);
            return *x == *y && *x == "abc" && *y == "abc";
        }());
        CHECK([]{
            auto x = name_t{"abc"};
            auto y(std::move(x));
            return *y == "abc";
        }());
    }
    TEST_CASE("Assignment operators")
    {
        CHECK([]{
            auto x = name_t{"abc"};
            auto y = name_t{"def"};
            x = y;
            return *x == "def" && *y == "def";
        }());
        CHECK([]{
            auto x = name_t{"abc"};
            auto y = name_t{"def"};
            x = std::move(y);
            return *x == "def";
        }());
    }
}