#include <constrained_type.hpp>

static_assert(std::same_as<
    ct::value_pack<1, 2>::add<3, 4>,
    ct::value_pack<1, 2, 3, 4>
>);
static_assert(std::same_as<
    ct::value_pack<1, 2>::add_pack<ct::value_pack<3, 4>>::add<5>,
    ct::value_pack<1, 2, 3, 4, 5>
>);
static_assert(std::same_as<
    ct::value_pack<1, 2>::get<0>,
    ct::value_pack<1>
>);
static_assert(std::same_as<
    ct::value_pack<1, 2>::clear::add<1, 2>::get<0>,
    ct::value_pack<1>
>);
static_assert(std::same_as<
    ct::value_pack<1, 2, 3, 4, 5>::get_many<3, 1, 3>,
    ct::value_pack<4, 2, 4>
>);
static_assert(std::same_as<
    ct::value_pack<1, 2, 3, 4, 5>::get_range<1, 3>,
    ct::value_pack<2, 3, 4>
>);

using my_pack = ct::value_pack<1, 2, 3, 4, 5>
    ::get_range<1, 4>
    ::add<6, 7>
    ::get_many<1, 3, 4, 5>
    ::get_range<1, 2>;
static_assert(std::same_as<my_pack, ct::value_pack<5, 6>>);