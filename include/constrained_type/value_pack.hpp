#pragma once

namespace ct
{
    template <auto... Xs>
    struct value_pack final
    {
    
    public:
        template <auto... Ys>
        using add = value_pack<Xs..., Ys...>;
    protected:
        template <auto... Ys>
        static consteval auto add_pack_impl(value_pack<Ys...>*) -> add<Ys...>;
    public:
        template <typename Pack>
        using add_pack = decltype(
            add_pack_impl(static_cast<Pack*>(nullptr))
        );
    };
} // namespace ct