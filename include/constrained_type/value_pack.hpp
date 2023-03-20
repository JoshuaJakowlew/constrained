#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>

namespace ct
{
    template <auto... Xs>
    struct value_pack final
    {
    public:
        /*
         * Amount of values in a pack
         */
        static constexpr std::size_t size = sizeof...(Xs);

        /*
         * Add Ys to current pack
         * value_pack<1, 2>::add<3, 4> <=> value_pack<1, 2, 3, 4>
         */
        template <auto... Ys>
        using add = value_pack<Xs..., Ys...>;

    // Order matters here and below. This is ugly, but I don't know a better solution
    private:
        template <auto... Ys>
        static consteval auto add_pack_impl(value_pack<Ys...>*) -> add<Ys...>;
    public:
        /*
         * Add content of Pack to current pack
         * value_pack<1, 2>::add_pack<value_pack<3, 4>> <=> value_pack<1, 2, 3, 4>
         */
        template <typename Pack>
        using add_pack = decltype(
            add_pack_impl(static_cast<Pack*>(nullptr))
        );

    public:
        using clear = value_pack<>;

        /*
         * Get wrapped value by index
         * value_pack<1, 2>::get<0> <=> value_pack<1>
         */
        template <std::size_t I>
        using get = std::tuple_element_t<I, std::tuple<value_pack<Xs>...>>;

    private:
        template <typename Pack, std::size_t I, std::size_t... Is>
        struct seq
        {
            using pack = typename Pack::template add_pack<get<I>>;
            using type = typename seq<pack, Is...>::type;
        };

        template <typename Pack, std::size_t I>
        struct seq<Pack, I>
        {
            using type = typename Pack::template add_pack<get<I>>;
        };
    public:
        /*
         * Get wrapped values by indices in any order
         * value_pack<1, 2, 3, 4, 5>::get_many<3, 1> <=> value_pack<4, 2>
         */
        template <std::size_t... Is>
        using get_many = typename seq<value_pack<>, Is...>::type;

    private:
        template <std::size_t Start, std::size_t... Is>
        static consteval auto get_range_impl(std::index_sequence<Is...>) -> get_many<Start + Is...>;
    public:
        /*
         * Get subpack of values of size Len starting from Start index
         * std::same_as<value_pack<1, 2, 3, 4, 5>::get_range<1, 3> <=> value_pack<2, 3, 4>
         */
        template <std::size_t Start, std::size_t Len>
        using get_range = decltype(
            get_range_impl<Start>(std::make_index_sequence<Len>{})
        );
    };
} // namespace ct